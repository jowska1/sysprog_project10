/* Group Members:
      Tia Malley - tcm326
      Blake Davis - bd1163
      Rix ...

   Player One - plays X
   Goal is to win.
 */

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
#include <sys/shm.h>
#include "binary_sem.h"
#include "semun.h"

#define BUF_SIZE 1024
#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

// block of shared memory
struct shmseg
{
    int counter;
    int board[3][3];
};

// function that fills the board with 0's (blank spaces) before the game is started
void resetBoard(struct shmseg *smap)
{
    int i = 0;
    int j = 0;
  
    for (i = 0; i <= 3; i++)
    {
        for (j = 0; j <= 3; j++)
        {
	        smap->board[i][j] = 0;
        }
    }
}

// check if all 9 spots are filled
// if full, return 0
// else, return 1
int checkBoardFull(struct shmseg *smap)
{
    int i = 0;
    int j = 0;
    int spacesFilled = 0;

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            if (smap->board[i][j] == 0)
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

// function that checks if a specific board spot is empty
// if empty, places a 1 (X) in the spot - if not empty, does nothing
int tryPlace(struct shmseg *smap, int x, int y)
{
    if (smap->board[x][y] == 0)
    {
        smap->board[x][y] = 1;
        return 0;
    }
    else {return 1;}
}

// tryRow, tryColumn, and tryDiagonal are helper functions for p1Move
// that try to complete a row/column/diagonal for a win
// Each function recieves coordinates for 2 spaces, checks if X is placed in them,
// then tries to place in the winning position between the 2 spaces

// look for winning row placement and place if so
// 3 rows: 0,0 and 0,2 - 1,0 and 1,2 - 2,0 and 2,0 - 2,2
int tryRow(struct shmseg *smap, int x1, int y1, int x2, int y2)
{
    // check x1, y1 for X
    if (smap->board[x1][y1] != 1)
    {
        return 1;
    }
    // check x2, y2 for X
    if (smap->board[x2][y2] != 1)
    {
        return 1;
    }

    // for any row, x1 should equal winX
    int winX = x1;
    // and winY will always be 1
    int winY = 1;

    // if winning place is available
    if (smap->board[winX][winY] == 0)
    {
        // place
        smap->board[winX][winY] = 1;
        return 0;
    }

    // if we can't place
    return 1;
}

// look for winning column placement and place if so
// 3 columns: 0,0 and 2,0 - 0,1 and 2,1 - 0,2 and 2,2
int tryColumn(struct shmseg *smap, int x1, int y1, int x2, int y2)
{
    // check x1, y1 for X
    if (smap->board[x1][y1] != 1)
    {
        return 1;
    }
    // check x2, y2 for X
    if (smap->board[x2][y2] != 1)
    {
        return 1;
    }

    // for any column, winX will always be 1
    int winX = 1;
    // and winY should equal y1
    int winY = y1;

    // if winning place is available
    if (smap->board[winX][winY] == 0)
    {
        // place
        smap->board[winX][winY] = 1;
        return 0;
    }

    // if we can't place
    return 1;
}

// look for winning diagonal placement and place if so
// 2 diagonal: 0,0 and 2,2 - 2,0 and 0,2
int tryDiagonal(struct shmseg *smap, int x1, int y1, int x2, int y2)
{
    // check x1, y1 for X
    if (smap->board[x1][y1] != 1)
    {
        return 1;
    }
    // check x2, y2 for X
    if (smap->board[x2][y2] != 1)
    {
        return 1;
    }

    // the winning place will always be 1,1

    // if winning place is available
    if (smap->board[1][1] == 0)
    {
        // place
        smap->board[1][1] = 1;
        return 0;
    }

    // if we can't place
    return 1;
}

// **end helper functions**

// first play - a random corner
// second play - the opposite corner (if available)
// third play - try the center if O hasn't placed there
// returns 0 if move successfully made
int p1Move(struct shmseg *smap)
{
    // Stores a randomly generated value
    int r;

    // first move: choose a random corner to place X
    // corners: 0,0  0,2  2,0  2,2
    if (smap->counter == 0)
    {
        // Seed time
        time_t t;
        srand((unsigned) time(&t));

        // Generate random number from 1-4
        r = rand() % (3 + 1 - 0) + 1;
        if (r == 1)
        {
            smap->board[0][0] = 1;
            return 0;
        }
        else if (r == 2)
        {
            smap->board[0][2] = 1;
            return 0;
        }
        else if (r == 3)
        {
            smap->board[2][0] = 1;
            return 0;
        }
        else if (r == 4)
        {
            smap->board[2][2] = 1;
            return 0;
        }
        // This should never happen
        else
        {
            printf("ERROR: r not a value between 1-4\n");
            printf("r's value: %d\n", r);
            exit(EXIT_FAILURE);
        }
    }

    // p1's second move will first try to 
    // place at the opposite corner of p1's first move
    if (smap->counter == 1)
    {
        // if X is left top (and right bottom is clear)
        if (smap->board[0][0] == 1 && smap->board[2][2] == 0)
        {
            // place right bottom
            smap->board[2][2] = 1;
            return 0;
        }
        // if X is right top
        else if (smap->board[0][2] == 1 && smap->board[2][0] == 0)
        {
            // place left bottom
            smap->board[2][0] = 1;
            return 0;
        }
        // if X is left bottom
        else if (smap->board[2][0] == 1 && smap->board[0][2] == 0)
        {
            // place right top
            smap->board[0][2] = 1;
            return 0;
        }
        // if X is right bottom
        else if (smap->board[2][2] == 1 && smap->board[0][0] == 0)
        {
            // place left top
            smap->board[0][0] = 1;
            return 0;
        }
        // if we make it here, O is placed in the opposite corner as X
        // so X should just place in a random corner(?)
        // NOTE I haven't seen if this works or not yet
        else
        {
            // try left top
            if (tryPlace(smap,0,0) == 0){return 0;}
            // try right top
            if (tryPlace(smap,0,2) == 0){return 0;}
            // try left bottom
            if (tryPlace(smap,2,0) == 0){return 0;}
            // try right bottom
            if (tryPlace(smap,2,2) == 0){return 0;}
        }
    }

    // third (or more) move:
    // if X is placed in 2 opposite corners, p1 should try to play center
    // for a diagonal win
    if (smap->counter >= 2)
    {
        // if X has left top and bottom right
        if (tryDiagonal(smap,0,0,2,2) == 0){return 0;}
        // if X has bottom left and top right
        if (tryDiagonal(smap,2,0,0,2) == 0){return 0;}

        // if we end up here, x has no diagonal play -or- O has the center
        // check if X can win in a row or column

        // if X has left top and left bottom
        if (tryColumn(smap,0,0,2,0) == 0){return 0;}
        // if X has right top and right bottom
        if (tryColumn(smap,0,2,2,2) == 0){return 0;}
        // if X has left top and right top
        if (tryRow(smap,0,0,0,2) == 0){return 0;}
        // if X has left bottom and right bottom
        if (tryRow(smap,2,0,2,2) == 0){return 0;}

        // if not, take any other available corner

        // try left top
        if (tryPlace(smap,0,0) == 0){return 0;}
        // try right top
        if (tryPlace(smap,0,2) == 0){return 0;}
        // try left bottom
        if (tryPlace(smap,2,0) == 0){return 0;}
        // try right bottom
        if (tryPlace(smap,2,2) == 0){return 0;}
    }
}

// function that checks if there is a row win
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

// function that checks if there is a column win
// return 1 - column win not found
// return 0 - column win found
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

// function that checks if there is a diagonal win
// return 1 - diagonal win not found
// return 0 - diagonal win found
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

    // diagonal win not found
    return 1;
}

// function provided by Mr. Knight in guided exercise 11
// checks if an error occured, if one has prints error message
int checkError(int e, const char *str)
{
    if (e == -1)
    {
        if (errno == EINTR) return e;
        perror(str);
        exit(EXIT_FAILURE);
    }
    return e;
}

// function that is used to change -1 to O, 1 to X, and 0 to a blank space for the board
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

int main(int argc, char* argv[])
{
    // Misc. setup
    int fd;
    int rblock, cblock, dblock;

    // Seed time
    time_t t;
    srand((unsigned) time(&t));

    // Semaphore/shared memory setup

    struct shmseg *smap;

    // semaphore id, shared memory id
    int semid, shmid, val;
    //union semun dummy;

    // semaphore key and shared memory key
    key_t semK, shmK;

    // 1. Attempt to create FIFO called xoSync
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

    // 2. Generate 2 random numbers for creating System V keys
    // Generate 2 values between 10-99
    int num1 = rand() % (99 + 1 - 10) + 10;
    int num2 = rand() % (99 + 1 - 10) + 10;

    // 3. Generate the System V keys with ftok using the random numbers 
    // and the FIFO xoSync. Use the first value generated as the projection value
    // for shared memory and the second value generated 
    // as the projection value for the semaphores.

    shmK = ftok("xoSync",num1);
    if (shmK == -1)
    {
        perror("ftok1");
        exit(EXIT_FAILURE);
    }
    semK = ftok("xoSync",num2);
    if (semK == -1)
    {
        perror("ftok2");
        exit(EXIT_FAILURE);
    }

    // 4. Create the block of shared memory
    checkError(shmid = shmget(shmK, sizeof(struct shmseg), IPC_CREAT | OBJ_PERMS), "shmget");

    // 5. Create a semaphore set with a size of 2
    checkError(semid = semget(semK, 2, IPC_CREAT | OBJ_PERMS), "semget");

    // 6. Initialize the semaphores in the semaphore set.
    // 0 is p1, 1 is p2
    // Set one semaphore to available
    checkError(initSemAvailable(semid,0), "initSemAvailable");
    // Set one semaphore to in use
    checkError(initSemInUse(semid,1), "initSemInUse");

    // 7. Attach the segment of shared memory to process and initialize it
    smap = shmat(shmid, NULL, 0);
    if (smap == (void *) -1)
    {
        checkError(-1, "shmat");
    }

    // 8. Open the FIFO xoSync for write.
    checkError(fd = open("xoSync", O_WRONLY), "open producer for write");

    // 9. Write the random numbers generated in step 2 and used in step 3 to the FIFO.
    checkError(write(fd, &num1, sizeof(num1)), "write 1");
    checkError(write(fd, &num2, sizeof(num2)), "write 2");

    // 10. Close the FIFO
    close(fd);

    // TODO initialize counter to 0 here?
    smap->counter = 0;

    // function to ensure the board is empty (stores 0's everywhere)
    resetBoard(smap);

    // 11. Enter the gameplay loop.
    while (smap->counter != -1)
    {
        // 1. reserve player 1's semaphore
        checkError(reserveSem(semid, 0), "reserveSem");

	// 2. display the state of the game board
	// if statement used to print title for the board that player 2 made a move in
	if(checkBoardFull(smap) == 1)
	  {
	    printf("Player 2 Move\n");
	  }
	printBoard(smap);
	
	// 2 1/2. if the turn counter is -1, exit the loop (player 2 won somehow)
	if (rowWin(smap, -1) == 0 || columnWin(smap, -1) == 0 || diagonalWin(smap, -1) == 0)
	{
	  printf("Player 2 Won!!\n");
	  smap->counter = -1;
	}
	// else statement that executes if player 2 has not won
	else
	  {
	    // 3. make player 1's move
	    p1Move(smap);
	
	    // 4. display the state of the game board.
	    printf("Player 1 Move\n");
	    printBoard(smap);
	  }

        // 5. if player 1 has won, or no more plays exist set the turn counter to -1
        if (rowWin(smap, 1) == 0 || columnWin(smap, 1) == 0 || diagonalWin(smap, 1) == 0)
	    {
	        printf("Player 1 Won!!\n");
	        smap->counter = -1;
	    }
        if(checkBoardFull(smap) == 0)
	    {
	        printf("Tie!!\n");
	        smap->counter = -1;
	    }
       
        // 6. release player 2's semaphore
        checkError(releaseSem(semid, 1), "releaseSem");
    }

    // 12. Open the FIFO xoSync for write
    checkError(fd = open("xoSync", O_WRONLY), "open producer");
    // 13. Close the FIFO
    close(fd);
    // 14. Detach the segment of shared memory
    checkError(shmdt(smap), "shmdt");
    // 15. Delete semaphores and shared memory before exiting
    checkError(semctl(semid,0,IPC_RMID),"semctl");
    checkError(shmctl(shmid,0,IPC_RMID),"shmctl");
    printf("Exiting...\n");
    exit(EXIT_SUCCESS);
}

