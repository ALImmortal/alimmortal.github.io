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
#include <stdlib.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <time.h>
#include <sys/time.h> 
#include <sys/resource.h>
#include <unistd.h>

int main(void)
{
	//Create the temporary file
	//char temp_filename[L_tmpnam];
	char buf[20];
	char *temp_filename;
	char template[]="time.txt";
	temp_filename=mktemp(template);

	printf("temp_filename=%s\n", temp_filename);
	int fd=open(temp_filename, O_RDWR | O_CREAT);
	printf("Using mktemp(): fd=%d\n", fd);
	FILE *file1=tmpfile();

	//Getting the time
	time_t t;
	struct tm* timenow; //In order to get hrs, min,etc

	time (&t);
	timenow = localtime (&t);

    strftime(buf, 20, "%x\t\t\t%H:%M", timenow);
	printf("Current time: %s", buf);

	//Write the table
	fprintf(file1, "------------------------------------------------------------\n");
	fprintf(file1, "|Date\t\t\t|Time\t\t\t|\n");
	fprintf(file1, "|------------------------------------------------------------|\n");

	int count=0;
	while(count<4){
		strftime(buf, 20, "%x\t\t\t%H:%M\n", timenow);
		fprintf(file1, "%s", buf);
		fprintf(file1, "|------------------------------------------------------------|\n");
		count++;
		sleep(3600); //This will not count any time between it
		close(fd);
	}
	fprintf(file1, "------------------------------------------------------------\n");

	return 0;
}

