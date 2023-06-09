#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {

    char *myFile = NULL;
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    //modified
    char myFile[] = argv[1];
    //code to open myFile
    int fd = open(myFile, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }

    //If user executes GET
    if (command == 'get')
    {
        //code to read myFile
        #define BUFFER_SIZE 4096
        int bytes_read = 0;
        char buf[BUFFER_SIZE];
        do {
            bytes_read = read(fd, buf, BUFFER_SIZE);
            if (bytes_read < 0) {
                fprintf(stderr, "Operation Failed\n");
                return 1;
            } else if (bytes_read > 0) {
                int bytes_written = 0;
                do {
                    //code to write file content to stdout (only if GET)
                    int bytes = write(STDOUT_FILENO, buf + bytes_written, bytes_read - bytes_written);
                    if (bytes <= 0) {
                        fprintf(stderr, "Operation Failed\n");
                        return 1;
                    }
                    bytes_written += bytes;
                } while (bytes_written < bytes_read);
            }
        } while (bytes_read > 0);
    }
    
    //code to read myFile
    #define BUFFER_SIZE 4096
    int bytes_read = 0;
    char buf[BUFFER_SIZE];
    do {
        bytes_read = read(fd, buf, BUFFER_SIZE);
        if (bytes_read < 0) {
            fprintf(stderr, "Operation Failed\n");
            return 1;
        } else if (bytes_read > 0) {
            int bytes_written = 0;
            do {
                //code to write file content to stdout (only if GET)
                int bytes = write(STDOUT_FILENO, buf + bytes_written, bytes_read - bytes_written);
                if (bytes <= 0) {
                    fprintf(stderr, "Operation Failed\n");
                    return 1;
                }
                bytes_written += bytes;
            } while (bytes_written < bytes_read);
        }
    } while (bytes_read > 0);

    //make errors go away
    (void) myFile;
    

//scanf("%[^\0]%*c", myFile)

//code to read filename and write it to stdout
#define BUFFER_SIZE 4096
    int bytes_read = 0;
    char buf[BUFFER_SIZE];
    do {
        bytes_read = read(fd, buf, BUFFER_SIZE);
        if (bytes_read < 0) {
            fprintf(stderr, "Operation Failed\n");
            return 1;
        } else if (bytes_read > 0) {
            int bytes_written = 0;
            do {
                int bytes = write(STDOUT_FILENO, buf + bytes_written, bytes_read - bytes_written);
                if (bytes <= 0) {
                    fprintf(stderr, "Operation Failed\n");
                    return 1;
                }
                bytes_written += bytes;
            } while (bytes_written < bytes_read);
        }
    } while (bytes_read > 0);
}
