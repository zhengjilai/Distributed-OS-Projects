#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>

// init a mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tag = PTHREAD_MUTEX_INITIALIZER;
// init a conditional variable
// pthread_cond_t cond_can_produce = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_can_consume = PTHREAD_COND_INITIALIZER;
 
void consumer(void*);
void producer(void*);
// buffer
#define BUFFER_LENGTH 100
long long buffer[BUFFER_LENGTH];
int buffer_indice = 0;
// final result
long long result = 0;

#define PRODUCER_NUM 2
#define CONSUMER_NUM 2
#define NUM_CONSUME_ONCE 2
int producer_tag = 0;
 
int main(void){
    // threads
    pthread_t consumers[CONSUMER_NUM];
    pthread_t producers[PRODUCER_NUM];
 
    // Create threads
    for (int i = 0; i < CONSUMER_NUM; i++) {
        pthread_create(&consumers[i],NULL,(void *)consumer,(void*)NULL);        
    }
    for (int i = 0; i < PRODUCER_NUM; i++) {
        pthread_create(&producers[i],NULL,(void *)producer,(void*)NULL);
    }

    // wait producers' threads end
    for (int i = 0; i < CONSUMER_NUM; i++) {
        pthread_join(consumers[i],NULL);       
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_can_consume);
    printf("Final result: %lld", result);
    exit(0);
}

void producer(void *junk){

    for( long long i = 1; i <= 1000000; i++){
        pthread_mutex_lock(&mutex);
        if (buffer_indice < BUFFER_LENGTH){
            //printf("value sent %lld, buffer indice %d\n", i, buffer_indice);
            pthread_mutex_lock(&mutex_tag);
            buffer[buffer_indice++] = i;
            pthread_mutex_unlock(&mutex_tag);
            if (++producer_tag == NUM_CONSUME_ONCE){
                producer_tag = 0;
                pthread_cond_signal(&cond_can_consume);     
            } 
        }
        pthread_mutex_unlock(&mutex); 
        //pthread_mutex_lock(&mutex_tag);
        //pthread_mutex_unlock(&mutex_tag);
        usleep(0);
    }
    printf("producer exit.\n");
    return;
}
 
void consumer(void*junk){
    long long local_result = 0;
    // time stamp
    struct timespec ts;
    while(1)
    {
        if ( clock_gettime(CLOCK_REALTIME, &ts ) < 0){
            perror("error in getting time!\n");
            exit(1);
        }
        ts.tv_sec += 5; // wait for 5 seconds before break
        pthread_mutex_lock(&mutex);
        int timed_res = pthread_cond_timedwait(&cond_can_consume, &mutex, &ts); // wait
        if (timed_res == ETIMEDOUT) {
            pthread_mutex_unlock(&mutex);
            printf("Consumer TIMEOUT!\n");
            break; 
        } else if (timed_res != 0){
            printf("Consumer not running well.\n");
            break;
        }
        buffer_indice -= NUM_CONSUME_ONCE;
        for (int j = 0; j < NUM_CONSUME_ONCE; j++){
            local_result += buffer[buffer_indice + j];
        }
        /**
        printf("value received %lld and %lld, buffer indice %d and %d\n", 
                  buffer[buffer_indice], buffer[buffer_indice+1], buffer_indice, buffer_indice+1);
        **/
        while(buffer_indice >= 1){
            buffer_indice--;
            local_result += buffer[buffer_indice];
            printf("value received %lld, buffer indice %d\n", buffer[buffer_indice],buffer_indice);
        }
        pthread_mutex_unlock(&mutex);
        // pthread_cond_broadcast(&cond_can_produce);
    }
    printf("Local result: %lld\n", local_result);
    result += local_result;
    return;
}