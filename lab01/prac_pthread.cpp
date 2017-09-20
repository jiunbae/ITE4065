#include <stdio.h>
#include <pthread.h>

#define NUM_THREAD 	10
#define NUM_INCREASE 100000

int cnt_global = 0;

void* ThreadFunc(void * arg) {
	long cnt_local = 0;
	for (int i = 0;  i<  NUM_INCREASE; i ++) {
		cnt_global++;
		cnt_local++;
	}
	return (void*)cnt_local;
}

int main (void ){
	pthread_t threads[NUM_THREAD];
	
	for (int i = 0; i < NUM_THREAD; i ++) {
		if (pthread_create(&threads[i], 0, ThreadFunc, NULL) < 0) {
			printf("ptrhead_create error!\n");
			return -1;
		}
	}

	long ret = 0;
	for (int i = 0; i < NUM_THREAD; i++) {
		pthread_join(threads[i], (void**)&ret);
		printf("thread %ld, local count: %ld\n", threads[i], ret);
	}
	printf("global count: %d\n", cnt_global);
	return 0;
}
