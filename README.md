Authors: Kusuma Kumar

Known Bugs:

Resources: https://stackoverflow.com/questions/8268316/how-to-retrieve-the-user-name-from-the-user-id, TA sessions with John , https://stackoverflow.com/questions/5125919/stat-error-no-such-file-or-directory-when-file-name-is-returned-by-readdir, https://www2.hawaii.edu/~walbritt/ics212/examples/linkedlist.c

Project Description:
The myls program is a simplified implementation of the ls command in C. It provides basic functionality for listing files and directories in the current working directory. The program supports various scenarios, such as listing non-hidden files, displaying detailed information with the -l option, and including hidden files with the -a option.

Features
Lists non-hidden files in the current directory by default.
Behaves similarly to ls with options for listing hidden files (-a) and providing detailed information (-l).
Implements basic file and directory listing logic in C.
Utilizes system calls and functions such as opendir(3), readdir(3), closedir(3), getopt(3), stat(2), strftime(3), and getpwnam(3).
