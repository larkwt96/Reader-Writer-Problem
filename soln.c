#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <pthread.h>

/**
 * This program provides a solution to the first reader-writer problem and will
 * exit with a value of 1 on error, 0 otherwise. Appropriate error messages
 * will be printed.
 */

/* Shared memory global definitions */
#define SHM_SIZE 1024000
int shmid; // file descriptor
void *shm; // shared memory pointer

// Function definitions
#define READER 1 // start_thread(READER) is clearer thatn start_tread(1)
#define WRITER 2
void *reader(void*); // the reader thread
void read_file(); // unsynchronized read funciton, used by reader
void *writer(void*); // the writer thread
void write_file(); // unsyncrhonized write funciton
// starts a reader/writer thread and returns the thread id
pthread_t start_thread(unsigned type);
pthread_mutex_t r;
pthread_mutex_t rw;
pthread_mutex_t w;
int rc = 0; // count for readers

/**
 * Terminates program and closes resources. Exits with exit code of val unless
 * an error occurs during the resource closure process, in which case the exit
 * code will be 1 instead of val.
 */
void end(int val);
void start();

/* Begin demonstration */
int main() {
    /* Create and attach to shared memory */
    shmid = shmget(IPC_PRIVATE, SHM_SIZE, S_IRUSR|S_IWUSR);
    if (shmid == -1) {
        perror("Unable to create shared memory");
        end(1);
    }
    shm = shmat(shmid, NULL, 0);
    if (shm == (void*)-1) {
        perror("Unable to attach to shared memory");
        end(1);
    }
    // POST: shmid and shm are set and valid
    /* Start Implementation */
    start();
    /* Detach from shared memory */
    if (shmdt(shm) == -1) {
        perror("Unable to detach from shared memory");
        end(1);
    }
    /* Clean exit :) */
    end(0);
}

void end(int val) {
    /* Delete shared memory */
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Unable to delete shared memory");
        val = 1;
    }
    /* Exit */
    exit(val);
}

void read_file() {

}
void *reader(void *arg) {
    int id = *((int*)arg);
    printf("[reader: %d] Started.\n", id);

    /* Implementation */
    pthread_mutex_lock(&r);
    if (rc++==0) pthread_mutex_lock(&rw);
    pthread_mutex_unlock(&r);
    read_file();
    pthread_mutex_lock(&r);
    if (--rc==0) pthread_mutex_unlock(&rw);
    pthread_mutex_unlock(&r);

    printf("[reader: %d] Ending.\n", id);
    free(arg);
    pthread_exit(NULL);
}
void write_file() {

}
void *writer(void *arg) {
    int id = *((int*)arg);
    printf("[writer: %d] Started.\n", id);

    /* Implementation */
    pthread_mutex_lock(&w);
    pthread_mutex_lock(&rw);
    write_file();
    pthread_mutex_unlock(&rw);
    pthread_mutex_unlock(&w);


    printf("[writer: %d] Ending.\n", id);
    free(arg);
    pthread_exit(NULL);
}

pthread_t start_thread(unsigned type) {
    // counts readers/writers and assigns them a more readable id than tid
    static unsigned rc = 0;
    static unsigned wc = 0;

    // build thread
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr); // defaults are ok

    // start thread
    // It's weird I know. It's for readability in output.
    int *id = malloc(sizeof(*id)); 
    switch (type) {
    case READER:
        *id = ++rc;
        pthread_create(&tid, &attr, reader, id);
        break;
    case WRITER:
        *id = ++wc;
        pthread_create(&tid, &attr, writer, id);
        break;
    }
    return tid;
}

/* TEST FUNCTIONS */
void easy_test() {
    printf("Starting reader thread...\n");
    pthread_t rtid = start_thread(READER);
    printf("Started reader thread.\n");

    printf("Starting writer thread...\n");
    pthread_t wtid = start_thread(WRITER);
    printf("Started writer thread.\n");
    
    printf("Joining reader thread...\n");
    pthread_join(rtid, NULL);
    printf("Joined reader thread.\n");

    printf("Joining writer thread...\n");
    pthread_join(wtid, NULL);
    printf("Joined writer thread.\n");
}

void x_of_each(unsigned count) {
    printf("Starting a total of %u readers and writers.\n", count*2);
    pthread_t threads[count*2];
    for (unsigned i = 0; i < count; i++) {
        threads[i*2]   = start_thread(READER);
        threads[i*2+1] = start_thread(WRITER);
    }
    for (unsigned i = 0; i < count*2; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("All readers and writers' threads are done executing.\n");
}

void starve_writer() {
    printf("Starting 10 readers.\n");
    pthread_t threads[1001];
    for (int i = 0; i < 10; i++) threads[i] = start_thread(READER);
    printf("Starting writer.\n");
    threads[10] = start_thread(WRITER);
    printf("Starting remaining readers.\n");
    for (int i = 11; i < 1001; i++) threads[i] = start_thread(READER);
    for (unsigned i = 0; i < 1001; i++) pthread_join(threads[i], NULL);
}
/* END OF TEST FUNCTIONS */
/**
 * This is where to customize how the implementation is tested. The easiest way
 * is to create a different function for different test cases and just call
 * that function from here.
 */
void start() {
    starve_writer();
}
