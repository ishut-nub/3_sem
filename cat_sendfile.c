#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {
            char *args[] = {"./mycat_a", NULL};
            execvp("./mycat_a", args);
            perror("execvp");
            _exit(127);
        }

        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return -1;
        }
    }

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);

        if (fd == -1) {
            perror(argv[i]);
            continue;
        }

        ssize_t n;

        while (1) {
            n = sendfile(STDOUT_FILENO, fd, NULL, 4096);

            if (n == 0)
                break;

            if (n == -1) {
                perror("sendfile");
                break;
            }
        }

        if (n == -1)
            perror("sendfile");

        close(fd);
    }

    return 0;
}
