/**************************************************************
* Class:  CSC-415-0# Spring 2023
* Name: Gwangwoo Lee
* Student ID: 922784535
* GitHub UserID: pangwoo
* Project: Assignment 5 – Buffered I/O
*
* File: b_io.c
*
* Description: This project implements a custom buffered I/O layer in C, similar in concept
to the standard `read()` system call, but built **entirely on top of low-level
block I/O APIs**.
*
**************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "b_io.h"
#include "fsLowSmall.h"

#define MAXFCBS 20	//The maximum number of files open at one time

// This structure is all the information needed to maintain an open file
// It contains a pointer to a fileInfo strucutre and any other information
// that you need to maintain your open file.
typedef struct b_fcb
	{
	fileInfo * fi;	//holds the low level systems file info
	
	// Add any other needed variables here to track the individual open file
	char* buf;
	// buffer location
	int bIdx;
	// number of block read
	int rb;
	int nb;
	// buffer size
	int bSize;

	} b_fcb;
	
//static array of file control blocks
b_fcb fcbArray[MAXFCBS];

// Indicates that the file control block array has not been initialized
int startup = 0;	

// Method to initialize our file system / file control blocks
// Anything else that needs one time initialization can go in this routine
void b_init ()
	{
	if (startup)
		return;			//already initialized

	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].fi = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free File Control Block FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].fi == NULL)
			{
			fcbArray[i].fi = (fileInfo *)-2; // used but not assigned
			return i;		//Not thread safe but okay for this project
			}
		}

	return (-1);  //all in use
	}

// b_open is called by the "user application" to open a file.  This routine is 
// similar to the Linux open function.  	
// You will create your own file descriptor which is just an integer index into an
// array of file control blocks (fcbArray) that you maintain for each open file.  
// For this assignment the flags will be read only and can be ignored.

b_io_fd b_open (char * filename, int flags)
	{
	if (startup == 0) b_init();  //Initialize our system

	//*** TODO ***//  Write open function to return your file descriptor
	//				  You may want to allocate the buffer here as well
	//				  But make sure every file has its own buffer
	// This is where you are going to want to call GetFileInfo and b_getFCB

	// find file info by file name
	fileInfo * file = GetFileInfo(filename);

	// check that file exist
	if(file == NULL) return -1;
	
	printf("fileName: %s\nfileSize: %d\nfileLocation: %d\n", file->fileName, file->fileSize, file->location);

	b_io_fd fd = b_getFCB();
	
	if(fd == -1){
		return -1;
	}

	fcbArray[fd].fi = file;
	fcbArray[fd].buf = malloc(sizeof(char) * B_CHUNK_SIZE);
	fcbArray[fd].bIdx = 0;
	fcbArray[fd].rb = 0;
	fcbArray[fd].bSize = file->fileSize%B_CHUNK_SIZE;
	fcbArray[fd].nb = file->fileSize/B_CHUNK_SIZE;
	if(fcbArray[fd].bSize) {
		fcbArray[fd].nb++;
	}else{
		fcbArray[fd].bSize = B_CHUNK_SIZE;
	}
	printf("nb: %d\n", fcbArray[fd].nb);
	return fd;
	}



// b_read functions just like its Linux counterpart read.  The user passes in
// the file descriptor (index into fcbArray), a buffer where thay want you to 
// place the data, and a count of how many bytes they want from the file.
// The return value is the number of bytes you have copied into their buffer.
// The return value can never be greater then the requested count, but it can
// be less only when you have run out of bytes to read.  i.e. End of File	
int b_read (b_io_fd fd, char * buffer, int count)
{
	//*** TODO ***//  
	// Write buffered read function to return the data and # bytes read
	// You must use LBAread and you must buffer the data in B_CHUNK_SIZE byte chunks.
	
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	// and check that the specified FCB is actually in use	
	if (fcbArray[fd].fi == NULL)		//File not open for this descriptor
		{
		return -1;
		}
	
	
	// Your Read code here - the only function you call to get data is LBAread.
	// Track which byte in the buffer you are at, and which block in the file	
	int LBAres;	
	if(fcbArray[fd].rb == 0) {
		LBAres = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].fi->location);
		fcbArray[fd].rb++;

		//check it read block
		if(LBAres < 1){
			return -1;
		}
	}
	
	if(fcbArray[fd].rb > fcbArray[fd].nb){
		return -1;
	}

	int remainCount = count;
	while(remainCount > 0){
		int bufSize = B_CHUNK_SIZE;		
		if(fcbArray[fd].rb == fcbArray[fd].nb) bufSize = fcbArray[fd].bSize;

		int copyBytes = minBytes(remainCount, bufSize-fcbArray[fd].bIdx);
		memcpy(buffer + (count - remainCount), fcbArray[fd].buf + fcbArray[fd].bIdx, copyBytes);
		fcbArray[fd].bIdx += copyBytes;
		remainCount -= copyBytes;
		
		if(bufSize == fcbArray[fd].bIdx){
			LBAres = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].fi->location+fcbArray[fd].rb);
			fcbArray[fd].rb++;
			fcbArray[fd].bIdx = 0;
		}

		if(fcbArray[fd].rb > fcbArray[fd].nb || LBAres < 1){
			break;
		}



	}

	return count - remainCount;	
}
	


// b_close frees and allocated memory and places the file control block back 
// into the unused pool of file control blocks.
int b_close (b_io_fd fd)
	{
	//*** TODO ***//  Release any resources
	free(fcbArray[fd].buf);
	fcbArray[fd].fi = NULL;
	return 0;
	}
	


int minBytes(int a, int b){
	if(a < b) return a;
	return b;
}