#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>

#define BUF 4096

struct counter {
    long lines, words, bytes;
    int in_word;
};

void count(struct counter *c, const char *b, ssize_t n)
{
    c->bytes += n;
    for (ssize_t i = 0; i < n; i++)
    {
        if (b[i] == '\n')
            c->lines++;
        if (isspace((unsigned char)b[i]))
            c->in_word = 0;
        else if (!c->in_word)
        {
            c->in_word = 1;
            c->words++;
        }
    }
}

void drain(int fd, struct counter *c)
{
    char buffer[BUF];
    ssize_t n;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0)
        count(c, buffer, n);
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        printf("usage: %s <cmd> [args...]\n", argv[0]);
        return 1;
    }

    struct rusage usage;
    struct timeval t1, t2;
    int st;

    int out_fds[2], err_fds[2];
    if (pipe(out_fds) < 0 || pipe(err_fds) < 0)
    {
        perror("pipe");
        return 1;
    }

    gettimeofday(&t1, NULL);

    pid_t fk = fork();
    if (fk < 0)
    {
        perror("fork");
        return 1;
    }

    if (fk == 0)
    {
        close(out_fds[0]);
        close(err_fds[0]);

        if (dup2(out_fds[1], 1) == -1 || dup2(err_fds[1], 2) == -1)
        {
            perror("dup2");
            _exit(1);
        }
        close(out_fds[1]);
        close(err_fds[1]);

        execvp(argv[1], argv + 1);
        perror("no exec");
        _exit(1);
    }


    close(out_fds[1]);
    close(err_fds[1]);

    struct counter out = {0}, err = {0};

    drain(out_fds[0], &out);
    drain(err_fds[0], &err);

    wait4(fk, &st, 0, &usage);
    gettimeofday(&t2, NULL);

    double elapsed = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    double t_user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
    double t_sys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;

    printf("stdout: %ld %ld %ld\n", out.lines, out.words, out.bytes);
    printf("stderr: %ld %ld %ld\n", err.lines, err.words, err.bytes);

    printf("\n %lf", elapsed);
    printf("\n %lf", t_user);
    printf("\n %lf\n", t_sys);

    return 0;
}
