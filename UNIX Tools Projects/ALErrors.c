/*************************************************************************************************
UNIX Tools
HW #9
Due: 03/16/18
Description: Add functionality to this program to print the
error messages associated with the numeric termination status values from
<signal.h>. Also add functionality to print the values of the members of struct
rusage, which will get filled in by calling wait3(). Check these entries with man
2.
*************************************************************************************************/
#include <unistd.h> /* for read(), write() */
#include <fcntl.h> /* for open() */
#include <string.h> /* for strlen() */
#include <stdio.h> /* for printf() */
#include <stdlib.h> /* for calloc() */
#include <stdbool.h> /* for true */
#include <sys/wait.h> /* for the macros */
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

extern int errno;

void Input(int);
void PrintStatus(int status);

int main(int argc, char *argv[])
{
	int errnum;
	int *status_child=(int *)malloc(sizeof(int));
	int fd=open("file4.txt", O_CREAT|O_RDWR|O_APPEND, S_IRUSR| S_IWUSR);
	int ret=fork();

	//Rusage
	int w = RUSAGE_SELF;
	struct rusage u;

	ret = getrusage(w, &u);

	if (ret==0) /* child process */
	{
	Input(fd);
		char *signature=(char *)malloc(20*sizeof(char));
		signature="--------------END child process---------------\n";
		ssize_t num_bytes=write(fd, signature, strlen(signature));
		ret = getrusage(w, &u);

		sleep(2);
	}
	else /* parent process */
	{
		int pid_child=wait(status_child);
		printf("pid_child=%d\n", pid_child);
		printf("status_child=%d\n", *status_child);
		PrintStatus(*status_child);
		free(status_child);
		Input(fd);
		char *signature=(char *)malloc(20*sizeof(char));
		signature="--------------END parent process---------------\n";
		ssize_t num_bytes=write(fd, signature, strlen(signature));
	}
	return 0;
}

void Input(int filedes)
{
	char **name=(char **)calloc(3, 20*sizeof(char *));
	char **it;
	name[0]="Adriana Wise\n";
	name[1]="Richard Stevens\n";
	name[2]="Evi Nemeth\n";
	for (it=&name[0]; *it!=NULL; it++)
	{
		ssize_t num_bytes=write(filedes, *it, strlen(*it));
		sleep(2);
	}
}

void PrintStatus(int status)
{
	int lower_8_bits;
	if ((lower_8_bits=WIFEXITED(status))==true)
	{
		printf("Exit status for child=%d\n", status);
		printf("lower_8_bits=%x\n", status);
		printf("WIFEXITED(status)=%d\n", WIFEXITED(status));
		printf("Error Message: %s\n", strsignal(status));
	}
	else if (WIFSIGNALED(status)==true){
		printf("Exit status for child=%d\n", status);
		printf("WIFSIGNALED(status)=%d\n", WIFSIGNALED(status));
		printf("Error Message: %s\n", strsignal(status));
	}
	else if (WIFSTOPPED(status)==true){
		printf("Exit status for child=%d\n", WSTOPSIG(status));
		printf("WIFSTOPPED(status)=%d\n", WIFSTOPPED(status));
		printf("Error Message: %s\n", strsignal(status));
	}
}

void PrintRusage(struct rusage r){
	printf("-------------------rusage--------------\n");
	printf("ru_nsignals: %ld\n", r->ru_nsignals);
	printf("ru_nvcsw: %ld\n", ru_nvcsw);
	printf("ru_nivcsw: %ld\n", ru_nivcsw);
	printf("ru_msgsnd: %ld\n", ru_msgsnd);
	printf("ru_msgrcv: %ld\n", ru_msgrcv);
	printf("---------------------------------------\n");
}
