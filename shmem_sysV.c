#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
struct sembuf p = {0, -1, SEM_UNDO};
struct sembuf v = {0, 1, SEM_UNDO};
int main()
{
    int n = 10;
    scanf("%d", &n);
    int id = shmget(IPC_PRIVATE, 1024, 0666);

    int *count = (int*) shmat(id, (void*)0, 0);
    *count = 0;

    int sem_id = semget(IPC_PRIVATE, 1, 0666);
    semctl(sem_id, 0, SETVAL, 1);

    for (int i = 1; i <= n; i++) {
        pid_t runner_pid = fork();
        if (runner_pid == 0) {
            for (int j = 0; j<100000; j++) {
                semop(sem_id, &p, 1);
                (*count)++;
                semop(sem_id, &v, 1);
            }
            exit(0);
        }
    }
    for (int i = 1; i <= n; i++) {
        wait(NULL);
    }
    printf("%d\n", *count);
    shmdt(count);
    shmctl(id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    return 0;
}
