/* Group Members:
      Tia Malley - tcm326
      Blake Davis - bd1163
      Rix ...

   Player Two - plays O
 */

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

// check if all 9 spots are filled
// if full, return 1 
// else, return 0
int checkBoardFull(struct shmseg *smap)
{
    int i = 0;
    int j = 0;
    int spacesFilled = 0;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (smap->board[i][j] == 1 || smap->board[i][j] == -1)
            {
                spacesFilled++;
            }
        }
    }
    
    if (spacesFilled == 9)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int tryRowBlock(struct shmseg *smap)
{
  int i;

  for(i = 0; i < 3; i++)
    {
      if(smap->board[i][0] == 1 && smap->board[i][1] == 1 && smap->board[i][2] == 0)
	{
	  // right block
	  return 0;
	}
      if(smap->board[i][0] == 1 && smap->board[i][2] == 1 && smap->board[i][1] == 0)
	{
	  // middle block
	  return 0;
	}
      if (smap->board[i][1] == 1 && smap->board[i][2] == 1 && smap->board[i][0] == 0)
	{
	  // left block
	  return 0;
	}
    }

  // row block not found
  return 1;
}

// function that checks if two X's (1) are found in a row and places an O (-1) to block
int rowBlock(struct shmseg *smap)
{
  int i;

  for(i = 0; i < 3; i++)
    {
      if(smap->board[i][0] == 1 && smap->board[i][1] == 1 && smap->board[i][2] == 0)
	{
	  // right block
	  smap->board[i][2] = -1;
	  return 0;
	}
      if(smap->board[i][0] == 1 && smap->board[i][2] == 1 && smap->board[i][1] == 0)
	{
	  // middle block
	  smap->board[i][1] = -1;
	  return 0;
	}
      if (smap->board[i][1] == 1 && smap->board[i][2] == 1 && smap->board[i][0] == 0)
	{
	  // left block
	  smap->board[i][0] = -1;
	  return 0;
	}
    }

  // row block not found
  return 1;
}

int tryColumnBlock(struct shmseg *smap)
{
  int i;

  for(i = 0; i < 3; i++)
    {
      if(smap->board[0][i] == 1 && smap->board[1][i] == 1 && smap->board[2][i] == 0)
	{
	  // bottom block
	  return 0;
	}
      if(smap->board[0][i] == 1 && smap->board[2][i] == 1 && smap->board[1][i] == 0)
	{
	  // middle block
	  return 0;
	}
      if (smap->board[1][i] == 1 && smap->board[2][i] == 1 && smap->board[0][i] == 0)
	{
	  // top block
	  return 0;
	}
    }

  // column block not found
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
	  smap->board[2][i] = -1;
	  return 0;
	}
      if(smap->board[0][i] == 1 && smap->board[2][i] == 1 && smap->board[1][i] == 0)
	{
	  // middle block
	  smap->board[1][i] = -1;
	  return 0;
	}
      if (smap->board[1][i] == 1 && smap->board[2][i] == 1 && smap->board[0][i] == 0)
	{
	  // top block
	  smap->board[0][i] = -1;
	  return 0;
	}
    }

  // column block not found
  return 1;
}

int tryDiagonalBlock(struct shmseg *smap)
{
  // top left to bottom right block
  if(smap->board[0][0] == 1 && smap->board[1][1] == 1 && smap->board [2][2] == 0)
    {
      // bottom right block
      return 0;
    }
  if(smap->board[0][0] == 1 && smap->board[2][2] == 1 && smap->board[1][1] == 0)
    {
      // center block
      return 0;
    }
  if(smap->board[1][1] == 1 && smap->board[2][2] == 1 && smap->board[0][0] == 0)
    {
      // top left block
      return 0;
    }

  // top right to bottom left block
  if(smap->board[0][2] == 1 && smap->board[1][1] == 1 && smap->board[2][0] == 0)
    {
      // bottom left block
      return 0;
    }
  if(smap->board[0][2] == 1 && smap->board[2][0] == 1 && smap->board[1][1] == 0)
    {
      // center block
      return 0;
    }
  if(smap->board[1][1] == 1 && smap->board[2][0] == 1 && smap->board[0][2] == 0)
    {
      // top right block
      return 0;
    }
  
  // diagonal block not found
  return 1;
}

// function that checks if two X's (1) are found diagonally and places an O (-1) to block
int diagonalBlock(struct shmseg *smap)
{
  // top left to bottom right block
  if(smap->board[0][0] == 1 && smap->board[1][1] == 1 && smap->board [2][2] == 0)
    {
      // bottom right block
      smap->board[2][2] = -1;
      return 0;
    }
  if(smap->board[0][0] == 1 && smap->board[2][2] == 1 && smap->board[1][1] == 0)
    {
      // center block
      smap->board[1][1] = -1;
      return 0;
    }
  if(smap->board[1][1] == 1 && smap->board[2][2] == 1 && smap->board[0][0] == 0)
    {
      // top left block
      smap->board[0][0] = -1;
      return 0;
    }

  // top right to bottom left block
  if(smap->board[0][2] == 1 && smap->board[1][1] == 1 && smap->board[2][0] == 0)
    {
      // bottom left block
      smap->board[2][0] = -1;
      return 0;
    }
  if(smap->board[0][2] == 1 && smap->board[2][0] == 1 && smap->board[1][1] == 0)
    {
      // center block
      smap->board[1][1] = -1;
      return 0;
    }
  if(smap->board[1][1] == 1 && smap->board[2][0] == 1 && smap->board[0][2] == 0)
    {
      // top right block
      smap->board[0][2] = -1;
      return 0;
    }
  
  // diagonal block not found
  return 1;
}

char intToChar(struct shmseg *smap, int i, int j)
{
    if (smap->board[i][j] == 1)
    {
        return 'X';
    }
    else if (smap->board[i][j] == -1)
    {
        return 'O';
    }
    else
    {
        return ' ';
    }
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
            printf("  %c | %c | %c ",  intToChar(smap,a,0), intToChar(smap,a,1), intToChar(smap,a,2));
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

// return 1 - row win not found
// return 0 - row win found
int rowWin(struct shmseg *smap, int num)
{
    int i;
  
    for(i = 0; i < 3; i++)
    {
        // row win found
        if(smap->board[i][0] == num && smap->board[i][0] == smap->board[i][1] && smap->board[i][1] == smap->board[i][2])
	    {
	        return 0;
	    }
    }
    // row win not found
    return 1;
}

int columnWin(struct shmseg *smap, int num)
{
    int i;

    for(i = 0; i < 3; i++)
    {
        // column win found
        if(smap->board[0][i] == num && smap->board[0][i] == smap->board[1][i] && smap->board[1][i] == smap->board[2][i])
	    {
	        return 0;
	    }
    }
    // column win not found
    return 1;
}

int diagonalWin(struct shmseg *smap, int num)
{
    // top left to bottom right diagonal win
    if(smap->board[0][0] == num && smap->board[0][0] == smap->board [1][1] && smap->board[1][1] == smap->board[2][2])
    {
        return 0;
    }
  
    // bottom left to top right diagonal win
    if(smap->board[2][0] == num && smap->board[2][0] == smap->board[1][1] && smap->board[1][1] == smap->board[0][2])
    {
        return 0;
    }
  
    return 1;
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
  int rblockFound, cblockFound, dblockFound;
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

  // 8 - Attach the shared memory segment
  smap = shmat(shmid, NULL, 0);
  if(smap == (void *) -1)
    {
      checkError(-1, "shmat");
    }
  
  // 9 - Enter the game play loop
  while(smap->counter != -1)
    {

      // 1 - reserve player 2's semaphore
      checkError(reserveSem(semid, 1), "reserveSem");

      // 3 - if the turn counter is -1, exit the loop
      if (smap->counter == -1)
	{
	  break;
	}
      
      // 2 - display the state of the game board
      printf("Player 1 Move\n");
      printBoard(smap);
	  
      // 4 - make players 2 move
      // checks to see if player two needs to block player one
      // if block is found O (-1) is placed in the functions
      rblockFound = tryRowBlock(smap);
      cblockFound = tryColumnBlock(smap);
      dblockFound = tryDiagonalBlock(smap);

      if(rblockFound == 0)
	{
	  rowBlock(smap);
	}
      else if(cblockFound == 0)
	{
	  columnBlock(smap);
	}
      else if(dblockFound == 0)
	{
	  diagonalBlock(smap);
	}

      // no blocks found - randomly places O
      if(rblockFound == 1 && cblockFound == 1 && dblockFound == 1)
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

  printf("Exiting...\n");
  exit(EXIT_SUCCESS);
}
