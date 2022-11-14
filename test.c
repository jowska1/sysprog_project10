#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>

//#include "semun.h"

// Program 10 Testing

// System V is an IPC facility
// Every System V IPC facility has a
// header file, data structure, get function, control operation
// and a key (analogous to a file descriptor)

// A semaphore is a kernel maintained unsigned integer whose value is 
// restricted to being greater than or equal to 0
// Changes are Atomic - meaning if one thread/process wants to
// increment the integer and another thread/process wants to
// decrement the integer, these operations CANNOT interrupt each other
// Binary semaphores can be used like mutexes
// Unlike mutexes, semaphores are not "owned" by a thread/process

union semun 
{
    int val;                 /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

void assertError(int e, const char *str)
{
    if (e == -1)
    {
        perror(str);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    // Attempt to create FIFO called xoSync
    if (mkfifo("xoSync", S_IRWXU) == -1)
    {
        // If the error isn't EEXIST
        if (errno != EEXIST)
        {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
        printf("FIFO already exists\n");
    }
    else {printf("FIFO created\n");}

    // Seed time
    time_t t;
    srand((unsigned) time(&t));

    // Generate 2 random numbers for creating System V keys
    // Generate 2 values between 10-99
    int v1 = rand() % (99 + 1 - 10) + 10;
    int v2 = rand() % (99 + 1 - 10) + 10;

    // Semaphore setup
    // TODO understand
    key_t key;
    int semid;
    int perms = S_IRUSR | S_IWUSR;

    struct sembuf sops;
    union semun arg;
    struct timespec ts;

    // Generate the System V keys with ftok using the random numbers and the FIFO xoSync. 
    // Use the first value generated as the projection value for shared memory
    // and the second value generated as the projection value for the semaphores.

    // TODO understand this
    // Create key?
    if (ftok("xoSync",v1) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    
    // Create the block of shared memory
    struct sharedMem
    {
        // TODO these need more descriptive names
        int value;
        int array[3][3][3];

    } mySharedMem;

    // Create a semaphore set with a size of 2; 
    // make certain to give it proper access permissions as well. 

    assertError(semid = semget(key, 2, IPC_CREAT | perms), "semget");

    // Initialize the semaphores in the semaphore set.
    // Semaphore 0 represents player 1; Semaphore 1 represents player 2. 
    // Set the player 1 semaphore to available and the player 2 semaphore to in use.

    // What does "available" mean in this context?
    arg.val = 3;
    assertError(semctl(semid, 0, SETVAL, arg), "semctl 1");


    



    // Delete semaphore set before exiting
    assertError(semctl(semid, 0, IPC_RMID), "semctl");

    exit(EXIT_SUCCESS);
}