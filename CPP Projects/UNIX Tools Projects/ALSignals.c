/**********************************************************************************
Anne Liang
UNIX Tools
HW #:
Due: 05/01/18
Description: Write a simple program showing the system call sigsuspend(). Your 
program should initialize three sets of signals (oldset, newset, waitset) to the 
empty set, using sigemptyset(). It should then add a few
signals to this set, using sigaddset(). It should contain a signal handler, which
you can design to be shared across the few signals you have added to your set. You
should then specify a set of blocked signals, using sigprocmask(). The signals you
want triggered and handled, you should set, and establish handler(s) for, using
either signal() or sigaction(). Then, you want these signals in the blocked
mask to be released after your critical code, using sigsuspend(). In addition, you
can make your critical code deliver a different signal set, defined as a wait set. 
Have each delivered signal leave a “footprint” in a text file. 
***************************************************************************************/
#include <stdio.h> 
#include <signal.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 

void handler(int s); //Making a handler
void PrintSet(sigset_t ss);

int main(void)
{
	//Create a text file for the "footprints"
	int fd=open("footprint.txt", O_CREAT);

	//Create an empty set
	sigset_t *set;
	//Initialize three sets of signals
	sigset_t *oldset=malloc(sizeof(sigset_t));
	sigset_t *newset=malloc(sizeof(sigset_t));
	sigset_t *waiting=malloc(sizeof(sigset_t));

	sigemptyset(oldset);
	sigemptyset(newset);
	sigemptyset(waiting);

	printf ("We have just initialized three sets: oldset, newset, and waitset.\n");
	printf ("\n");

	//Add signals to the set
	sigaddset(&set, SIGINT); //Interrupt from keyboard
	sigaddset(&set, SIGHUP); //Hangup detected
	sigaddset(&set, SIGALRM); //Timer signal from alarm()
	sigaddset(&oldset, SIGQUIT); //Quit from keyboard
	sigaddset(&set, SIGCHLD); //Child stopped or terminated
	sigaddset(&set, SIGTERM); //Termination signal

	sigprocmask(SIG_BLOCK, newset, NULL);
	printf("Printing newset:\n");
	PrintSet(newset);

	//sigaction()
	struct sigaction *act=malloc(sizeof(sigaction));

	//Suspend
	sigsuspend(set);

	
}

void PrintSet(sigset_t *s)
{
	int i;
	i=SIGSTKSZ;

	if (sigismember(set, SIGINT))
		printf("Signal SIGINT added to the set.\n");
	if (sigismember(set, SIGHUP))
		printf("Signal SIGHUP added to the set.\n");
	if (sigismember(set, SIGALRM))
		printf("Signal SIGALRM added to the set.\n");
	if (sigismember(set, SIGQUIT))
		printf("Signal SIGQUIT added to the set.\n");
	if (sigismember(set, SIGCHLD))
		printf("Signal SIGCHLD added to the set.\n");
	if (sigismember(set, SIGTERM))
		printf("Signal SIGTERM added to the set.\n");
			
	for (int i=1; i<32; i++)
	{
		if (sigismember(set, i))
			printf("Signal %d: %s\n", i, strsignal(i));

	}
}

void handler(int signo)
{
	switch (signo)
	{
		case SIGALRM:
			printf("Received SIGALRM.\n");
			break;
		case SIGHUP:
			printf("Received SIGHUP.\n");
			break;
		case SIGINT:
			printf("Received SIGINT.\n");
			break;
		case SIGCHLD:
			printf("Received SIGCHLD.\n");
			break;
		case SIGQUIT:
			printf("Received SIGQUIT.\n");
			break;
		case SIGTERM:
			printf("Received SIGTERM.\n");
			break;
	}
}
