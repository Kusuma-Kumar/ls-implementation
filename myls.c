/*
 * myls.c
 *
 * Makefile,parse command line args, open dir , read dir, stat, permissions(i node), close dir 
 * for all perrors related to specific files i have return; as i want leave exit my current function to mock what the real ls does
 * the real ls just print the error and continues on the the next file/dir listed in filesAndDirectories
 */

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

void listFiles(const char *path, bool showHiddenFiles, bool showFileInfo, int sizeOfFilesAndDirectories);
void printFileDetails(const char *path, struct stat fileInfo);
void insertNode(struct Node **head, char *data);
void freeList(struct Node *head);

int main(int argc, char *argv[]) {
    int opt;
    bool showHiddenFiles = false;
    bool showFileInfo = false;
    // Using a linked list to store file/ directory names user has entered
    struct Node *filesAndDirectories = NULL;
    int sizeOfFilesAndDirectories = 0;

    // loop though all option arguments provided
    while ((opt = getopt(argc, argv, "la")) != -1) {
        switch (opt) {
            case 'a':
                // List all files, including hidden 
                showHiddenFiles = true;
                break;
            case 'l':
                // List all files non hidden files with details
                showFileInfo = true;
                break;
            case '?':
                // I want to exit entirely based on what the real ls does. ex:ls -j .git
                // ls: invalid option -- 'j'
                // since i am exiting the entire program this case does not require a break;
                exit(1);
            default:
                // list all non hidden file-names
                break;
        }
    }

    // save all the files and/or directories in a linkedList
    while (optind < argc) {
        char *tempPath = argv[optind];
        struct stat fileInfo;
        
        // check if file/dir exists before adding it to linkedList
        if (stat(tempPath, &fileInfo) == -1) {
            perror("stat");
        } else {
            insertNode(&filesAndDirectories, argv[optind]);
            sizeOfFilesAndDirectories += 1;
        }
        
        optind++; // Move to the next argument
    }

    if (filesAndDirectories == NULL) {
        // If there are no non-option arguments, list the current directory
        listFiles(".", showHiddenFiles, showFileInfo, sizeOfFilesAndDirectories);
        return 0;
    }

    struct Node *current = filesAndDirectories;
    
    // loop through all requested files and directories and print user request based on flags
    while (current != NULL) {
        listFiles(current->data, showHiddenFiles, showFileInfo, sizeOfFilesAndDirectories);
        current = current->next;
    }

    freeList(filesAndDirectories);

    return 0;
}

void listFiles(const char *path, bool showHiddenFiles, bool showFileInfo, int sizeOfFilesAndDirectories) {
    struct stat fileInfo;

    if (stat(path, &fileInfo) == -1) {
        perror("stat");
        return;
    }

    // check if path given is a file, S_ISREG returns 0 if it is not a regular file
    if (S_ISREG(fileInfo.st_mode) != 0) {
        if(showFileInfo == false) {
            printf(" %s\n", path);
            return;
        } else {
            // If it's a regular file print its details
            printFileDetails(path, fileInfo);
            return;
        }
    }

    // print the name of directory before entering to list all its files only if more than one directory/files are requested
    if(sizeOfFilesAndDirectories > 1) {
        printf("\n%s:\n", path);
    }
    
    //saving the current working directory so before i go into my directories so i can always come back to initial directory
    char workingDir[PATH_MAX];
    if (getcwd(workingDir, sizeof(workingDir)) == NULL) {
        perror("getcwd");
        return;
    }
    
    DIR *dirp = NULL;
    if ((dirp = opendir(path)) == NULL) {
        perror("opendir");
        return;
    }
    errno = 0;
    struct dirent *dir;
    // Change current working directory to path to get the full filename to avoid stat: No such file or directory error. 
    // dirp->d_name is the name of the file in the directory:
    // Without changing the current working directory stat() is trying to access a file in a folder("./demoFile.js") instead of ("./demo/demoFile.js")
    if(chdir(path) == -1) {
        perror("chdir");
        return;
    }
    
    while ((dir = readdir(dirp)) != NULL) {
        if (!showHiddenFiles && dir->d_name[0] == '.') {
            // Ignore hidden files when not using -a
            continue;
        }
        if (stat(dir->d_name, &fileInfo) == -1) {
            perror("stat");
            return;
        }
        if (showFileInfo) {
            printFileDetails(dir->d_name, fileInfo);
        } else {
            // have to space after the name and no-newline characters to mock the real ls formatting
            printf("%s  ", dir->d_name);
        }
    }

    if (closedir(dirp) != 0) {
        perror("closedir");
    }

    // change current path to initial working directory since we used chdir to move into other directories
    if(chdir(workingDir) == -1) {
        perror("chdir");
        return;
    }
    
    printf("\n");
}

void printFileDetails(const char *path, struct stat fileInfo) {
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
    if ((owner = getpwuid(fileInfo.st_uid)) != NULL) {
        printf(" %s", owner->pw_name);
    } else {
        // should print numerical version of UID if getpwuid fails
        printf(" %u", fileInfo.st_uid);
    }
    
    errno = 0;
    if ((grp = getgrgid(fileInfo.st_gid)) != NULL) {
        printf(" %s", grp->gr_name);
    } else {
        // should print numerical version of GID if getgrgid fails
        printf(" %u", fileInfo.st_gid);
    }
    
    time_t modTime = fileInfo.st_mtime;
    char formattedTime[PATH_MAX];
    // "%b %d %R" fromat for month in short, date and 24 hour time
    if (strftime(formattedTime, sizeof(formattedTime), "%b %d %R", localtime(&modTime))) {
        printf(" %s", formattedTime);
    } else {
        // strftime may return value 0 but does not necessarily indicate an error.
        fprintf(stderr, "strftime failed\n");
    }
    
    printf(" %s\n", path);
}

void insertNode(struct Node **head, char *data) {
    struct Node *newNode;
    if((newNode = (struct Node *)malloc(sizeof(struct Node))) == NULL) {
        perror("malloc");
        return;
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

