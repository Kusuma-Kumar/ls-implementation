/*
 * myls.c
 */

/* Makefile,parse command line args, open dir , read dir, stat, permissions(i node), close dir */
// figure out which path when absolute path is given, not(relative path)
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdbool.h>

void printFileDetails(const char *path, struct stat fileInfo)
{
    struct passwd *owner;
    struct group *grp;
    extern int errno;
    // File permissions
    printf((S_ISDIR(fileInfo.st_mode)) ? "d" : "-");
    printf((fileInfo.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileInfo.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileInfo.st_mode & S_IXUSR) ? "x" : "-");
    printf((fileInfo.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileInfo.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileInfo.st_mode & S_IXGRP) ? "x" : "-");
    printf((fileInfo.st_mode & S_IROTH) ? "r" : "-");
    printf((fileInfo.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileInfo.st_mode & S_IXOTH) ? "x" : "-");
    // links
    printf(" %hu", fileInfo.st_nlink);

    // owner
    errno = 0;
    if ((owner = getpwuid(fileInfo.st_uid)) != NULL)
    {
        printf(" %s", owner->pw_name);
    }
    else
    {
        perror("getpwuid");
        exit(5);
    }
    // group
    errno = 0;
    if ((grp = getgrgid(fileInfo.st_gid)) != NULL)
    {
        printf(" %s", grp->gr_name);
    }
    else
    {
        perror("getgrgid");
        exit(6);
    }
    // size
    printf(" %lld", fileInfo.st_size);
    // modification time
    time_t modTime = fileInfo.st_mtime;
    char formattedTime[PATH_MAX];
    // "%b %d %R" fromat for month in short, date and 24 hour time
    if (strftime(formattedTime, sizeof(formattedTime), "%b %d %R", localtime(&modTime)))
    {
        printf(" %s", formattedTime);
    }
    else
    {
        // strftime may return value 0 but does not necessarily indicate an error.
        fprintf(stderr, "strftime failed\n");
    }
    // Print file name
    printf(" %s\n", path);
}

int main(int argc, char *argv[])
{
    int opt;
    bool flagA = false;
    bool flagL = false;
    extern int optind, errno;
    // optind=0;
    //printf("optind: %d\n", optind);
    while ((opt = getopt(argc, argv, "la")) != -1)
    {
        switch (opt)
        {
        case 'a':
            /* List all files, including hidden */
            flagA = true;
            break;
        case 'l':
            /* List all files non hidden files with details*/
            flagL = true;
            break;
        case '?':
            break;
        default:
            // list all non hidden file-names
            break;
        }
    }
    //printf("optind: %d\n", optind);
    //check if a path is given as an argument, getopt will change value of optind to store the position of first "non-optional" argument
    char *path = (optind < argc) ? argv[optind] : ".";
    struct stat fileInfo;
    if (stat(path, &fileInfo) == -1)
    {
        perror("stat");
        exit(1);
    }
    // check if path given is a file, S_ISREG returns 0 is it is not a regular file
    if (S_ISREG(fileInfo.st_mode)!=0)
    {
        // If it's a regular file print its details
        printFileDetails(path, fileInfo);
        return 0;
    }
    DIR *dirp = NULL;
    if ((dirp = opendir(path)) == NULL)
    {
        perror("opendir");
        exit(2);
    }
    errno = 0;
    struct dirent *dir;
    // Change current working directory to path to get the full filename to avoid stat: No such file or directory error. 
    // dirp->d_name is the name of the file in the directory:
    // Without changing the current working directory stat() is trying to access a file in a folder("./demoFile.js") instead of ("./demo/demoFile.js")
    chdir(path);
    while ((dir = readdir(dirp)) != NULL)
    {
        if (!flagA && dir->d_name[0] == '.')
        {
            // Ignore hidden files when not using -a
            continue;
        }
        if (stat(dir->d_name, &fileInfo) == -1)
        {
            perror("stat");
            exit(3);
        }
        if (flagL)
        {
            // Print file details
            printFileDetails(dir->d_name, fileInfo);
        }
    }
    // close dir
    if (closedir(dirp) != 0)
    {
        perror("closedir");
        exit(4);
    }
    return 0;
}
