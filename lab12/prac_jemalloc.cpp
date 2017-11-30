#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// allocate ~640MB
#define MEMORY_BLOCK_SIZE   16
#define NUM_BLOCK           40000000

int block_per_thread;

void* ThreadFunc(void* args) {
    for (int i = 0; i < block_per_thread; i++) {
        malloc(MEMORY_BLOCK_SIZE);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    int num_thread = atoi(argv[1]);
    block_per_thread = NUM_BLOCK / num_thread;

    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t*) * num_thread);

    for (int i = 0; i < num_thread; i++) {
        pthread_create(&threads[i], 0, ThreadFunc, NULL);
    }
    for (int i = 0; i < num_thread; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);

    return 0;
}

