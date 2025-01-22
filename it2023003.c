/*
 * Project assigment for the course Operating Systems at the Harokopio University of Athens
 * 
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * 
 * 
 * 
 * 
 * 
*/



#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NUM_THREADS 10

typedef struct {
    int file;                // File 
    pthread_mutex_t* mutex;    // Mutex lock
    int lineNumber;            // line that will be read by thread
} ThreadParams;

// Define PAGE_SIZE as a constant
#define page_size sysconf(_SC_PAGESIZE) /1024

// golbal variables for metrics 
int totalCPU = 0;
int totalMEM = 0;
int averageRSS = 0;
int averageVSS = 0;



void *thread_function(void *arg){
    ThreadParams* p = (ThreadParams*) arg;
    printf("Thread %d\n", p->lineNumber); // DEBUG REMOVE LATER
    int pid, ppid, rss, vsz; // locaal variables for this processes 
    pthread_mutex_lock(p->mutex); // start mutex
    
    pthread_mutex_unlock(p->mutex);
    
    
    printf("Stats for pid: %d, RSS pages: %d, VSZ pages: %d", pid, rss, vsz );
    free(p); // Free the allocated memory for this thread's parameters
    return NULL;
}

int main(void){
    pthread_mutex_t glb_variable_mutex = PTHREAD_MUTEX_INITIALIZER;

    printf("Retrieving top 10 proccesses, %ld \n", page_size);
    int fd;
    if ((fd=open("processes.txt", O_WRONLY))==-1)
	{
		perror("open");
		exit(1);
    }
    pthread_t threads[NUM_THREADS]; // array of threads
    
    for (int i = 0; i < NUM_THREADS; i++){
        ThreadParams* params = malloc(sizeof(ThreadParams)); 
        params->file = fd;
        params->mutex = &glb_variable_mutex;
        params->lineNumber = i + 1;
        pthread_create(&threads[i], NULL, thread_function, params);
    }
    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    close(fd);
    return 0;


}
