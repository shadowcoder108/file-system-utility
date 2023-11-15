#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

// Function to create a regular file with specified permissions
void createFile(char *filename, mode_t mode) {
    int result = mknod(filename, S_IFREG | mode, 0);
    if (result == -1) {
        perror("Error creating file");
        exit(1);
    }
    printf("File '%s' created successfully.\n", filename);
}

// Function to read data from a file at a specified offset
void readFile(char *filename, off_t offset, size_t size) {
    int file = open(filename, O_RDONLY);
    if (file == -1) {
        perror("Error opening file");
        exit(1);
    }
    char *buffer = malloc(size);
    if (buffer == NULL) {
        perror("Error allocating memory");
        exit(1);
    }
    ssize_t bytesRead = pread(file, buffer, size, offset);
    if (bytesRead == -1) {
        perror("Error reading file");
        exit(1);
    }
    printf("Read %zd bytes from file '%s':\n", bytesRead, filename);
    //printf("%.*s\n", (int)bytesRead, buffer);
    fwrite(buffer, 1, bytesRead, stdout);
    printf("\n");
    free(buffer);
    close(file);
}

// Function to write data to a file at a specified offset
void writeFile(char *filename, off_t offset, char *data) {
    int file = open(filename, O_WRONLY);
    if (file == -1) {
        perror("Error opening file");
        exit(1);
    }
    ssize_t bytesWritten = pwrite(file, data, strlen(data), offset);
    if (bytesWritten == -1) {
        perror("Error writing to file");
        exit(1);
    }
    printf("Written %zd bytes to file '%s' at offset %ld.\n", bytesWritten, filename, offset);
    close(file);
}

// Function to display information about a file
void displayFileInfo(char *filename) {
    struct stat fileStat;
    int result = stat(filename, &fileStat);
    if (result == -1) {
        perror("Error getting file information");
        exit(1);
    }
    printf("File Information for '%s':\n", filename);
    printf("Owner: %d\n", fileStat.st_uid);
    printf("Permissions: %o\n", fileStat.st_mode & 0777);
    printf("Inode: %ld\n", fileStat.st_ino);
    printf("Last Access Time: %ld\n", fileStat.st_atime);
    printf("Last Modification Time: %ld\n", fileStat.st_mtime);
    printf("Last Status Change Time: %ld\n", fileStat.st_ctime);
}

// Function to copy the content of one file to another file
void copyFileContent(char *sourceFilename, char *destinationFilename) {
    int sourceFile = open(sourceFilename, O_RDONLY);
    if (sourceFile == -1) {
        perror("Error opening source file");
        exit(1);
    }
    int destinationFile = open(destinationFilename, O_WRONLY | O_CREAT, 0644);
    if (destinationFile == -1) {
        perror("Error opening destination file");
        exit(1);
    }
    char buffer[4096];
    ssize_t bytesRead, bytesWritten;
    while ((bytesRead = read(sourceFile, buffer, sizeof(buffer))) > 0) {
        bytesWritten = write(destinationFile, buffer, bytesRead);
        if (bytesWritten == -1) {
            perror("Error writing to destination file");
            exit(1);
        }
    }
    if (bytesRead == -1) {
        perror("Error reading from source file");
       

 exit(1);
    }
    printf("File '%s' copied to '%s' successfully.\n", sourceFilename, destinationFilename);
    close(sourceFile);
    close(destinationFile);
}

// Function ot copy file contents using unnamed pipe 
void copyFileContentUsingPipe(char *sourceFilename, char *destinationFilename) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Error creating pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Error forking process");
        exit(1);
    } else if (pid == 0) {
        // Child process - read from pipe and write to destination file
        close(pipefd[1]); // Close the write end of the pipe

        int destinationFile = open(destinationFilename, O_WRONLY | O_CREAT, 0644);
        if (destinationFile == -1) {
            perror("Error opening destination file");
            exit(1);
        }

        char buffer[4096];
        ssize_t bytesRead, bytesWritten;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            bytesWritten = write(destinationFile, buffer, bytesRead);
            if (bytesWritten == -1) {
                perror("Error writing to destination file");
                exit(1);
            }
        }

        if (bytesRead == -1) {
            perror("Error reading from pipe");
            exit(1);
        }

        close(pipefd[0]); // Close the read end of the pipe
        close(destinationFile);
        printf("File '%s' copied to '%s' successfully.\n", sourceFilename, destinationFilename);

        exit(0);
    } else {
        // Parent process - read from source file and write to pipe
        close(pipefd[0]); // Close the read end of the pipe

        int sourceFile = open(sourceFilename, O_RDONLY);
        if (sourceFile == -1) {
            perror("Error opening source file");
            exit(1);
        }

        char buffer[4096];
        ssize_t bytesRead, bytesWritten;
        while ((bytesRead = read(sourceFile, buffer, sizeof(buffer))) > 0) {
            bytesWritten = write(pipefd[1], buffer, bytesRead);
            if (bytesWritten == -1) {
                perror("Error writing to pipe");
                exit(1);
            }
        }

        if (bytesRead == -1) {
            perror("Error reading from source file");
            exit(1);
        }

        close(pipefd[1]); // Close the write end of the pipe
        close(sourceFile);
        wait(NULL); // Wait for the child process to finish

        exit(0);
    }
}
// Function to create a named pipe with specified permissions
void createNamedPipe(char *pipeName, int mode) {
    int result = mkfifo(pipeName, mode);
    if (result == -1) {
        perror("Error creating named pipe");
        exit(1);
    }
    printf("Named pipe '%s' created successfully.\n", pipeName);
}

// Function to read from or write to a named pipe
void communicateThroughPipe(char *pipeName, int mode) {
    char buffer[4096];
    if (mode == O_RDONLY) {
        int pipe = open(pipeName, O_RDONLY);
        if (pipe == -1) {
            perror("Error opening named pipe");
            exit(1);
        }
        printf("Reading from named pipe '%s'.\n", pipeName);

        
        ssize_t bytesRead;
        /*while ((bytesRead = read(pipe, buffer, sizeof(buffer))) >= 0) {
            printf("Received %zd bytes from named pipe:\n", bytesRead);
            printf("%.*s\n", (int)bytesRead, buffer);
        }*/
        if (bytesRead == -1) {
            perror("Error reading from named pipe");
            close(pipe);
            exit(1);
        }
        printf("Data read from named pipe: %s\n", buffer);
        close(pipe);
    } 
    else if (mode == O_WRONLY) {
        int pipe = open(pipeName, O_WRONLY);
        if (pipe == -1) {
            perror("Error opening named pipe");
            exit(1);
        }
        
        printf("Enter data to write (max %d characters): ", 4096);
        fflush(stdout); // Flush the output buffer
        //scanf("%*c");
        //fgets(buffer, sizeof(buffer), stdin);
        //buffer[strcspn(buffer, "\n")] = '\0';
        ssize_t bytestoWrite = read(0, buffer, sizeof(buffer));
        if (bytestoWrite == -1) {
            perror("Error reading from standard input");
            exit(1);
        }
        ssize_t bytesWritten = write(pipe, buffer, bytestoWrite);
        if (bytesWritten == -1) {
            perror("Error writing to named pipe");
            close(pipe);
            exit(1);
        }
        printf("data is now written to the named pipe");
        close(pipe);
    } 
    else {
        printf("Invalid pipe mode.\n");
        exit(1);
    }

}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <task> [arguments]\n", argv[0]);
        return 1;
    }

    char *task = argv[1];

    if (strcmp(task, "create-file") == 0) {
        if (argc != 4) {
            printf("Usage: %s create-file <filename> <permissions>\n", argv[0]);
            return 1;
        }
        char *filename = argv[2];
        mode_t permissions = strtol(argv[3], NULL, 8);
        createFile(filename, permissions);
    } 
    else if (strcmp(task, "read-file") == 0) {
        if (argc != 5) {
            printf("Usage: %s read-file <filename> <offset> <size>\n", argv[0]);
            return 1;
        }
        char *filename = argv[2];
        off_t offset = atol(argv[3]);
        size_t size = atol(argv[4]);
        readFile(filename, offset, size);
    } 
    else if (strcmp(task, "write-file") == 0) {
        if (argc != 5) {
            printf("Usage: %s write-file <filename> <offset> <data>\n", argv[0]);
            return 1;
        }
        char *filename = argv[2];
        off_t offset = atol(argv[3]);
        char *data = argv[4];
        writeFile(filename, offset, data);
    } 
    else if (strcmp(task, "display-file-info") == 0) {
        if (argc != 3) {
            printf("Usage: %s display-file-info <filename>\n", argv[0]);
            return 1;
        }
        char *filename = argv[2];
        displayFileInfo(filename);
    } 
    else if (strcmp(task, "copy-file-by-pipe") == 0) {
        if (argc != 4) {
            printf("Usage: %s copy-file <source-file> <destination-file>\n", argv[0]);
            return 1;
        }
        char *sourceFilename = argv[2];
        char *destinationFilename = argv[3];
        copyFileContentUsingPipe(sourceFilename, destinationFilename);
    } 
    else if (strcmp(task, "copy-file") == 0) {
        if (argc != 4) {
            printf("Usage: %s copy-file <source-file> <destination-file>\n", argv[0]);
            return 1;
        }
        char *sourceFilename = argv[2];
        char *destinationFilename = argv[3];
        copyFileContent(sourceFilename, destinationFilename);
    } 
    else if (strcmp(task, "create-named-pipe") == 0) {
        if (argc != 4) {
            printf("Usage: %s create-named-pipe <pipe-name> <permissions>\n", argv[0]);
            return 1;
        }
        char *pipeName = argv[2];
        mode_t permissions = strtol(argv[3], NULL, 8);
        createNamedPipe(pipeName, permissions);
    } 
    else if (strcmp(task, "communicate-through-pipe") == 0) {
        if (argc != 4) {
            printf("Usage: %s communicate-through-pipe <pipe-name> <mode>\n", argv[0]);
            return 1;
        }
        char *pipeName = argv[2];
        int mode = strcmp(argv[3], "read") == 0 ? O_RDONLY : O_WRONLY;
        communicateThroughPipe(pipeName, mode);
    } 
    else {
        printf("Invalid task: %s\n", task);
        return 1;
    }

    return 0;
}