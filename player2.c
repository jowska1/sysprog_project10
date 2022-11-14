// Player Two

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include "binary_sem.h"

// function provided by Mr. Knight in guided exercise 11
// checks if an error occured, if one has prints error message
int checkError(int e, const char *str)
{
  if(e == -1)
    {
      if(errno == EINTR) return e;
      perror(str);
      exit(EXIT_FAILURE);
    }
  return e;
}

// block of shared memory
struct shmseg
{
  int counter;
  int board[3][3][3];
};

int main(int argc, char *argv[])
{
  struct shmseg *smap;
  int fd;
  int num1, num2;
  int semid, shmid;
  key_t semK, shmK;

  // 1 - checks to see if FIFO exists - if equal to -1 mkfifo has failed
  if(mkfifo("xoSync", S_IRWXU) == -1)
    {
      // checks to see if the FIFO doesn't already exists
      if(errno != EEXIST)
	{
	  perror("mkfifo producer");
	  exit(EXIT_FAILURE);
	}
    }

  // 2 - open FIFO xoSync for read
  checkError(fd = open("FIFOstring", O_RDONLY), "open consumer");
  
  // 3- reading string from the FIFO
  checkError(read(fd, &num1, sizeof(int)), "read num");
  checkError(read(fd, &num2, sizeof(int)), "read num");

  // 5 - close FIFO
  close(fd);

  // 6 - Generate System V keys with ftok
  // first number uses for shared memory
  shmK = ftok(".", num1);
  // second number used for semaphores
  semK = ftok(".", num2);

  // 7 - retrieve the shared memory and the semaphore set create by player 1
  checkError(semid = semget(semK, 0, 0), "semget");
  checkError(shmid = shmget(shmK, 0, 0), "shmget");

  // 8 - Attach the shared memory segment
  smap = shmat(shmid, NULL, 0);
  if(smap == (void *) -1)
    {
      checkError(-1, "shmat");
    }
  
  // 9 - Enter the game play loop
  while(true)
    {
      // 1 - reserve player 2's semaphore
      checkError(reserveSem(semid, 1), "reserveSem");

      // 2 - display the state of the game board

      // 3 - if the turn counter is -1, exit the loop
      if (counter == -1)
	{
	  
      // 4 - make players 2 move
      // logic goes here

      // 5 - display the state of the game board

      // 6 - increment the game turn by 1

      // 7 - release player 1's semaphore
    }
  
  // 10 - Open the FIFO xoSync for read
  checkError(fd = open("FIFOstring", O_RDONLY), "open consumer");
  
  // 11 - Close the FIFO
  close(fd);
  
  // 12 - Detach the segment of shared memory
  checkError(shmdt(smap), "shmdt");

  exit(EXIT_SUCCESS);
}
