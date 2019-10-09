#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>

#define BUFFER_LENGTH 100
long long *buffer;
pthread_mutex_t *work_mutex;
sem_t *element_sem, *empty_sem;
long long *result;
int *buffer_indice;

void *createSharedMemory(size_t size) {
    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    if (addr == MAP_FAILED) {
        return NULL;
    }
    return addr;
}

void freeSharedMemory(void *addr, size_t size)
{
    if (munmap(addr, size) == -1) {
        printf("munmap(%p, %d) failed", addr, (int)size);
    }
}

int main ()   
{   
    // init elements in shared memory
    buffer = createSharedMemory(sizeof(long long) * BUFFER_LENGTH);
    result = createSharedMemory(sizeof(long long));
    buffer_indice = createSharedMemory(sizeof(int));
    element_sem = createSharedMemory(sizeof(sem_t));
    empty_sem = createSharedMemory(sizeof(sem_t));
    work_mutex = createSharedMemory(sizeof(pthread_mutex_t));

    // time stamp
    struct timespec ts;
    *buffer_indice = 0;
    *result = 0;

    // init a mutex
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
    int mutex_res = pthread_mutex_init(work_mutex, &mutexattr);    
    if (mutex_res != 0) {
        perror("Mutex initialization failed\n");
        exit(1);
    }

    // init two semaphores
    int sem_res_ele = sem_init(element_sem, 4, 0);
    int sem_res_emp = sem_init(empty_sem, 4, BUFFER_LENGTH);
    if (sem_res_ele != 0 || sem_res_emp != 0) {
        perror("Semaphore initialization failed\n");
        exit(1);
    }

    // fork the first time, only test whether fork is successful
    pid_t fpid1 = fork(); 
    if (fpid1 < 0){
        perror("error in fork!\n");
        exit(1);
    }
          
    // fork the second time, children for producers, fathers for consumers
    pid_t fpid2 = fork();
    if (fpid2 < 0){
        perror("error in fork!\n");
        exit(1);
    } else if (fpid2 == 0) {  

        printf("I am the child process, a producer\n");
        // producers' logic
        long long i;
        for (i = 1; i <= 1000; i++){
            /*
            printf("i = %lld\n", i);
            int test;
            sem_getvalue(empty_sem, &test);
            printf("empty before wait %d \n", test);
            */
            sem_wait(empty_sem);
            pthread_mutex_lock(work_mutex);
            buffer[*buffer_indice] = i;
            // printf("value sent %lld, buffer indice %d\n", buffer[*buffer_indice], *buffer_indice);
            (*buffer_indice) += 1;
            pthread_mutex_unlock(work_mutex);
            sem_post(element_sem);
            
            usleep(1000);
        }
        
    }  
    else {  
        printf("I am the father process, a consumer\n");  
        long long local_result = 0;
        // consumers' logic
        while (1){
            if ( clock_gettime(CLOCK_REALTIME, &ts ) < 0){
                perror("error in getting time!\n");
                exit(1);
            }
            ts.tv_sec += 5; // wait for 5 seconds before break

            int timed_res = sem_timedwait(element_sem, &ts);
            if (timed_res == -1 && errno == ETIMEDOUT) {
                printf("Consumer TIMEOUT!\n");
                break; 
            }
            pthread_mutex_lock(work_mutex);
            // printf("value received %lld, buffer indice %d\n", buffer[*buffer_indice], *buffer_indice);
            *(buffer_indice) -= 1;
            local_result += buffer[*buffer_indice];            
            pthread_mutex_unlock(work_mutex);
            sem_post(empty_sem);
        }
        printf("The local result: %lld\n", local_result);
        *result += local_result;
        printf("The final result: %lld\n", *result);
    }

    // destroy semsphore. mutex and shared momory
    sem_destroy(empty_sem); 
    sem_destroy(element_sem); 
    pthread_mutex_destroy(work_mutex);
    freeSharedMemory(buffer, sizeof(long long) * BUFFER_LENGTH);
    freeSharedMemory(result, sizeof(long long));
    freeSharedMemory(buffer_indice, sizeof(int));
    freeSharedMemory(element_sem, sizeof(sem_t));
    freeSharedMemory(empty_sem, sizeof(sem_t));
    freeSharedMemory(work_mutex, sizeof(pthread_mutex_t));
      
    return 0;  
}  