// Including all the external libraries needed
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <getopt.h>

// Defining a CDT to store values needed
struct fileData {
  pid_t pid;
  int fd;
  char filename[512];
  ino_t inode;
  struct fileData* next;
};

struct fileData *fdAdd(struct fileData *curNode, DIR *curProcess, long pid) {
  // Iteration over the number of files owned by current process
  struct dirent *curFD;
  char *temp;
  while ((curFD = readdir(curProcess)) != NULL) {
    // Check if the current file is the previous or the current working directory file
    if ((strcmp(".", curFD->d_name) == 0) || (strcmp("..", curFD->d_name) == 0)) continue;
    char fdPath[512];
    
    // Getting FD of file
    snprintf(fdPath, sizeof(fdPath), "/proc/%ld/fd/%s", pid, curFD->d_name);
    
    // Getting the filename of file
    struct stat statInfo;
    if (stat(fdPath, &statInfo) == -1) continue;
    ssize_t len = readlink(fdPath, curNode->filename, 255);
    if (len == -1) continue;
    curNode->filename[len] = '\0';

    // Inserting data into the current LL.
    curNode->pid = pid;
    curNode->inode = statInfo.st_ino;
    curNode->fd = (long)strtol(curFD->d_name, &temp, 10);
    
    // Declaring new node for next iter.
    curNode->next = (struct fileData *)malloc(1 * sizeof(struct fileData));
    curNode->next->next = NULL;
    curNode = curNode->next;
  }
  return curNode;
}

int printPerProcess(struct fileData *root, int counter, int positional) {
  printf("          PID     FD\n");
  printf("        ============\n");
  while (root->next != NULL) {
    if ((positional == -1) || (positional == root->pid)) {
      printf("%d       %d    %d\n", counter, root->pid, root->fd);
      counter++;
    }
    root = root->next;
  }
  
  printf("\n");
  return counter;
}

int printSystemWide(struct fileData *root, int counter, int positional) {
  printf("          PID     FD     Filename\n");
  printf("        ===============================\n");
  while (root->next != NULL) {
    if ((positional == -1) || (positional == root->pid)) {
      printf("%d       %d    %d      %s\n", counter, root->pid, root->fd, root->filename);
      counter++;
    }
    root = root->next;
  }
  
  printf("\n");
  return counter;
}

int printVnodes(struct fileData *root, int counter, int positional) {
  printf("            FD      Inode\n");
  printf("        ===============================\n");
  while (root->next != NULL) {
    if ((positional == -1) || (positional == root->pid)) {
      printf("%d        %d     %ld\n", counter, root->pid, root->inode);
      counter++;
    }
    root = root->next;
  }

  printf("\n");
  return counter;
}

int printComposite(struct fileData *root, int counter, int positional) {
  printf("          PID     FD      Filename                Inode\n");
  printf("        ===============================================\n");
  while (root->next != NULL) {
    if ((positional == -1) || (positional == root->pid)) {
      printf("%d       %d   %d   %s   %ld\n", counter, root->pid, root->fd, root->filename, root->inode);
      counter++;
    }
    root = root->next;
  }

  printf("\n");
  return counter;
}

void printThreshold(struct fileData *root, int positional, int threshold) {
  printf("## Offending processes:\n   ");
  int counter = 0;
  int curPID = root->pid;

  while (root->next != NULL) {
    if (root->pid != curPID) {
      if ((counter > threshold) && ((positional == -1) || (positional == curPID))) {
        printf("%d (%d), ", curPID, counter);
      }
      curPID = root->pid;
      counter = 0;
    }
    root = root->next;
    counter++;
  }
  
  if ((counter > threshold)) {
    printf("%d (%d), ", root->pid, counter);
  }
  printf("\n\n");
}

int main(int argc, char *argv[]) {
  // Integers used as booleans
  int opt;
  int perProcess = 0;
  int systemWide = 0;
  int vnodes = 0;
  int composite = 0;
  int threshold = -1;
  int positional = -1;
    
  // Declaring and initializing structs for my long flags
  struct option long_options[] = {
    {"per-process", 0, 0, 'p'},
    {"systemWide", 0, 0, 's'},
    {"Vnodes", 0, 0, 'v'},
    {"composite", 0, 0, 'c'},
    {"threshold=", 1, 0, 't'},
    {0, 0, 0, 0}
  };

  // Getting each command line arguments
  while ((opt = getopt_long(argc, argv, "psvc::", long_options, NULL)) != -1) {
    // For each flags, set its corresponding integer to 1 (true)
    switch(opt) {
      case 'p':
        perProcess = 1;
        break;
      case 's':
        systemWide = 1;
        break;
      case 'v':
        vnodes = 1;
        break;
      case 'c':
        composite = 1;
        break;
      case 't':
        if (optarg) threshold = atoi(optarg);
        break;
    }
  }

  // Considering case for positional arguments particular PID
  if (optind != argc) positional = atoi(argv[optind]);

  // Check if there is no flag (not including positional)
  if ((perProcess + systemWide + vnodes) == 0) composite = 1;

  // Getting the UID
  uid_t uid = getuid();

  // Creating a root node for fileData
  struct fileData *root = (struct fileData *)malloc(1 * sizeof(struct fileData));
  struct fileData *curNode = root;

  // Opening /proc directory
  DIR *procDir = opendir("/proc");
  if (procDir == NULL) {
    perror("Opendir /proc failed.");
    exit(EXIT_FAILURE);
  }
  
  struct dirent *curEntry;
  char *endCheck;
  long pid;

  // Iterating through all files and directories of /proc
  while ((curEntry = readdir(procDir)) != NULL) {
    pid = strtol(curEntry->d_name, &endCheck, 10);
    char procPID[256];
    snprintf(procPID, sizeof(procPID), "/proc/%ld", pid);

    struct stat info;
    uid_t procUID;

    if (stat(procPID, &info) == -1) {
      continue;
    }
    procUID = info.st_uid;

    if (procUID == uid) {
      char fd_path[256];
      snprintf(fd_path, sizeof(fd_path), "/proc/%ld/fd", pid);
      DIR *fd_dir = opendir(fd_path);
      if (fd_dir == NULL) {
        // Skip over current process;
        closedir(fd_dir);
        continue;
      }
      
      curNode = fdAdd(curNode, fd_dir, pid);
      closedir(fd_dir);
    }    
  }
  // Closing directories used and freeing node not used
  free(curNode);
  curNode = NULL;
  closedir(procDir);

  // Setting up counter for number of outputs
  int counter = 0;  

  // Printing results by the flags inputted
  if (positional != -1) printf(">>> Target PID: %d\n", positional);
  printf("\n");
  if (perProcess == 1) counter = printPerProcess(root, counter, positional);
  if (systemWide == 1) counter = printSystemWide(root, counter, positional);
  if (vnodes == 1) counter = printVnodes(root, counter, positional);
  if (composite == 1) counter = printComposite(root, counter, positional);
  if (threshold != -1) printThreshold(root, positional, threshold);

  exit(EXIT_SUCCESS);
}
