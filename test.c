#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_B = PTHREAD_MUTEX_INITIALIZER;

void* thread1_routine(void* arg) {
    printf("Thread 1: Trying to grab Lock A...\n");
    pthread_mutex_lock(&lock_A);
    printf("Thread 1: Locked A. Now sleeping...\n");
    
    sleep(1); // Give Thread 2 time to grab Lock B

    printf("Thread 1: Trying to grab Lock B...\n");
    pthread_mutex_lock(&lock_B); // Thread 1 will hang here forever
    
    printf("Thread 1: Locked B! (This will never print)\n");
    
    pthread_mutex_unlock(&lock_B);
    pthread_mutex_unlock(&lock_A);
    return NULL;
}

void* thread2_routine(void* arg) {
    printf("Thread 2: Trying to grab Lock B...\n");
    pthread_mutex_lock(&lock_B);
    printf("Thread 2: Locked B. Now sleeping...\n");
    
    sleep(1); // Give Thread 1 time to grab Lock A

    printf("Thread 2: Trying to grab Lock A...\n");
    pthread_mutex_lock(&lock_A); // Thread 2 will hang here forever
    
    printf("Thread 2: Locked A! (This will never print)\n");
    
    pthread_mutex_unlock(&lock_A);
    pthread_mutex_unlock(&lock_B);
    return NULL;
}

int main() {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, thread1_routine, NULL);
    pthread_create(&t2, NULL, thread2_routine, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Main: Finished (This will never happen)\n");
    return 0;
}