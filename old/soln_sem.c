// Author: Lucas Wilson
// Email: lkwilson96@gmail.com
// Created for CS370 Honors Assignment

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
//#include <error.h> // commented for Mac OS X support
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

/* Shared memory definitions */
#define SHM_SIZE 1024000
int shmid; // file descriptor (actually shared memory)
void *shm; // shared memory pointer

/* Reader writer definitions */
#define READER 1
#define WRITER 2
pthread_t start_thread(unsigned type); // type specifies reader/writer
pthread_mutex_t rw; // the writers lock
pthread_mutex_t r; // the rc lock
int rc = 0; // count for readers, initially none
void *reader(void*); // the reader thread routine
void read_file(int id); // function to define how the reader reads
void *writer(void*); // the writer thread routine
void write_file(int id); // function to define how the writer writes

/* Used to start and end the demonstration */
// Called after everything is initialized. Implementation for start() is at the bottom after the test functions.
void start();
// Deallocates shared mem regions, etc. Exit code specified by val, but if an
// error occurs during deallocation, val is overwritten by new error code.
void end(int val);

/* Initialize components */
int main() {
    // rand() used by write_file
    srand(time(NULL));
    // Buffers make the output stream of multithreading inaccurate.
    setbuf(stdout, NULL);

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
    } // ELSE: shmid and shm are set and valid

    /* Start implementation */
    start();
    end(0); // clean exit :)
}

void end(int val) {
    /* Detach from shared memory */
    if (shmdt(shm) == -1) {
        perror("Unable to detach from shared memory");
        val = 1;
    }
    /* Delete shared memory */
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Unable to delete shared memory");
        val = 1;
    }
    exit(val);
}

void *reader(void *arg) {
    int id = *((int*)arg);
    printf("[reader: %d] Started.\n", id);

    /* Implementation */
    pthread_mutex_lock(&r);
    if (rc++==0) pthread_mutex_lock(&rw);
    pthread_mutex_unlock(&r);
    read_file(id);
    pthread_mutex_lock(&r);
    if (--rc==0) pthread_mutex_unlock(&rw);
    pthread_mutex_unlock(&r);

    printf("[reader: %d] Ending.\n", id);
    free(arg);
    pthread_exit(NULL);
}

void read_file(int id) {
    printf("[reader: %d] Started reading.\n", id);
    int hash = 0; // calculate hash of memory
    for (unsigned long pos = 0; pos < SHM_SIZE; pos++)
        hash += *((char*)(shm+pos));
    printf("[reader: %d] Reader hash: %d.\n", id, hash);
    printf("[reader: %d] Done reading.\n", id);
}

void *writer(void *arg) {
    int id = *((int*)arg);
    printf("[writer: %d] Started.\n", id);

    /* Implementation */
    pthread_mutex_lock(&rw);
    write_file(id);
    pthread_mutex_unlock(&rw);

    printf("[writer: %d] Ending.\n", id);
    free(arg);
    pthread_exit(NULL);
}

void write_file(int id) {
    printf("[writer: %d] Started writing.\n", id);
    for (unsigned long pos = 0; pos < SHM_SIZE; pos++)
        *((char*)(shm+pos)) = rand();
    printf("[writer: %d] Done writing.\n", id);
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
    //starve_writer();
    x_of_each(10);
}
