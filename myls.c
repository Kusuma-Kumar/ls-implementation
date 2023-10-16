/*
 * myls.c
 */

/* Makefile,parse command line args, open dir , read dir, stat, permissions(i node), close dir */
// figure out which path when absolute path is given, not(relative path)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#include <errno.h>

struct Node {
    char *data;
    struct Node *next;
};

void listFiles(const char *path, bool showHiddenFiles, bool showFileInfo);
void printFileDetails(const char *path, struct stat fileInfo);
void insertNode(struct Node **head, char *data);
void freeList(struct Node *head);
void printLinkedList(struct Node *head);


int main(int argc, char *argv[])
{
    int opt;
    bool showHiddenFiles = false;
    bool showFileInfo = false;
    //Using a linked list to store non-option file names
    struct Node *nonOptionalArgs = NULL;

    while ((opt = getopt(argc, argv, "la")) != -1)
    {
        switch (opt)
        {
        case 'a':
            /* List all files, including hidden */
            showHiddenFiles = true;
            break;
        case 'l':
            /* List all files non hidden files with details*/
            showFileInfo = true;
            break;
        case '?':
            printf(" %s\n", "unrecognized option argument");
            exit(7);
        default:
            // list all non hidden file-names
            break;
        }
    }

    while (optind < argc) {
        char *tempPath = argv[optind];
        struct stat fileInfo;
        // check if file/dir exists before adding it to linkedList
        if (stat(tempPath, &fileInfo) == -1)
        {
            perror("stat");
            exit(8);
        }
        insertNode(&nonOptionalArgs, argv[optind]);
        optind++; // Move to the next argument
    }

    if (nonOptionalArgs == NULL ) 
    {
        // If there are no non-option arguments, list the current directory
        listFiles(".", showHiddenFiles, showFileInfo);
        return 0;
    }

    // printLinkedList(nonOptionalArgs);
    struct Node *current = nonOptionalArgs;
    
    while (current != NULL) {
        listFiles(current->data, showHiddenFiles, showFileInfo);
        current = current->next;
    }

    freeList(nonOptionalArgs);

    return 0;
}



void listFiles(const char *path, bool showHiddenFiles, bool showFileInfo) {
    struct stat fileInfo;
    if (stat(path, &fileInfo) == -1)
    {
        perror("stat");
        exit(1);
    }
    // check if path given is a file, S_ISREG returns 0 if it is not a regular file
    if (S_ISREG(fileInfo.st_mode) != 0)
    {
        if(showFileInfo == false){
            printf(" %s\n", path);
            return;
        } else {
            // If it's a regular file print its details
            printFileDetails(path, fileInfo);
            return;
        }
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
        if (!showHiddenFiles && dir->d_name[0] == '.')
        {
            // Ignore hidden files when not using -a
            continue;
        }
        if (stat(dir->d_name, &fileInfo) == -1)
        {
            perror("stat");
            exit(3);
        }
        if (showFileInfo)
        {
            printFileDetails(dir->d_name, fileInfo);
        }else{
            printf(" %s\n", dir->d_name);
        }
    }
    if (closedir(dirp) != 0)
    {
        perror("closedir");
        exit(4);
    }
    // change current path to parent directory because we used chdir 
    chdir("..");
}



void printFileDetails(const char *path, struct stat fileInfo)
{
    struct passwd *owner;
    struct group *grp;
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
    printf(" %lu", fileInfo.st_nlink);
    errno = 0;
    if ((owner = getpwuid(fileInfo.st_uid)) != NULL)
    {
        printf(" %s", owner->pw_name);
    }else{
        // should print numerical version of UID if getpwuid fails
        printf(" %u", fileInfo.st_uid);
    }
    errno = 0;
    if ((grp = getgrgid(fileInfo.st_gid)) != NULL)
    {
        printf(" %s", grp->gr_name);
    }else{
        // should print numerical version of GID if getgrgid fails
        printf(" %u", fileInfo.st_gid);
    }
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
    printf(" %s\n", path);
}




void insertNode(struct Node **head, char *data) {
    struct Node *newNode;
     ;
    if((newNode = (struct Node *)malloc(sizeof(struct Node))) == NULL){
        perror("malloc");
        exit(9);
    }
    newNode->data = strdup(data);  // Make a copy of the data
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
    } else {
        struct Node *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}




void freeList(struct Node *head) {
    struct Node *current = head;
    while (current != NULL) {
        struct Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}




void printLinkedList(struct Node *head) {
    struct Node *current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }
}


