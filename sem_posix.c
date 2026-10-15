#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>


int main()
{
    int n = 5;
    scanf("%d", &n);
    sem_t *s;
    s = sem_open("/empty", O_CREAT, 0677, 1);
    for (int i = 0; i<n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(1);
        }
        if (pid == 0)
        {
            sem_wait(s);
            printf("pid: %d ", getpid());
            fflush(stdout);
            printf("hello world");
            fflush(stdout);
            printf("\n");
            fflush(stdout);
            sem_post(s);
            sem_close(s);
            exit(0);
        }
    }
    for (int i = 0; i <= n; i++) {
        wait(NULL);
    }
    sem_close(s);
    sem_unlink("/empty");

    printf("\n\n\n");
    for (int i = 0; i<n; i++) { //no semaphores
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(1);
        }
        if (pid == 0)
        {
            printf("pid: %d ", getpid());
            fflush(stdout);
            printf("hello world");
            fflush(stdout);
            printf("\n");
            fflush(stdout);
            exit(0);
        }
    }
    for (int i = 0; i <= n; i++) {
        wait(NULL);
    }
    return 0;
}
