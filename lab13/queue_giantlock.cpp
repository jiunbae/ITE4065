#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include <atomic>
#include <iostream>

#define NUM_PRODUCER                4
#define NUM_CONSUMER                NUM_PRODUCER
#define NUM_THREADS                 (NUM_PRODUCER + NUM_CONSUMER)
#define NUM_ENQUEUE_PER_PRODUCER    100000
#define NUM_DEQUEUE_PER_CONSUMER    NUM_ENQUEUE_PER_PRODUCER

bool flag_verification[NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER];
void enqueue(int key);
int dequeue();

// -------- Queue with coarse-grained locking --------
// ------------------- Modify here -------------------
#define QUEUE_SIZE      1024

struct QueueNode {
    int key;
    std::atomic<int> flag;
};

QueueNode queue[QUEUE_SIZE];
std::atomic<uint64_t> front;
std::atomic<uint64_t> rear;

pthread_mutex_t mutex_for_queue = PTHREAD_MUTEX_INITIALIZER;

void init_queue(void) {
    front.store(0);
    rear.store(0);
}

void enqueue(int key) {
    while (1) {
        //pthread_mutex_lock(&mutex_for_queue);

        if (rear.load() >= front.load() + QUEUE_SIZE) {
            // queue full!
            // pthread_mutex_unlock(&mutex_for_queue);
            pthread_yield_np();
        } else {
            break;
        }
    }
    uint64_t old = rear.fetch_add(1);
    int oldFlag;
    do {
        oldFlag = queue[old % QUEUE_SIZE].flag.load();
    } while (old / QUEUE_SIZE != oldFlag / 2 && rear.load() < front.load() + QUEUE_SIZE);

    queue[old % QUEUE_SIZE].key = key;
    queue[old % QUEUE_SIZE].flag.fetch_add(1);

    // pthread_mutex_unlock(&mutex_for_queue);
}

int dequeue(void) {
    while (1) {
        // pthread_mutex_lock(&mutex_for_queue);
        
        if (rear.load() == front.load()) {
            // queue empty!
            // pthread_mutex_unlock(&mutex_for_queue);
            pthread_yield_np();
        } else {
            break;
        }
    }
    uint64_t old = front.fetch_add(1);
    int oldFlag;
    do {
        oldFlag = queue[old % QUEUE_SIZE].flag.load();
    } while (old / QUEUE_SIZE != oldFlag / 2 && rear.load() != front.load());

    int ret_key = queue[old % QUEUE_SIZE].key;   
    queue[old % QUEUE_SIZE].flag.fetch_add(1);
    // pthread_mutex_unlock(&mutex_for_queue);
    return ret_key;
}
// ------------------------------------------------

void* ProducerFunc(void* arg) {
    long tid = (long)arg;

    int key_enqueue = NUM_ENQUEUE_PER_PRODUCER * tid;
    for (int i = 0; i < NUM_ENQUEUE_PER_PRODUCER; i++) {
        enqueue(key_enqueue);
        key_enqueue++;
    }

    return NULL;
}

void* ConsumerFunc(void* arg) {
    for (int i = 0; i < NUM_DEQUEUE_PER_CONSUMER; i++) {
        int key_dequeue = dequeue();
        flag_verification[key_dequeue] = true;
    }

    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];

    init_queue();

    for (int i = 0; i < NUM_THREADS; i++) {
        if (i < NUM_PRODUCER) {
            pthread_create(&threads[i], 0, ProducerFunc, (void**)i);
        } else {
            pthread_create(&threads[i], 0, ConsumerFunc, NULL);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // verify
    bool flag = false;
    for (int i = 0; i < NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER; i++) {
        if (flag_verification[i] == false) {
            flag = true;
        } else {

            std::cout << i << std::endl;
        }
    }

    if (flag) {
        printf("INCORRECT!\n");
    } else {
        printf("CORRECT!\n");   
    }

    return 0;
}

