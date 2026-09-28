#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>


int main(int argc, char* argv[])
{
    struct rusage usage1, usage2;
    struct timeval t1, t2;

    double elapsed_time, elapsed_time_user, elapsed_time_sys;

    gettimeofday(&t1, NULL);


    int fk = fork();

    getrusage(RUSAGE_CHILDREN, &usage1);

    if (fk == 0)
    {
        execvp(argv[1], argv+1);
        perror("disappointment");
    }
    int st;
    while(wait(&st)>0){}

    getrusage(RUSAGE_CHILDREN, &usage2);
    gettimeofday(&t2, NULL);

    elapsed_time = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) / 1000000.0;
    elapsed_time_user = (usage2.ru_utime.tv_sec - usage1.ru_utime.tv_sec) + (usage2.ru_utime.tv_usec - usage1.ru_utime.tv_usec)/ 1000000.0;
    elapsed_time_sys = (usage2.ru_stime.tv_sec - usage1.ru_stime.tv_sec) + (usage2.ru_stime.tv_usec - usage1.ru_stime.tv_usec)/ 1000000.0;

    printf("\n %lf", elapsed_time);
    printf("\n %lf", elapsed_time_user);
    printf("\n %lf\n", elapsed_time_sys);

    return 0;
}
