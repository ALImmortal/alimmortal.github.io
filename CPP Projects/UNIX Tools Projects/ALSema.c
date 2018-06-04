/*****************************************************************
Name: Anne Liang
UNIX Tools
HW #6, P1
Due: 03/13/18
Description: Write  a  program  that  appends  the  current  
date  and  time  into  a  temporary  file every  hour.  Run  it  
as  a  background  process.  The  output  should  be  in  a 
formatted table. 
*****************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h> /* for malloc() */
#include <unistd.h> /* for fork() */
#include <fcntl.h> /* for open() */
#include <string.h> /* for strlen() */

int main(void)
{
	int i, j;
	int pid;
	int semid; /* semid of semaphore set */
	key_t key=IPC_PRIVATE; /* key to pass to semget() */
	int semflg=IPC_CREAT | 0666;

	/* semflg to pass to semget() */
	int nsems=3;
	/* nsems (# semaphores per set) to pass to semget() */
	int nops; /* number of operations to do */
	struct sembuf *semoparray=(struct sembuf *) malloc(2*sizeof(struct sembuf));
	/* ptr to operations to perform */

	int fd=open("file.txt", O_CREAT|O_RDWR|O_APPEND);
	ssize_t numBytes;

	char **childInput=calloc(3, 20*sizeof(char));
	childInput[0]="Adriana Wise\n";
	childInput[1]="Richard Stevens\n";
	childInput[2]="Evi Nemeth\n";

	char **parentInput=calloc(3, 20*sizeof(char));
	parentInput[0]="Bridget Wise\n";
	parentInput[1]="Rosanna Wise\n";
	parentInput[2]="James Russell Wise\n";

	/* Set up semaphore set */
	fprintf(stderr, "Calling semget() with key=%x, nsems=%d, semflg=%o.\n", key, nsems, semflg);
	if ((semid=semget(key, nsems, semflg))==-1)
	{
		perror("Call to semget() failed.");
		exit(1);
	}
	else
		(void) fprintf(stderr, "Call to semget() succeeded: semid=%d\n", semid);
	int err;
	static union semun arg;
	arg.buf=malloc(sizeof(struct semid_ds));
	arg.val=1;
	err=semctl(semid, 0, SETVAL, arg);
	printf("Semaphore 0 has semval=err=%d.", err);
	printf("Semaphore 0 has semval=arg.val=%d.\n", arg.val);
	err=semctl(semid, 0, GETVAL, NULL);
	printf("Semaphore 0 has semval=err=%d.\n", err);
	printf("Semaphore 0 has semval=arg.val=%d.\n", arg.val);

	if ((pid=fork())<0)
	{
		perror("Call to fork() failed.");
		exit(1);
	}
	if (pid==0) /* child */
	{
		i=0;
		while (i<3)
		/* There is 1 semaphore in the set, to acquire/release 3 times */
		{
			/* Test the semaphore */
			if (err>1)
			{
				/* Set arguments for 1st call to semop() on the
				set */
				/* 1. First argument: semid */
				/* 2. Second argument: semoparray[nops] */
				/* Operation -1: “acquire resource” */
				semoparray[0].sem_num=0;
				/* Semaphore # in set=only use 1 "track": 1
				semaphore in semaphore set */
				semoparray[0].sem_op=-1;
				/* Operation -1="DECREMENT SEMAPHORE:
				resource IN USE" */
				semoparray[0].sem_flg=SEM_UNDO;
				/* Flag="take off semaphore asynchronous" */
				/* 3. Third argument: nops */
				nops=1;
				fprintf(stderr,"\nsemop: CHILD calling semop(%d, &semoparray, %d) with: ", semid, nops);
				/* Recap the call to be made. */
				for (j=0; j<nops; j++)
				{
					fprintf(stderr, "\n\tsemoparray[%d].sem_num=%d, ", j, semoparray[j].sem_num);
					fprintf(stderr, "sem_op=%d, ", semoparray[j].sem_op);
					fprintf(stderr, "sem_flg=%o\n", semoparray[j].sem_flg);
				}
				/* First semop() call to acquire the resource */
				if ((j=semop(semid, semoparray, nops))==-1)
				{
					perror("Call to semop() failed.");
				}
				else
				{
					err=semctl(semid, 0, GETVAL, NULL);
					printf("After DECREMENT CHILD: semval=%d\n", err);
					fprintf(stderr, "\n\nChild process taking control of track: %d/3 times\n", i+1);
					numBytes=write(fd, childInput[i], strlen(childInput[i]));
					sleep(5);
				}

				/* Set arguments for 2nd call to semop() on the
				set */
				/* 1. First argument: semid */
				/* 2. Second argument: semoparray[nops] */
				/* Operation 1: “release the resource” */
				semoparray[0].sem_num=0; /* Semaphore #=0 */
				semoparray[0].sem_op=1;
				/* Operation="release resource" */
				semoparray[0].sem_flg=SEM_UNDO | IPC_NOWAIT;
				/* Flag="take off semaphore, asynchronous" */
				/* 3. Third argument: nops */
				nops=1;
				/* Recap the call to be made. */
				fprintf(stderr,"\nsemop: CHILD calling semop(%d, &semoparray, %d) with: ", semid, nops);

				for (j=0; j<nops; j++)
				{
					fprintf(stderr, "\n\tsemoparray[%d].sem_num=%d, ", j, semoparray[j].sem_num);
					fprintf(stderr, "sem_op=%d, ", semoparray[j].sem_op);
					fprintf(stderr, "sem_flg=%o\n", semoparray[j].sem_flg);
				}

				/* Second semop() call to release the resource */
				if ((j=semop(semid, semoparray, nops))==-1)
				{
					perror("Call to semop() failed.");
				}
				else
				{
				err=semctl(semid, 0, GETVAL, NULL);
					printf("After INCREMENT CHILD: semval=%d\n",err);
				}
			}
			else if (err==0)
			{
				fprintf(stderr, "Waiting for resource to be released by PARENT.\n");
				sleep(5);
			}
			++i;
		} /* end while */
	}
	else /* parent */
	{
		i=0;
		while (i<3)
		{
			/* Test the semaphore */
			if (err>1)
			{
				/* Set arguments for 1st call to semop() on the
				set */
				/* 1. First argument: semid */
				/* 2. Second argument: semoparray[nops] */
				/* Operation -1: “acquire resource” */
				semoparray[0].sem_num=0;
				/* Semaphore # in set=only use 1 "track": 1
				semaphore in semaphore set */
				semoparray[0].sem_op=-1;
				/* Operation -1="DECREMENT SEMAPHORE:
				resource IN USE" */
				semoparray[0].sem_flg=SEM_UNDO;
				/* Flag="take off semaphore asynchronous" */
				/* 3. Third argument: nops */
				nops=1;
				/* Recap the call to be made. */
				fprintf(stderr,"\nsemop: PARENT calling semop(%d, &semoparray, %d) with: ", semid, nops);

				for (j=0; j<nops; j++)
				{
					fprintf(stderr, "\n\tsemoparray[%d].sem_num=%d, ", j, semoparray[j].sem_num);
					fprintf(stderr, "sem_op=%d, ", semoparray[j].sem_op);
					fprintf(stderr, "sem_flg=%o\n", semoparray[j].sem_flg);
				}

				/* First semop() call to acquire the resource */
				if ((j=semop(semid, semoparray, nops))==-1)
				{
					perror("Call to semop() failed.");
				}
				else
				{
					err=semctl(semid, 0, GETVAL, NULL);
					printf("After DECREMENT PARENT: semval=%d\n", err);
					fprintf(stderr, "\n\nParent process taking control of track: %d/3 times\n", i+1);
					numBytes=write(fd, parentInput[i], strlen(parentInput[i]));
					sleep(5);
				}

				/* Set arguments for 2nd call to semop() on the
				set */
				/* 1. First argument: semid */
				/* 2. Second argument: semoparray[nops] */
				/* Operation 1: “release resource” */
				semoparray[0].sem_num=0;
				/* Semaphore #=0 */
				semoparray[0].sem_op=1;
				/* Operation="release resource" */
				semoparray[0].sem_flg=SEM_UNDO | IPC_NOWAIT;
				/* Flag="take off semaphore, asynchronous" */
				/* 3. Third argument: nops */
				nops=1;
				/* Recap the call to be made. */
				fprintf(stderr,"\nsemop: PARENT calling semop(%d, &semoparray, %d) with: ", semid, nops);

				for (j=0; j<nops; j++)
				{
				fprintf(stderr, "\n\tsemoparray[%d].sem_num=%d, ", j, semoparray[j].sem_num);
				fprintf(stderr, "sem_op=%d, ", semoparray[j].sem_op);
				fprintf(stderr, "sem_flg=%o\n", semoparray[j].sem_flg);
				}

				/* Second semop() call to release the resource */
				if ((j=semop(semid, semoparray, nops))==-1)
				{
					perror("Call to semop() failed.");
				}
				else
				{
					err=semctl(semid, 0, GETVAL, NULL);
					printf("After INCREMENT PARENT: semval=%d\n", err);
				}
			}
			else if (err==0)
			{
				fprintf(stderr, "Waiting for resource to be released by CHILD.\n");
				sleep(5);
			}
			++i;
		} /* end while */
	}
	return 0;
}