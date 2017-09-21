#include <stdio.h>
#include <pthread.h>
#include <math.h>

#define NUM_THREAD      10
#define NUM_INCREASE    100000

int cnt_global = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

bool is_prime[NUM_INCREASE];

void IsPrime(int n) {
    if (n < 2) {
        is_prime[n] = false;
        return;
    }

    for (int i = 0; i <= sqrt(n); i++) {
        if (n % i == 0) {
            is_prime[n] = false;
            return;
        }
    }
    is_prime[n] = true;
}

void* ThreadFunc(void* arg) {
    long tid = (long)arg;
    long cnt_local = 0;

    for (int i = 0; i < NUM_INCREASE; i++) {
        pthread_mutex_lock(&mutex);
        cnt_global++;   // increase global value
        pthread_mutex_unlock(&mutex);
        cnt_local++;    // increase local value

        IsPrime((tid * NUM_INCREASE) + i);
    }

    return (void*)cnt_local;
}

int main(void) {
    pthread_t threads[NUM_THREAD];

    // create threads
    for (long i = 0; i < NUM_THREAD; i++) {
        if (pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0) {
            printf("pthread_create error!\n");
            return 0;
        }
    }

    // wait threads end
    long ret;
    for (int i = 0; i < NUM_THREAD; i++) {
        pthread_join(threads[i], (void**)&ret);
        printf("thread %ld, local count: %ld\n", threads[i], ret);
    }
    printf("global count: %d\n", cnt_global);

    int cnt_prime = 0;
    for (int i = 0; i < NUM_THREAD * NUM_INCREASE; i++) {
        if (is_prime[i] == true) {
            cnt_prime++;
        }
    }

    printf("number of prime: %d\n", cnt_prime);

    return 0;
}

