#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    for (int i = 1; i<argc; i++)
    {
        int fk = fork();
        if (fk == 0)
        {
            int x = atoi(argv[i]);
            usleep(x*5000);
            printf(" %d", x);
            break;
        }
    }
    int st;
    while(wait(&st)>0){}
    printf("\n");
    return 0;
}
