#include "../../apue.h"
#include <dirent.h>
#include <limits.h>

/* function type that is called for each filename */
typedef int Myfunc(const char *, const struct stat *, int); 

static Myfunc Myfunc; 
static int  myftw(char *, Myfunc *);
static int dopath(Myfunc *); 

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot, npattern; 

int main(int argc, char *argv[]){
    int ret; 
    if(argc !=2)
        err_quit("usage: ftw <starting-pathname>"); 
    /*this function does everything*/
    ret = myftw(argv[1], myfunc); 

    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock; 
    if(ntot == 0)
        ntot = 1; /*avoid divide by zero*/
    printf("Matches Pattern = %7ld, %5.2f, %%\n\n", npattern, npattern*100.0/ntot);

    /*printf("regular files  = %7ld, %5.2f, %%\n", nreg, nreg*100.0/ntot); 
    printf("directories    = %7ld, %5.2f, %%\n", ndir, ndir*100.0/ntot); 
    printf("block special  = %7ld, %5.2f, %%\n", nblk, nblk*100.0/ntot); 
    printf("char special   = %7ld, %5.2f, %%\n", nchr, nchr*100.0/ntot); 
    printf("FIFOs          = %7ld, %5.2f, %%\n", nfifo, nfifo*100.0/ntot); 
    printf("symbolic links = %7ld, %5.2f, %%\n", nslink, nslink*100.0/ntot); 
    printf("sockets        = %7ld, %5.2f, %%\n", nsock, nsock*100.0/ntot);*/ 
    
    exit(ret); 
}

/*
    Dexcend through the heirarchy, starting with "pathname". 
    The caller's func() is called for every file. 
*/

#define FTW_F   1 /*file other than directory*/
#define FTW_D   2 /*directory*/
#define FTW_DNR 3 /*directory that cant be read*/
#define FTW_NS  4 /*file we cant stat*/


#define PATTERN 5 /*file matches the pattern*/

static char *fullpath; /*contains full pathname for every file*/
static size_t pathlen; 

/*collect the path name into an appropriately sized variable*/
static int
myftw(char *pathname, Myfunc *func){
    fullpath = path_alloc(&pathlen); /* malloc PATH_MAX+1 bytes*/
    /*if the pathlength is less than the string length*/
    if(pathlen <= strlen(pathname)){
        /*increase the size of of the pathlength field*/
        pathlen = strlen(pathname) * 2; 
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            err_sys("realloc failed"); 
    }
    strcpy(fullpath, pathname); 
    return(dopath(func)); 
}

/*
    Descend through the heirarchy, starting at "fullpath". 
    If "fullpath" is anything other than a directory, we lstat() it, 
    call func(), and return. For a directory, we call ourself 
    recursively for each name in the directory
*/
static int 
dopath(Myfunc* func){
    struct stat statbuf; 
    struct dirent *dirp; 
    DIR *dp; 
    int ret, n; 
    /*if getting information about the symbolic link fails, return stat error*/
    if(lstat(fullpath, &statbuf) <0)
        return(func(fullpath, &statbuf, FTW_NS));
    /*check if its a directory, if not, return as such*/     
    if(S_ISDIR(statbuf.st_mode) == 0)
        return(func(fullpath, &statbuf, FTW_F)); 

    /*
        If its passed the two tests above, the file path is a directory
        then process each filename in the directory
    */   
    if ((ret = func(fullpath, &statbuf, FTW_D)) !=0)
        return(ret); 
    /*extend the pathlength*/
    n = strlen(fullpath); 
    if (n + NAME_MAX + 2 > pathlen){
        pathlen *= 2; 
        if((fullpath = realloc(fullpath, pathlen))== NULL)
            err_sys("realloc failed"); 
    }
    /*end the newly allocated path with a / and null terminator*/
    fullpath[n++] = '/';
    fullpath[n] = 0; 

    /*if the directory cant be read return*/
    if((dp = opendir(fullpath)) == NULL)
        return(func(fullpath, &statbuf, FTW_DNR)); 
    /*while the directory has items to be read*/
    while ((dirp = readdir(dp)) != NULL){
        /*skip . and .. directories*/
        if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, ".."))
            continue; 
        /*copy the path name*/
        strcpy(&fullpath[n], dirp->d_name); 
        /*make a recursive call to collect the next segment in the directory path
        until there is nothing left to parse and store*/
        if ((ret = dopath(func)) != 0)
            break;
    }
    /*terminate the path at the / where the path we just copied begins*/
    fullpath[n-1] = 0; 
    /*close the directory*/
    if(closedir(dp) < 0)
        err_ret("cant close directory %s", fullpath); 
    return(ret); 
}

static int
myfunc(const char *pathname, const struct stat *statptr, int type){
    switch (type){
        /*if its a file determine the type of each file in the directory and increment its count*/
        case FTW_F:
            switch (statptr->st_mode & S_IFMT){
                case S_IFREG: if (strstr(pathname, pattern)==0){nreg++;} break; 
                case S_IFBLK: nblk++; break; 
                case S_IFCHR: nchr++; break; 
                case S_IFIFO: nfifo++; break; 
                case S_IFLNK: nslink++; break; 
                case S_IFSOCK: nsock++; break; 
                /*directories have case FTW_D  NOT FTW_F*/
                case S_IFDIR: err_dump("for S_FDIR for %s", pathname);
            }
            break; 
        /*if its a directory, increment the directory count*/
        case FTW_D:
            ndir++; 
            break;
        /*if read failed, indicate as such*/
        case FTW_DNR: 
            err_ret("cant read directory %s, pathname"); 
            break;
        /*if reading info about the file failed, indicate as such*/
        case FTW_NS: 
            err_ret("stat error %s, pathname"); 
            break; 
        /*if none of the cases above, indicate unknown error*/
        default:
            err_dump("unknown type %d for pathname %s", type, pathname); 
    }
    return (0); 
}

static int
myFind(const char *pathname, const char *pattern, const struct stat *statptr, int type){
    switch (type){
        /*if its a file determine the type of each file in the directory and increment its count*/
        case FTW_F:
            switch (statptr->st_mode & S_IFMT){
                case S_IFREG: if (strstr(pathname, pattern)==0){nreg++;} break; 
                case S_IFBLK: nblk++; break; 
                case S_IFCHR: nchr++; break; 
                case S_IFIFO: nfifo++; break; 
                case S_IFLNK: nslink++; break; 
                case S_IFSOCK: nsock++; break; 
                /*directories have case FTW_D  NOT FTW_F*/
                case S_IFDIR: err_dump("for S_FDIR for %s", pathname);
            }
            break; 
        /*if its a directory, increment the directory count*/
        case FTW_D:
            ndir++; 
            break;
        /*if read failed, indicate as such*/
        case FTW_DNR: 
            err_ret("cant read directory %s, pathname"); 
            break;
        /*if reading info about the file failed, indicate as such*/
        case FTW_NS: 
            err_ret("stat error %s, pathname"); 
            break; 
        /*if none of the cases above, indicate unknown error*/
        default:
            err_dump("unknown type %d for pathname %s", type, pathname); 
    }
    return (0); 
}

