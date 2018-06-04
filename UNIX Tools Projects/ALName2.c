/****************************************************************************************
Name : Anne Liang
CSCI 49366: UNIX Tools
Professor: Adriana Wise
HW #: 1
Part: 2
Description: Adapt the ./mycp prgram to copy the newly created file into another file 
without the redirect operators. You need to pass the source and the target files as 
command line arguments.
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <fcntl.h> //For open()

#define BUFFSIZE 4096

int main(int argc, char *argv []){
	//Variables for the program
	char buffer[BUFFSIZE]; //Create buffer for file descriptors
	char *originalFile=argv[1]; //ALname.txt
	char *newFile=argv[2]; //New file: Any name you choose but best to be in ".txt" format
	int n; 
	int fd1=open(originalFile, O_RDWR); //Open original file
	int fd2;
	if((fd2=open(newFile, O_RDWR))<0){
		mode_t mode=  S_IRWXO | S_IRWXU; 
		fd2=creat(newFile, mode); //Create a file if file doesn't exist
	}

	printf("The file descriptor for %s is: %d\n", originalFile, fd1);
	printf("The file descriptor for %s is: %d\n", newFile, fd2);


	while((n=read(fd1, buffer, BUFFSIZE))>0){
		if(write(fd2, buffer, n)!=n)
			printf("write error\n");
	}
	if(n<0)
		printf("read error\n");

	printf("Copying file from %s to %s has been done. \nPlease look at %s for the result.\n", originalFile, newFile, newFile);
	exit(0);
}

