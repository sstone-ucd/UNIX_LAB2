#include "apue.h"
#include <dirent.h>
#include <limits.h>
/* function type that is called for each filename */
typedef int Myfunc(const char *, const char *, const struct stat *, int);
static Myfunc myfunc;
static int myftw(char *, char *, Myfunc *);
static int dopath(Myfunc *, char *);
static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;
int
main(int argc, char *argv[])
{
    int ret;
    if (argc != 3)
        err_quit("usage: ftw <starting-pathname> <pattern>");
    char *pattern; 
    pattern = malloc(strlen(argv[2])); 
    memcpy(pattern, argv[2], strlen(argv[2])); 
    
    ret = myftw(argv[1], pattern, myfunc); /* does it all */
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (ntot == 0)
        ntot = 1; /* avoid divide by 0; print 0 for all counts */
    printf("Files with \"%s\" = %7ld, %5.2f %%\n",pattern, nreg, nreg*100.0/ntot);

    exit(ret);
}
/*
* Descend through the hierarchy, starting at "pathname".
* The callers func() is called for every file.
*/
#define FTW_F 1 /* file other than directory */
#define FTW_D 2 /* directory */
#define FTW_DNR 3 /* directory that cant be read */
#define FTW_NS 4 /* file that we cant stat */
static char *fullpath; /* contains full pathname for every file */
static size_t pathlen;
static int /* we return whatever func() returns */
myftw(char *pathname, char *pattern, Myfunc *func)
{
    fullpath = path_alloc(&pathlen); /* malloc PATH_MAX+1 bytes */
    /* (Figure 2.16) */
    if (pathlen <= strlen(pathname)) {
        pathlen = strlen(pathname) * 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            err_sys("realloc failed");
    }
    strcpy(fullpath, pathname);
    return(dopath(func, pattern));
}
/*
* Descend through the hierarchy, starting at "fullpath".
* If "fullpath" is anything other than a directory, we lstat() it,
* call func(), and return. For a directory, we call ourself
* recursively for each name in the directory.
*/
static int /* we return whatever func() returns */
dopath(Myfunc* func, char *pattern)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;
    if (lstat(fullpath, &statbuf) < 0)
        return (func(fullpath, pattern,&statbuf, FTW_NS));  
    if (S_ISDIR(statbuf.st_mode) == 0) /* not a directory */
        return(func(fullpath, pattern, &statbuf, FTW_F));
    /*
    * Its a directory. First call func() for the directory,
    * then process each filename in the directory.
    */
    if ((ret = func(fullpath,pattern, &statbuf, FTW_D)) != 0)
        return(ret);
    n = strlen(fullpath);
    if (n + NAME_MAX + 2 > pathlen) { /* expand path buffer */
        pathlen *= 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            err_sys("realloc failed");
    }
    fullpath[n++] = '/';
    fullpath[n] = 0;
    if ((dp = opendir(fullpath)) == NULL) /* cant read directory */
        return(func(fullpath, pattern, &statbuf, FTW_DNR));
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            continue; /* ignore dot and dot-dot */
        strcpy(&fullpath[n], dirp->d_name); /* append name after "/" */
        if ((ret = dopath(func, pattern)) != 0) /* recursive */
            break; /* time to leave */
    }
    fullpath[n-1] = 0; /* erase everything from slash onward */
    if (closedir(dp) < 0)
        err_ret("cant close directory %s", fullpath);
    return(ret);
}
static int
myfunc(const char *pathname, const char *pattern, const struct stat *statptr, int type)
{
    switch (type) {
            case FTW_F:
                switch (statptr->st_mode & S_IFMT) {
                    case S_IFREG: if (strstr(pathname, pattern)!=NULL){printf("%s\n",pathname); nreg++;} break;
                    case S_IFBLK: nblk++; break;
                    case S_IFCHR: nchr++; break;
                    case S_IFIFO: nfifo++; break;
                    case S_IFLNK: nslink++; break;
                    case S_IFSOCK: nsock++; break;
                    case S_IFDIR: /* directories should have type = FTW_D */
                    err_dump("for S_IFDIR for %s", pathname);
                } 
                break;
            case FTW_D: 
                ndir++; 
                break;
            case FTW_DNR:
                err_ret("cant read directory %s", pathname); 
                break;
            case FTW_NS:
                err_ret("stat error for %s", pathname); 
                break;
            default:
                err_dump("unknown type %d for pathname %s", type, pathname);
        }
        return(0);
}
