#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>


#define buffer 4096

int main()
{
    char buf[buffer] = {0};
    int fds[2] = {0};
    if (pipe(fds) < 0)
    {
        perror("pipe fail\n");
        return 0;
    }
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        close(fds[1]);
        ssize_t bytes = read(fds[0], buf, sizeof(buf)-1);
        if (bytes>0)
        {
            printf("child got %s\n", buf);
        }
        close(fds[0]);
    }
    else
    {
        close(fds[0]);
        const char *par = "from parent\n";
        printf("hi from child\n");
        write(fds[1], par, 12);
        close(fds[1]);
    }
    return 0;
}
