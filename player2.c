#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include "binary_sem.h"
#include "semun.h"

// block of shared memory
struct shmseg
{
  int counter;
  int board[3][3];
};

// function that checks if two X's (1) are found in a row and places an O (-1) to block
int rowBlock(struct shmseg *smap)
{
  int i;

  for(i = 0; i < 3; i++)
    {
      if(smap->board[i][0] == 1 && smap->board[i][1] == 1 && smap->board[i][2] == 0)
	{
	  // right block
	  smap->board[i][2] == -1;
	  return 0;
	}
      if(smap->board[i][0] == 1 && smap->board[i][2] == 1 && smap->board[i][1] == 0)
	{
	  // middle block
	  smap->board[i][1] == -1;
	  return 0;
	}
      if (smap->board[i][1] == 1 && smap->board[i][2] == 1 && smap->board[i][0] == 0)
	{
	  // left block
	  smap->board[i][0] == -1;
	  return 0;
	}
    }

  // row block not found
  return 1;
}

// function that checks if two X's (1) are found in a column and places an O (-1) to block
int columnBlock(struct shmseg *smap)
{
    int i;

  for(i = 0; i < 3; i++)
    {
      if(smap->board[0][i] == 1 && smap->board[1][i] == 1 && smap->board[2][i] == 0)
	{
	  // bottom block
	  smap->board[2][i] == -1;
	  return 0;
	}
      if(smap->board[0][i] == 1 && smap->board[2][i] == 1 && smap->board[1][i] == 0)
	{
	  // middle block
	  smap->board[1][i] == -1;
	  return 0;
	}
      if (smap->board[1][i] == 1 && smap->board[2][i] == 1 && smap->board[0][i] == 0)
	{
	  // top block
	  smap->board[0][i] == -1;
	  return 0;
	}
    }

  // column block not found
  return 1;
}

// function that checks if two X's (1) are found diagonally and places an O (-1) to block
int diagonalBlock(struct shmseg *smap)
{
  // top left to bottom right block
  if(smap->board[0][0] == 1 && smap->board[1][1] == 1 && smap->board [2][2] == 0)
    {
      // bottom right block
      smap->board[2][2] == -1;
      return 0;
    }
  if(smap->board[0][0] == 1 && smap->board[2][2] == 1 && smap->board[1][1] == 0)
    {
      // center block
      smap->board[1][1] == -1;
      return 0;
    }
  if(smap->board[1][1] == 1 && smap->board[2][2] == 1 && smap->board[0][0] == 0)
    {
      // top left block
      smap->board[0][0] == -1;
      return 0;
    }

  // top right to bottom left block
  if(smap->board[0][2] == 1 && smap->board[1][1] == 1 && smap->board[2][0] == 0)
    {
      // bottom left block
      smap->board[2][0] == -1;
      return 0;
    }
  if(smap->board[0][2] == 1 && smap->board[2][0] == 1 && smap->board[1][1] == 0)
    {
      // center block
      smap->board[1][1] == -1;
      return 0;
    }
  if(smap->board[1][1] == 1 && smap->board[2][0] == 1 && smap->board[0][2] == 0)
    {
      // top right block
      smap->board[2][0] == -1;
      return 0;
    }
  
  // diagonal block not found
  return 1;
}

// function to print the tic-tac-toe board
void printBoard(struct shmseg *smap)
{
    int iteration = 6;
    int a = 0;
    for(int i = 1; i <= iteration; i++)
    {
        
        if (i % 2 != 0 )
        {
            printf("  %d | %d | %d ", smap->board[a][0],smap->board[a][1],smap->board[a][2]);
            a++;
        }
        else if (i == 6)
        {
            printf("\n");
        }
        else
        {
            printf("\n ---|---|---\n");
        }
    }

    printf("\n");
}

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

int main(int argc, char *argv[])
{
  struct shmseg *smap;
  int fd;
  int num1, num2, x, y, empty = 1;
  int semid, shmid;
  int rblock, cblock, dblock;
  key_t semK, shmK;
  time_t t;

  srand((unsigned) time(&t));

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
  checkError(fd = open("xoSync", O_RDONLY), "open FIFO");
  
  // 3- reading string from the FIFO
  checkError(read(fd, &num1, sizeof(num1)), "read num");
  checkError(read(fd, &num2, sizeof(num2)), "read num");

  // 5 - close FIFO
  close(fd);

  // 6 - Generate System V keys with ftok
  // first number uses for shared memory
  shmK = ftok("xoSync", num1);
  if (shmK == -1)
    {
        perror("ftok1");
        exit(EXIT_FAILURE);
    }
  // second number used for semaphores
  semK = ftok("xoSync", num2);
  if (semK == -1)
    {
        perror("ftok1");
        exit(EXIT_FAILURE);
    }

  // 7 - retrieve the shared memory and the semaphore set create by player 1
  checkError(semid = semget(semK, 0, 0), "semget");
  checkError(shmid = shmget(shmK, 0, 0), "shmget");

  printf("DEBUG passed shared mem retrieval\n");

  // 8 - Attach the shared memory segment
  smap = shmat(shmid, NULL, 0);
  if(smap == (void *) -1)
    {
      checkError(-1, "shmat");
    }
  
  // 9 - Enter the game play loop
  while(1)
    {
      printf("DEBUG entering game loop\n");

      // 1 - reserve player 2's semaphore
      checkError(reserveSem(semid, 1), "reserveSem");
      
      // 2 - display the state of the game board
      printf("Player 1 Move\n");
      printBoard(smap);
      
      // 3 - if the turn counter is -1, exit the loop
      if (smap->counter == -1)
	{
	  exit(EXIT_SUCCESS);
	}
	  
      // 4 - make players 2 move
      // checks to see if player two needs to block player one
      // if block is found O (-1) is placed in the functions
      rblock = rowBlock(smap);
      cblock = columnBlock(smap);
      dblock = diagonalBlock(smap);

      // no blocks found - randomly places O
      if(rblock == 1 || cblock == 1 || dblock == 1)
	{
	  // do..while loop that generates two random numbers between 0 and 2
	  // then checks if that space on the board is empty
	  // if empty... space stores -1 (for O)
	  // if not... repeats steps until empty space is found
	  do {
	    x = rand() % (2 + 1) + 0;
	    y = rand() % (2 + 1) + 0;

	    if(smap->board[x][y] == 0)
	      {
		smap->board[x][y] = -1;
		empty = 0;
	      }
	    
	  }while(empty == 1);	    
	}

      // 5 - display the state of the game board
      printf("Player 2 Move\n");
      printBoard(smap);
      
      // 6 - increment the game turn by 1
      smap->counter++;
      // 7 - release player 1's semaphore
      checkError(releaseSem(semid, 0), "releaseSem");
    }
  
  // 10 - Open the FIFO xoSync for read
  checkError(fd = open("xoSync", O_RDONLY), "open consumer");
  
  // 11 - Close the FIFO
  close(fd);
  
  // 12 - Detach the segment of shared memory
  checkError(shmdt(smap), "shmdt");

  exit(EXIT_SUCCESS);
}
