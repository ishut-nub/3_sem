#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    char buf[BUFFER_SIZE] = {0};
    int fds[2];

    if (pipe(fds) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) { //child
        close(fds[0]);
        if (argc > 1) {
            for (int i = 1; i < argc; i++) {
                int fd = open(argv[i], O_RDONLY);
                if (fd < 0) {
                    perror("open fail");
                    return 1;
                }
                ssize_t bytes_read;
                while ((bytes_read = read(fd, buf, BUFFER_SIZE)) > 0) {
                    write(fds[1], buf, bytes_read);
                }
                close(fd);
            }
        }
        else {
            ssize_t bytes_read;
            while ((bytes_read = read(0, buf, BUFFER_SIZE)) > 0) {
                write(fds[1], buf, bytes_read);
            }
        }
        close(fds[1]);
        exit(0);
    }
    //parent
    close(fds[1]);
    ssize_t bytes_write;
    while ((bytes_write = read(fds[0], buf, BUFFER_SIZE)) > 0) {
        write(1, buf, bytes_write);
    }
    close(fds[0]);
    int st;
    waitpid(pid, &st, 0);
    return 0;
}
