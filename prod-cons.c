#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

typedef int buffer_item;
#define PROD_SLEEP 	1
#define CONS_SLEEP 	1
#define BUFFER_SIZE 5
#define MAX_PROD	20
#define MAX_CONS	20

buffer_item buffer[BUFFER_SIZE];
unsigned int buff_idx;

int insert_item(buffer_item item) {
	if (buff_idx == BUFFER_SIZE) {
		/* what happens if space is not available? */
		return -1;
	}

	/* insert an item into buffer */
	buffer[buff_idx] = item;
	buff_idx++;
	return 0;
}

int remove_item(buffer_item *item) {
	if (buff_idx == 0) {
		/* what happens if the buffer is empty */
		return -1;
	}
	/* remove an item from the buffer */
	*item = buffer[buff_idx - 1];
	buff_idx--;
	return 0;
}

void * producer(void * param) {
	buffer_item item;
	
	while (1) {
		/* sleep a random time period */
		//sleep(PROD_SLEEP);

		/* generate a random item */
		item = rand();
		
		if (insert_item(item) == -1) {
			fprintf(stderr,"buffer is full, cannot write\n");
		} else {
			printf("Producer produced %d\n",item);
		}
	}
}

void * consumer(void * param) {
	buffer_item item;

	while (1) {
		/* sleep a random time period */
		//sleep(CONS_SLEEP);
		
		/* generate a random item */
		if (remove_item(&item) == -1) {
			fprintf(stderr,"buffer is empty, cannot read\n");
		} else {
			printf("Consumer consumed %d\n",item);
		}
	}
}

int main(int argc, char * argv[]) {
	int i;
	pthread_t prod_tid[MAX_PROD];
	pthread_t cons_tid[MAX_CONS];

	/* 1. get cmd line arguments */
	long int num_sec = atoi(argv[1]);
	long int num_prod = atoi(argv[2]);
	long int num_cons = atoi(argv[3]);

	buff_idx = 0;

	/* 2. Initialize buffer */
	memset((void*)buffer, 0, BUFFER_SIZE*sizeof(buffer_item));

	/* 3. Create producer threads */
	for (i = 0; i < num_prod; i++) {
		pthread_create(&prod_tid[i], NULL, producer, NULL);
	}

	/* 4. Create consumer threads */
	for (i = 0; i < num_cons; i++) {
		pthread_create(&cons_tid[i], NULL, consumer, NULL);
	}

	/* 5. Sleep */
	sleep(num_sec);
	
	/* 6. Exit */
	return 0;
}