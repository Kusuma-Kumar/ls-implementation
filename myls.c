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
#include <linux/limits.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
    int opt;
    bool flagA = false;
    bool flagL = false;
    DIR *dirp;
    struct dirent *dir;
    struct stat fileStatus;
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
        default:
            break;
        }
    }
    //check if a path is given as an argument
    if(optind < argc){
        dirp = opendir(argv[optind]);
    }
    else{
        dirp = opendir(".");
    }
    if (dirp == NULL)
    {
        perror("opendir");
        exit(1);
    }
    while ((dir = readdir(dirp)) != NULL)
    {
        if (stat(dir->d_name, &fileStatus) == -1)
        {
            perror("stat");
            exit(1);
        }
        if (!flagA && dir->d_name[0] == '.')
        {
            // ignore hidden files when not using -a or -l
            continue;
        }
        if (flagL)
        {
            // file permissions
            printf((S_ISDIR(fileStatus.st_mode)) ? "d" : "-");
            printf((fileStatus.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStatus.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStatus.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStatus.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStatus.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStatus.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStatus.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStatus.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStatus.st_mode & S_IXOTH) ? "x" : "-");
            printf(" %lu", fileStatus.st_nlink);

            // owner
            // use fileStatus.st_uid from inode 7 to get User name
            struct passwd *owner = getpwuid(fileStatus.st_uid);
            if (owner != NULL)
            {
                printf(" %s", owner->pw_name);
            }
            else
            {
                perror("getpwuid");
            }
            // group
            // use fileStatus.st_gid from inode 7 to get group name
            struct group *grp = getgrgid(fileStatus.st_gid);
            if (grp != NULL)
            {
                printf(" %s", grp->gr_name);
            }
            else
            {
                perror("getgrgid");
            }
            // size
            printf(" %lu", fileStatus.st_size);
            // modification time
            time_t modTime = fileStatus.st_mtime;
            char formattedTime[PATH_MAX];
            // "%b %d %R" fromat fro maonth in short, date and 24 hour time
            if (strftime(formattedTime, sizeof(formattedTime), "%b %d %R", localtime(&modTime)))
            {
                printf(" %s", formattedTime);
            }
            else
            {
                // strftime may return value 0 but does not necessarily indicate an error.
                fprintf(stderr, "strftime failed\n");
            }
        }
        // file name
        printf(" %s\n", dir->d_name);
    }
    // close dir
    closedir(dirp);
    return 0;
}
