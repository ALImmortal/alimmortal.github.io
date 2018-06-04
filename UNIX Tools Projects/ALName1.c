/****************************************************************************************
Name : Anne Liang
CSCI 49366: UNIX Tools
Professor: Adriana Wise
HW #: 1
Part: 1
Description: Write a small program that creates a file and writes your name in it. Use 
		system functions creat() and write().
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> //For mode

#define BUFFSIZE 4096

int main(int argc, char *argv []){
	//Variables for the program
	char buffer[BUFFSIZE]; //Create buffer for file descriptors
	char *fileN=argv[1];
	char *name=argv[2];
	
	//Create file
	/************************************************************************
	int creat(const char *path, mode_t mode);
	Modes: S_IRWXO -> Read, Write and Search for users other than file owner
	       S_IRWXU -> Read, Write and Search for file owner
	************************************************************************/
	mode_t mode=  S_IRWXO | S_IRWXU; 
	int fd;
	fd=creat(fileN, mode); //Create a file
	printf("The file descriptor for this file is: %d\n", fd);
	
	//Write into file
	/************************************************************************
	ssize_t write(int fd, const void *buf, size_t nbytes);
	nbytes: can get the value by getting length of string
	************************************************************************/
	if(fd<0){printf("Cannot write into file");}
	else{
		//Write into file
		int content=write(fd, name, strlen(name));
		
		printf("Writing has been completed in %s.\n", fileN);
		printf("Please check the text file for the result.\n");
		printf("The text written into file descriptor %d is %d bytes.\n", fd, content);
	}
	
	close(fd);
	exit(0);

}
