#include <pthread.h>
#include <semaphore.h>

#include <stdio.h>
	#include <stdlib.h>

#define BUFFER_SIZE 5

//Global variables
int bufferProducer[BUFFER_SIZE]; /* this data is shared by the thread(s) */
int bufferConsumer[BUFFER_SIZE]; /* this data is shared by the thread(s) */
int in = 0;
int middleProducer = 0;
int middleConsumer = 0;
int out = 0;
//Semaphores and Mutexes: Same scope as global variables
sem_t fullProducer;
sem_t emptyProducer;
sem_t fullConsumer;
sem_t emptyConsumer;

pthread_mutex_t mutexProducer;
pthread_mutex_t mutexMiddle;
pthread_mutex_t mutexConsumer;

//Decaring the fucntions
void *producer(void *param); /* the thread */
void *consumer(void *param);
void *middle(void *param);


int randomVal(int maxR){
		int val = (rand() % maxR)  + 1;
		return val;
		
}

	//TODO: pass index and buffer array by reference 
int insert_item(pthread_t pid, int* buffer, int *index, int inBuffVal,sem_t *full,sem_t *empty,pthread_mutex_t *mutex){
	sem_wait(empty);	
	pthread_mutex_lock (mutex);
	printf("Insert_item inserted item %d at position %d\n",inBuffVal,*index);
	buffer[*index] = inBuffVal;
	(*index)++;
	pthread_mutex_unlock (mutex);
	sem_post(full);
	return 0;
}

//TODO: pass index and buffer array by reference 
int remove_item(pthread_t pid,int* buffer, int *index,sem_t *full,sem_t *empty,pthread_mutex_t *mutex ){
	sem_wait(full);	
	pthread_mutex_lock (mutex);
	int buffer_out = buffer[*index];
	printf("Remove_item removed item %d at position %d\n",buffer_out,*index);
	(*index)++;
	pthread_mutex_unlock (mutex);
	sem_post(empty);
	return buffer_out;
}

void *producer(void *param){
	pthread_t pid = pthread_self();
	while(1){
	int inBuffVal =randomVal(50);
	int sleepT = randomVal(3);
	printf("Producer thread %ld sleeping for %d seconds\n",pid,sleepT);
	sleep(sleepT);
	insert_item(pid,bufferProducer,&in,inBuffVal,&fullProducer,&emptyProducer,&mutexProducer);	
	printf("Producer thread %ld inserted value %d\n",pid,inBuffVal);
	}
}


//TODO: Correctly implement this function.
void *middle(void *param){
	pthread_t mid = pthread_self();
	while(1){
	int sleepT =randomVal(3);
	printf("Middle man thread %ld sleeping for %d seconds\n",mid,sleepT);
	sleep(sleepT);
	pthread_mutex_lock (&mutexMiddle);
	int removed_item = remove_item(mid,bufferProducer,&middleProducer,&fullProducer,&emptyProducer, &mutexProducer);
	insert_item(mid,bufferConsumer,&middleConsumer,removed_item,&fullConsumer,&emptyConsumer,&mutexConsumer);
	pthread_mutex_unlock (&mutexMiddle);
	}
}

void *consumer(void *param){
	pthread_t cid = pthread_self();
	while(1){
	int sleepT =randomVal(3);
	printf("Consumer thread %ld sleeping for %d seconds\n",cid,sleepT);
	sleep(sleepT);
	int item_removed = remove_item(cid,bufferConsumer,&out,&fullConsumer,&emptyConsumer, &mutexConsumer);
	printf("Consumer thread %ld removed value %d\n",cid,item_removed);
    }
}


int main(int argc, char *argv[]){	
	// Initializing main thread
	printf("Main thread beginning\n");
	if (argc != 5) {
		fprintf(stderr,"usage: a.out <sleep time> <num producer threads> <num middleman threads> <num consumer threads>\n");
		return -1;
	}
	//Reading arguments and putting into variables
    int sleep_time = atoi(argv[1]);
    int num_producer_threads = atoi(argv[2]);
    int num_middle_threads = atoi(argv[3]);
    int num_consumer_threads = atoi(argv[4]);
	
	pthread_t proThd[num_producer_threads];
	pthread_t middleThd[num_middle_threads];
	pthread_t conThd[num_consumer_threads];

	printf("Response code for initializing empty semaphore for producer: %d\n",sem_init(&emptyProducer, 0, BUFFER_SIZE));
	printf("Response code for initializing full semaphore for producer: %d\n",sem_init(&fullProducer, 0, 0));
	printf("Response code for initializing empty semaphore for consumer: %d\n",sem_init(&emptyConsumer, 0, BUFFER_SIZE));
	printf("Response code for initializing full semaphore  for consumer: %d\n",sem_init(&fullConsumer, 0, 0));
	pthread_mutex_init(&mutexProducer, NULL);
	pthread_mutex_init(&mutexMiddle, NULL);
	pthread_mutex_init(&mutexConsumer, NULL);

	/* Get the default attributes */
	pthread_attr_t attr; /* set of attributes for the thread */
	pthread_attr_init(&attr);

	/* Creating the producer threads */
	for(int i=0; i<num_producer_threads; i++)
    {
    	pthread_create(&proThd[i], NULL, producer, NULL);
    	printf("Creating producer thread with id %ld\n",proThd[i]);
    }
    /* Creating the middle man threads */
	for(int i=0; i<num_middle_threads; i++)
    {
    	pthread_create(&middleThd[i], NULL, middle, NULL);
    	printf("Creating middleman thread with id %ld\n",middleThd[i]);
    }
    /* Creating the consumer threads */
	for(int i=0; i<num_consumer_threads; i++)
    {
    	pthread_create(&conThd[i], NULL, consumer, NULL);
    	printf("Creating consumer thread with id %ld\n",conThd[i]);
    }

    printf("Main thread sleeping for %d seconds",sleep_time);
	/* Joining all producer threads */
	for(int i=0; i<num_producer_threads; i++)
    {
    	pthread_join(proThd[i],NULL);
    }
    /* Joining all middle threads */
	for(int i=0; i<num_middle_threads; i++)
    {
    	pthread_join(middleThd[i],NULL);
    }
    /* Joining all consumer threads */
	for(int i=0; i<num_consumer_threads; i++)
    {
		pthread_join(conThd[i],NULL);
    }
    sleep(sleep_time);
    //Destroying mutexes and semaphores
	pthread_mutex_destroy(&mutexProducer);
	pthread_mutex_destroy(&mutexConsumer);
	sem_destroy(&fullProducer); // release resources of the semaphore
	sem_destroy(&emptyProducer); // release resources of the semaphore
	sem_destroy(&fullConsumer); // release resources of the semaphore
	sem_destroy(&emptyConsumer); // release resources of the semaphore

	//Exiting main thread
	printf("Main thread exiting");
	pthread_exit(NULL);
	
}