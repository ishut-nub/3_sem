#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    int p_pid = getpid();
    printf("parent %d\n", p_pid);
    int p_ppid = getppid();
    printf("parent %d\n", p_ppid);
    int num = atoi(argv[1]);
    for (int i = 0; i<num; i++)
    {
        int fk = fork();
        if (!fk)
        {
            int pid = getpid();
            printf("kid %d\n", pid);
            //int ppid = getppid();
            //printf("kid %d\n", ppid);
            break;
        }

    }
    return 0;
}
