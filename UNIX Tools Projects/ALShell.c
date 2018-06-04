/**********************************************************************
Name: Anne Liang
UNIX Tools
Midterm Project
Description: Create a Shell using given code from 
			 Advanced UNIX Programming, Edition 2
Due: 04/20/18 --> Redo Due: 05/27/18
**********************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>

#define MAXARG 50 /* max args in command */
#define MAXFNAME 500 /* max chars in file name */
#define MAXWORD 500 /* max chars in arg */

#define PERM_FILE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*All the specific TOKENs we need*/
typedef enum {T_WORD, T_BAR, T_AMP, T_SEMI, T_GT, T_GTGT, T_LT, T_NL, T_EOF, T_ERROR} TOKEN;

static bool store_char(char *word, size_t maxword, int c, size_t *np)
{
    errno=E2BIG;
	if(*np < maxword)
        word[(*np)++] = c;
	return true;
    /*This will not be used
	EC_CLEANUP_BGN
		return false;
	EC_CLEANUP_END
    ************************/
}

/*Get TOKEN*/
static TOKEN gettoken(char *word, size_t maxword)
{
	enum {NEUTRAL, GTGT, INQUOTE, INWORD} state = NEUTRAL;
	int c;
	size_t wordn = 0;

	/***************************************************************************
	T_WORD An argument or file name. If quoted, the quotes are removed after the
	token is recognized.
	T_BAR The symbol |.
	T_AMP The symbol &.
	T_SEMI The symbol ;.
	T_GT The symbol >.
	T_GTGT The symbol >>.
	T_LT The symbol <.
	T_NL A newline.
	T_EOF A special token signifying that the end-of-file has been reached. If
	the standard input is a terminal, the user has typed an EOT (Ctrl-d).
	T_ERROR A special token signifying an error.
	****************************************************************************/

	while ((c = getchar()) != EOF) {
		switch (state) {
			case NEUTRAL:
				switch (c) {
					case ';':
						return T_SEMI;
					case '&':
						return T_AMP;
					case '|':
						return T_BAR;
					case '<':
						return T_LT;
					case '\n':
						return T_NL;
					case ' ':
					case '\t':
						continue;
					case '>':
						state = GTGT;
						continue;
					case '"':
						state = INQUOTE;
						continue;
					default:
						state = INWORD;
						store_char(word, maxword, c, &wordn);
						continue;
				}
			case GTGT:
				if (c == '>')
					return T_GTGT;
					ungetc(c, stdin);
					return T_GT;
				case INQUOTE:
					switch (c) {
						case '\\':
							if ((c = getchar()) == EOF)
							c = '\\';
							store_char(word, maxword, c, &wordn);
							continue;
						case '"':
							store_char(word, maxword, '\0', &wordn);
							return T_WORD;
						default:
							store_char(word, maxword, c, &wordn);
							continue;
					}
				case INWORD:
					switch (c) {
						case ';':
						case '&':
						case '|':
						case '<':
						case '>':
						case '\n':
						case ' ':
						case '\t':
							ungetc(c, stdin);
							store_char(word, maxword, '\0', &wordn);
							return T_WORD;
						default:
							store_char(word, maxword, c, &wordn);
							continue;
					}
		}
	}
	if(!ferror(stdin))
		return T_EOF;
	/*This will be ignored
	EC_CLEANUP_BGN
		return T_ERROR;
	EC_CLEANUP_END
	*************************/
}

extern char **environ;
void set(int argc, char *argv[])
{
    int i;
    if (argc != 1)
        printf("Extra args\n");
    else
        for (i = 0; environ[i] != NULL; i++)
            printf("%s\n", environ[i]);
}
void asg(int argc, char *argv[])
{
    char *name, *val;
    if (argc != 1)
        printf("Extra args\n");
    else {
        name = strtok(argv[0], "=");
        val = strtok(NULL, ""); /* get all that's left */
        if (name == NULL || val == NULL)
            printf("Bad command\n");
        else
            setenv(name, val, true);
    }
    return;
    /*This will not be used
    EC_CLEANUP_BGN
    EC_FLUSH("asg")
    EC_CLEANUP_END
    ***********************/
}

static bool builtin(int argc, char *argv[], int srcfd, int dstfd)
{
	char *path;
	if (strchr(argv[0], '=') != NULL)
		asg(argc, argv);
	else if (strcmp(argv[0], "set") == 0)
		set(argc, argv);
	else if (strcmp(argv[0], "ls") == 0)
		set(argc, argv);
	else if (strcmp(argv[0], "cd") == 0) {
		if (argc > 1)
			path = argv[1];
		else if ((path = getenv("HOME")) == NULL)
			path = ".";
		if (chdir(path) == -1)
			fprintf(stderr, "%s: bad directory\n", path);
	else if (strcmp(argv[0], "e") == 0)
		set(argc, argv);
	}
	else
		return false;
	if (srcfd != STDIN_FILENO || dstfd != STDOUT_FILENO)
		fprintf(stderr, "Illegal redirection or pipeline\n");
	return true;
}

static struct sigaction entry_int, entry_quit;

static bool entry_sig(void)
{
    sigaction(SIGINT, &entry_int, NULL);
    sigaction(SIGQUIT, &entry_quit, NULL);
    return true;

    /*This will not be used
    EC_CLEANUP_BGN
        return false;
    EC_CLEANUP_END
    ***********************/
}

static void fd_check(void)
{
	int fd;
	bool ok = true;
	for (fd = 3; fd < 20; fd++)
		if (fcntl(fd, F_GETFL) != -1) {
			ok = false;
			fprintf(stderr, "*** fd %d is open ***\n", fd);
		}
	if (!ok)
		_exit(EXIT_FAILURE);
}

static void redirect(int srcfd, const char *srcfile, int dstfd, const char *dstfile, bool append, bool bckgrnd)
{
	int flags;
	if (srcfd == STDIN_FILENO && bckgrnd) {
		srcfile = "/dev/null";
		srcfd = -1;
	}
	if (srcfile[0] != '\0')
		srcfd = open(srcfile, O_RDONLY, 0);
	dup2(srcfd, STDIN_FILENO);
	if (srcfd != STDIN_FILENO)
		close(srcfd);
	if (dstfile[0] != '\0') {
		flags = O_WRONLY | O_CREAT;
		if (append)
			flags |= O_APPEND;
		else
			flags |= O_TRUNC;
		dstfd = open(dstfile, flags, PERM_FILE);
	}
	dup2(dstfd, STDOUT_FILENO);
	if (dstfd != STDOUT_FILENO)
		close(dstfd);
	fd_check();
	return;


	//EC_CLEANUP_BGN
		//_exit(EXIT_FAILURE); /* we are in child */
	//EC_CLEANUP_END
}

static pid_t invoke(int argc, char *argv[], int srcfd, const char *srcfile, int dstfd, const char *dstfile, bool append, bool bckgrnd, int closefd)
{
	pid_t pid;
	char *cmdname, *cmdpath;
	if (argc == 0 || builtin(argc, argv, srcfd, dstfd))
		return 0;
	switch (pid = fork()) {
		case -1:
			fprintf(stderr, "Can't create new process\n");
			return 0;
		case 0:
			if (closefd != -1)
				close(closefd);
				if (!bckgrnd)
					entry_sig();
				redirect(srcfd, srcfile, dstfd, dstfile, append, bckgrnd);
				cmdname = strchr(argv[0], '/');
				if (cmdname == NULL)
					cmdname = argv[0];
				else
					cmdname++;
				cmdpath = argv[0];
				argv[0] = cmdname;
				execvp(cmdpath, argv);
				fprintf(stderr, "Can't execute %s\n", cmdpath);
				_exit(EXIT_FAILURE);
			}
			/* parent */
			if (srcfd > STDOUT_FILENO)
				close(srcfd);
			if (dstfd > STDOUT_FILENO)
				close(dstfd);
			if (bckgrnd)
				printf("%ld\n", (long)pid);
			return pid;

	/*This will be ignored
	EC_CLEANUP_BGN
	if (pid == 0)
		_exit(EXIT_FAILURE);
		return -1;
	EC_CLEANUP_END
	***********************/
}

/*Token COMMAND*/
static TOKEN command(pid_t *wpid, bool makepipe, int *pipefdp)
{
	TOKEN token, term;
	int argc, srcfd, dstfd, pid, pfd[2] = {-1, -1};
	char *argv[MAXARG], srcfile[MAXFNAME] = "", dstfile[MAXFNAME] = "";
	char word[MAXWORD];
	bool append;
	argc = 0;
	srcfd = STDIN_FILENO;
	dstfd = STDOUT_FILENO;
	while (true) {
		switch (token = gettoken(word, sizeof(word))) {
			case T_WORD:
				if (argc >= MAXARG - 1) {
					fprintf(stderr, "Too many args\n");
					continue;
				}
				if ((argv[argc] = malloc(strlen(word) + 1)) == NULL) {
					fprintf(stderr, "Out of arg memory\n");
					continue;
				}
				strcpy(argv[argc], word);
				argc++;
				continue;
			case T_LT:
				if (makepipe) {
					fprintf(stderr, "Extra <\n");
					break;
				}
				if (gettoken(srcfile, sizeof(srcfile)) != T_WORD) {
					fprintf(stderr, "Illegal <\n");
					break;
				}
				srcfd = -1;
				continue;
			case T_GT:
			case T_GTGT:
				if (dstfd != STDOUT_FILENO) {
					fprintf(stderr, "Extra > or >>\n");
					break;
				}
				if (gettoken(dstfile, sizeof(dstfile)) != T_WORD) {
					fprintf(stderr, "Illegal > or >>\n");
					break;
				}
				dstfd = -1;
				append = token == T_GTGT;
				continue;
			case T_BAR:
			case T_AMP:
			case T_SEMI:
			case T_NL:
				argv[argc] = NULL;
				if (token == T_BAR) {
					if (dstfd != STDOUT_FILENO) {
						fprintf(stderr, "> or >> conflicts with |\n");
						break;
					}
					term = command(wpid, true, &dstfd);
					if (term == T_ERROR)
						return T_ERROR;
				}
				else
					term = token;
					if (makepipe) {
						pipe(pfd);
						*pipefdp = pfd[1];
						srcfd = pfd[0];
					}
					pid = invoke(argc, argv, srcfd, srcfile, dstfd, dstfile, append, term == T_AMP, pfd[1]);
					if (token != T_BAR)
						*wpid = pid;
						if (argc == 0 && (token != T_NL || srcfd > 1))
							fprintf(stderr, "Missing command\n");
						while (--argc >= 0)
							free(argv[argc]);
						return term;
			case T_EOF:
				exit(EXIT_SUCCESS);
			case T_ERROR:
				return T_ERROR;
			}
	}

	/*This will be ignored
	EC_CLEANUP_BGN
		return T_ERROR;
	EC_CLEANUP_END
	**********************/
}

static bool wait_and_display(pid_t pid)
{
	pid_t wpid;
	int status;
	do {
		wpid = waitpid(-1, &status, 0);
	} while (wpid != pid);
	return true;

	/*This will be ignored
	EC_CLEANUP_BGN
		return false;
	EC_CLEANUP_END
	***********************/
}

void printHelp(){
    printf("------------------------------------------------------------------\n");
	printf("Welcome to the Shell!\n");
	printf("In this shell, you will be able to do the following commands:\n");
	printf("pwd: tells you the path of the directory you are in currently\n");
	printf("ls: list the file in the directory\n");
	printf("cd: change the directory\n");
	printf("h: to look at the list of commands if you have forgot or want to\n");
	printf("    look at more commands\n");
	printf("e: Exit the program\n");
	printf("------------------------------------------------------------------\n\n");
}

int main(void)
{
	char word[200];
    struct dirent *de;  
    char *path;
    
    printHelp();
    
	while (1)
        switch (gettoken(word, sizeof(word))) {
            case T_WORD:
                if(word[0]=='l' &&word[1]=='s'){
                	printf("You used the \'ls\' command.\n");
                    DIR *dr = opendir(".");
                    while ((de = readdir(dr)) != NULL)
                        printf("%s\n", de->d_name);
                    closedir(dr); 
		    printf("\n");
                }
                else if(word[0]=='p'&&word[1]=='w'&&word[2]=='d'){
                    printf("You used the \'pwd\' command.\n\n");
                    printf("Path %s\n", getenv("PWD"));
                }
                else if(word[0]=='c'&&word[1]=='d'){
                	printf("You used the \'cd\' command.\n");
                    int s=3; //Starting where the string for the path is
                    int ps=0;
                    while(word[s]!=' '){
                        path[ps]=word[s];
                        ps++;
                        s++;
                    }
                    chdir(path);
                }
                else if(word[0]=='h'){
                	printf("You used the \'h\' command.\n");
                    printHelp();
                }
                else if(word[0]=='e'){
                	printf("You used the \'e\' command.\n");
                    printf("Thank you for using the shell. Now closing...\n");
                }
                break;
            case T_BAR:
                break;
            case T_AMP:
                break;
            case T_SEMI:
                break;
            case T_GT:
                break;
            case T_GTGT:
                break;
            case T_LT:
                break;
            case T_NL:
                break;
            case T_EOF:
                exit(EXIT_SUCCESS);
            case T_ERROR:
                exit(EXIT_SUCCESS);
        }	
    return 0;
}
