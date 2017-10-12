#include <stdio.h>
#include <pthread.h>

int var = 0;

void * thread_func(void * arg) {
	var ++;
	return NULL;
}
int main (int argc, char * argv[] ) {
	pthread_t child;
	pthread_create(&child, NULL, thread_func, NULL);
	var++;
	pthread_join(child, NULL);
	
	return 0;
}
