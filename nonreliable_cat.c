#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

unsigned char b;
int c;

void on_bit(int s)
{
    signal(s, on_bit);
    b = (b << 1) | (s == SIGUSR2);
    if (++c == 8) {
        if(write(STDOUT_FILENO, &b, 1) < 0)
            _exit(1);
        b = 0;
        c = 0;
    }
    kill(getppid(), SIGUSR1);
}

void on_term(int s)
{
    (void)s;
    _exit(0);
}

int fd, bit = -1;
unsigned char cur;
pid_t wpid;

void send_next()
{
    if (bit < 0) {
        if (read(fd, &cur, 1) != 1) {
            kill(wpid, SIGTERM);
            waitpid(wpid, NULL, 0);
            _exit(0);
        }
        bit = 7;
    }
    kill(wpid, ((cur>>bit) & 1) ? SIGUSR2 : SIGUSR1);
    bit--;
}

void on_ack(int s)
{
    signal(s, on_ack);
    send_next();
}

int main(int argc, char **argv)
{
    fd = STDIN_FILENO;
    if (argc > 1 && (fd = open(argv[1], O_RDONLY)) < 0) {
        perror(argv[1]);
        return 1;
    }

    signal(SIGUSR1, on_bit);
    signal(SIGUSR2, on_bit);
    signal(SIGTERM, on_term);
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid > 0) {
        wpid = pid;
        signal(SIGUSR1, on_ack);
        send_next();
    }
    while(1)
        pause();
}
