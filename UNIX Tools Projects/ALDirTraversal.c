/************************************************************
Name: Anne Liang
UNIX Tools
HW#4
Due: 03/06/18
Description: Copy, adapt, and compile the program that 
traverses a file hierarchy from Chapter 4, Files and 
Directories. Taking as input a starting pathname, the program 
descends the file hierarchy from that point, and returns how 
many files of each of the seven types there are, and what 
percentage of the total that represents. (You will need to 
(re)visit Chapter 2, UNIX Standardizations and Implementations, 
and create a *.c and a *.h file for path_alloc().)
************************************************************/
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef PATH_MAX
static long pathmax = PATH_MAX;
#else
static long pathmax = 0;
#endif

#define FTW_F 1 /* file other than directory */
#define FTW_D 2 /* directory */
#define FTW_DNR 3 /* directory that can’t be read */
#define FTW_NS 4 /* file that we can’t stat */

/* function type that is called for each filename */
typedef int Myfunc(const char *, const struct stat *, int);

static Myfunc myfunc;
static int myftw(char *, Myfunc *);
static int dopath(Myfunc *);

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

/* If PATH_MAX is indeterminate, no guarantee this is adequate */
#define PATH_MAX_GUESS 1024
char *path_alloc(size_t *sizep) /* also return allocated size, if nonnull */
{
    char *ptr;
    size_t size;
 
    if (pathmax == 0) { /* first time through */
    errno = 0;
    if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
    if (errno == 0)
    pathmax = PATH_MAX_GUESS; /* it’s indeterminate */
    } else {
    pathmax++; /* add one since it’s relative to root */
    }
}
    
static char *fullpath; /* contains full pathname for every file */
static size_t pathlen;
    
static int myftw(char *pathname, Myfunc *func)
{
    fullpath = path_alloc(&pathlen); /* malloc PATH_MAX+1 bytes */
    /* (Figure 2.16) */
    if (pathlen <= strlen(pathname)) {
        pathlen = strlen(pathname) * 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            printf("realloc failed");
    }
    strcpy(fullpath, pathname);
    return(dopath(func));
}

static int dopath(Myfunc* func)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;
    if (lstat(fullpath, &statbuf) < 0) /* stat error */
        return(func(fullpath, &statbuf, FTW_NS));
    if (S_ISDIR(statbuf.st_mode) == 0) /* not a directory */
        return(func(fullpath, &statbuf, FTW_F));
    /*
    * It’s a directory. First call func() for the directory,
    * then process each filename in the directory.
    */
    if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
        return(ret);
    n = strlen(fullpath);
    if (n + NAME_MAX + 2 > pathlen) { /* expand path buffer */
        pathlen *= 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            printf("realloc failed");
    }
    fullpath[n++] = '/';
    fullpath[n] = 0;
    if ((dp = opendir(fullpath)) == NULL) /* can’t read directory */
        return(func(fullpath, &statbuf, FTW_DNR));
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            continue; /* ignore dot and dot-dot */
        strcpy(&fullpath[n], dirp->d_name); /* append name after "/" */
        if ((ret = dopath(func)) != 0) /* recursive */
            break; /* time to leave */
    }
    fullpath[n-1] = 0; /* erase everything from slash onward */
    if (closedir(dp) < 0)
        printf("can’t close directory %s", fullpath);
    return(ret);
}
static int myfunc(const char *pathname, const struct stat *statptr, int type)
{
    switch (type) {
        case FTW_F:
        switch (statptr->st_mode & S_IFMT) {
            case S_IFREG: nreg++; break;
            case S_IFBLK: nblk++; break;
            case S_IFCHR: nchr++; break;
            case S_IFIFO: nfifo++; break;
            case S_IFLNK: nslink++; break;
            case S_IFSOCK: nsock++; break;
            case S_IFDIR: /* directories should have type = FTW_D */
            printf("for S_IFDIR for %s", pathname);
        }
        break;
        case FTW_D:
            ndir++;
        break;
        case FTW_DNR:
            printf("can’t read directory %s", pathname);
        break;
        case FTW_NS:
            printf("stat error for %s", pathname);
        break;
        default:
            printf("unknown type %d for pathname %s", type, pathname);
        }
    return(0);
}

int main(int argc, char *argv[])
{
    int ret;
    if (argc != 2)
        printf("usage: ftw <starting-pathname>");
    ret = myftw(argv[1], myfunc); /* does it all */
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (ntot == 0)
        ntot = 1; /* avoid divide by 0; print 0 for all counts */
    printf("regular files = %7ld, %5.2f %%\n", nreg, nreg*100.0/ntot);
    printf("directories = %7ld, %5.2f %%\n", ndir, ndir*100.0/ntot);
    printf("block special = %7ld, %5.2f %%\n", nblk, nblk*100.0/ntot);
    printf("char special = %7ld, %5.2f %%\n", nchr, nchr*100.0/ntot);
    printf("FIFOs = %7ld, %5.2f %%\n", nfifo, nfifo*100.0/ntot);
    printf("symbolic links = %7ld, %5.2f %%\n", nslink, nslink*100.0/ntot);
    printf("sockets = %7ld, %5.2f %%\n", nsock, nsock*100.0/ntot);
    exit(ret);
}