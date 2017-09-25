#include <stdio.h>
#include <pthread.h>
#include <math.h>

#define NUM_THREAD  10

int thread_ret[NUM_THREAD];

int range_start;
int range_end;

bool IsPrime(int n) {
    if (n < 2) {
        return false;
    }

    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

void* ThreadFunc(void* arg) {
    long tid = (long)arg;
    
    // Split range for this thread
    int start = range_start + ((range_end - range_start) / NUM_THREAD) * tid;
    int end = range_start + ((range_end - range_start) / NUM_THREAD) * (tid+1);
    if (tid == NUM_THREAD - 1) {
        end = range_end + 1;
    }
    
    long cnt_prime = 0;
    for (int i = start; i < end; i++) {
        if (IsPrime(i)) {
            cnt_prime++;
        }
    }

    thread_ret[tid] = cnt_prime;
        
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREAD];
    
    while (1) {
        // Input range
        scanf("%d", &range_start);
        if (range_start == -1) {
            break;
        }
        scanf("%d", &range_end);

        // Create threads to work
        for (long i = 0; i < NUM_THREAD; i++) {
            if (pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0) {
                printf("pthread_create error!\n");
                return 0;
            }
        }

        // Wait threads end
        for (int i = 0; i < NUM_THREAD; i++) {
            pthread_join(threads[i], NULL);
        }

        // Collect results
        int cnt_prime = 0;
        for (int i = 0; i < NUM_THREAD; i++) {
            cnt_prime += thread_ret[i];
        }
        printf("number of prime: %d\n", cnt_prime);
    }
 
    return 0;
}

