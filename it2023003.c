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
#include <fcntl.h>
#include <string.h>

#define NUM_THREADS 10



// Define page_size as a constant
#define page_size (sysconf(_SC_PAGESIZE)/1024 )

// golbal variables for metrics 
float totalCPU = 0.0;
float totalMEM = 0.0;
float averageRSS = 0;
float averageVSZ = 0;


typedef struct {
    int file;                        // File 
    pthread_mutex_t* file_mutex;     // Mutex lock for the file
    pthread_mutex_t* variable_mutex; // Mutex lock for the global variables
    int lineNumber;                  // line that will be read by thread
} ThreadParams;

char* read_line(ThreadParams* params){
    while (1) {
        size_t buffer_size = 256;          // Initial buffer size
        char *line = malloc(buffer_size);  // Dynamically allocate the buffer
        if (!line) {
            perror("malloc");
            return NULL;
        }
        size_t len = 0;
        char c;
        while (read(params->file, &c, 1) == 1) {
            // Resize the buffer if needed
            if (len + 1 >= buffer_size) {
                buffer_size *= 2;
                char *new_line = realloc(line, buffer_size);
                if (!new_line) {
                    perror("realloc");
                    free(line);
                    return NULL;
                }
                line = new_line;
            }

            line[len++] = c;
            if (c == '\n') {
                break;
            }
        }

        if (len == 0) {
            // End of file or no more lines to read
            free(line);
            return NULL;
        }
        line[len] = '\0'; // Null-terminate the line
        return line;
    }    
}
void change_glb_variables(char* cpu, char* mem, char* rss, char* vsz){
    //convert string to float
    totalCPU = totalCPU + atof(cpu);
    totalMEM = totalMEM + atof(mem);
    
    // Divide the values by page size
    int rss_pages = (atoi(rss) / page_size);
    int vsz_pages = (atoi(vsz) / page_size);

    // Update running averages
    averageRSS += (float)rss_pages / NUM_THREADS;
    averageVSZ += (float)vsz_pages / NUM_THREADS;
}

void *thread_function(void *arg){

    ThreadParams* params = (ThreadParams*) arg;
    char *pid = NULL, *stat = NULL, *ppid = NULL;
    char *cpu = NULL, *mem = NULL;
    char *rss = NULL, *vsz = NULL;
        pthread_mutex_lock(params->file_mutex);
        char* line = read_line(params); 
        pthread_mutex_unlock(params->file_mutex);

        // Divide the line to its componets
        char *token = strtok(line, " \t");
        int col = 0;
        while (token != NULL) {
            col++;
            if (col == 1) pid = token;  // PID column
            if (col == 2) cpu = token;  // CPU column
            if (col == 3) mem = token;  // Memory column
            if (col == 4) rss = token;  // RSS column
            if (col == 5) vsz = token;  // VRZ column
            if (col == 6) stat = token; // STAT column
            if (col == 7) ppid = token; // PPPID column
            token = strtok(NULL, " \t");
        }
        // change global varibales 
        pthread_mutex_lock(params->variable_mutex);
        change_glb_variables(cpu, mem, rss, vsz);
        pthread_mutex_unlock(params->variable_mutex);

        printf("Stats for pid: %s, RSS pages: %s, VSZ pages: %s \n", pid, rss, vsz );

    
        free(line); // Free the dynamically allocated line buffer
    
    free(params);
    return NULL;
}

int main(void){
    pthread_mutex_t glb_variable_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;


    printf("\n\n\nRetrieving top 10 proccesses\n");
    int fd;
    if ((fd=open("processes.txt", O_RDONLY))==-1)
	{
		perror("open");
		exit(1);
    }
    // Skip the first line (move past the first newline character)
    char c;
    while (read(fd, &c, 1) == 1) {
        if (c == '\n') {
            break;  // Stop after the first line
        }
    }
    pthread_t threads[NUM_THREADS]; // array of threads
    printf("Current top 10 resource metrics:\n");
    for (int i = 0; i < NUM_THREADS; i++){
        ThreadParams* params = malloc(sizeof(ThreadParams)); 
        params->file = fd;
        params->file_mutex = &file_mutex;
        params->variable_mutex = &glb_variable_mutex;
        params->lineNumber = i + 1;
        pthread_create(&threads[i], NULL, thread_function, params);
    }
    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    printf("\nSUM CPU%%: %.2f\nSUM MEM%%: %.2f\nRSS AVERAGE: %.2f\nVSS AVERAGE: %.2f\n", totalCPU, totalMEM, averageRSS, averageVSZ);
    close(fd);
    return 0;
}
