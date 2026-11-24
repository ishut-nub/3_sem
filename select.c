#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/resource.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

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

int set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
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
        _exit(127);
    }


    close(out_fds[1]);
    close(err_fds[1]);

    if (set_nonblock(out_fds[0]) == -1 || set_nonblock(err_fds[0]) == -1)
    {
        perror("fcntl");
        return 1;
    }

    struct counter cnt[2] = {{0}, {0}};
    int fd[2] = { out_fds[0], err_fds[0] };
    int open_cnt = 2;
    char buffer[BUF];

    while (open_cnt > 0)
    {
        fd_set rset;
        FD_ZERO(&rset);
        int maxfd = -1;

        for (int i = 0; i < 2; i++)
        {
            if (fd[i] < 0)
                continue;
            FD_SET(fd[i], &rset);
            if (fd[i] > maxfd)
                maxfd = fd[i];
        }

        int r = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (r < 0)
        {
            if (errno == EINTR)
                continue;
            perror("select");
            break;
        }

        for (int i = 0; i < 2; i++)
        {
            if (fd[i] < 0 || !FD_ISSET(fd[i], &rset))
                continue;

            ssize_t n = read(fd[i], buffer, sizeof(buffer));
            if (n > 0)
            {
                count(&cnt[i], buffer, n);
            }
            else if (n == 0)
            {
                close(fd[i]);
                fd[i] = -1;
                open_cnt--;
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR);
            else
            {
                perror("read");
                close(fd[i]);
                fd[i] = -1;
                open_cnt--;
            }
        }
    }

    wait4(fk, &st, 0, &usage);
    gettimeofday(&t2, NULL);

    double elapsed = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    double t_user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
    double t_sys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;

    printf("stdout: %ld %ld %ld\n", cnt[0].lines, cnt[0].words, cnt[0].bytes);
    printf("stderr: %ld %ld %ld\n", cnt[1].lines, cnt[1].words, cnt[1].bytes);

    printf("\n %lf", elapsed);
    printf("\n %lf", t_user);
    printf("\n %lf\n", t_sys);

    return 0;
}
