#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>


int main(int argc, char* argv[])
{
    struct rusage usage;
    struct timeval t1, t2;
    int st;
    double elapsed_time, elapsed_time_user, elapsed_time_sys;

    gettimeofday(&t1, NULL);

    int fk = fork();

    if (fk == 0)
    {
        execvp(argv[1], argv+1);
        perror("disappointment");
    }
    wait4(fk, &st, 0, &usage);
    printf("%d", st);
    int a = WEXITSTATUS(st);
    int b = WTERMSIG(st);


    gettimeofday(&t2, NULL);
    sleep(10);

    elapsed_time = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    elapsed_time_user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
    elapsed_time_sys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;

    printf("\n%lf", elapsed_time);
    printf("\n%lf", elapsed_time_user);
    printf("\n%lf\n", elapsed_time_sys);
    printf("\n%d\n", st);
    printf("%d\n", a);
    printf("%d\n", b);
    return 0;
}
