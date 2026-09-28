#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>

#define buf 4096

int main(int argc, char* argv[])
{
    if (argc <= 1) {
        printf("usage: %s <cmd> [args...]\n", argv[0]);
        return 1;
    }

    struct rusage usage;
    struct timeval t1, t2;
    int st;
    double elapsed_time, elapsed_time_user, elapsed_time_sys;

    int fds[2];
    if (pipe(fds) < 0)
    {
        perror("pipe");
        return 1;
    }

    gettimeofday(&t1, NULL);

    int fk = fork();
    if (fk < 0)
    {
        perror("fork");
        return 1;
    }

    if (fk == 0) //child
    {
        close(fds[0]);

        if (dup2(fds[1], 1) == -1)
        {
            perror("dup2");
            return 1;
        }
        close(fds[1]);

        execvp(argv[1], argv + 1);
        perror("no exec");
        return 1;
    }

    //parent
    close(fds[1]);

    int lines = 0, words = 0, bytes = 0;
    int in_word = 0;
    char buffer[buf];
    ssize_t bytes_read;

    while ((bytes_read = read(fds[0], buffer, sizeof(buffer))) > 0)
    {
        bytes += bytes_read;
        for (ssize_t i = 0; i < bytes_read; i++)
        {
            if (buffer[i] == '\n') {
                lines++;
            }
            if (isspace((unsigned char)buffer[i]))
            {
                in_word = 0;
            }
            else if (!in_word)
            {
                in_word = 1;
                words++;
            }
        }
    }
    close(fds[0]);

    wait4(fk, &st, 0, &usage);
    gettimeofday(&t2, NULL);

    elapsed_time = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    elapsed_time_user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
    elapsed_time_sys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;

    printf("%d %d %d\n", lines, words, bytes);

    printf("\n %lf", elapsed_time);
    printf("\n %lf", elapsed_time_user);
    printf("\n %lf\n", elapsed_time_sys);

    return 0;
}
