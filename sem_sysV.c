#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int n = 0;
    scanf("%d", &n);

    int id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);
    struct sembuf ops1;
        ops1.sem_num = 0;
        ops1.sem_op = n;
        ops1.sem_flg = 0;
    semop(id, &ops1, 1);

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(1);
        }
        if (pid == 0)
        {
            printf("READY SET player %d\n", getpid());

            struct sembuf ops2;

            ops2.sem_num = 0;
            ops2.sem_op = -1;
            ops2.sem_flg = 0;

            struct sembuf ops3;

            ops3.sem_num = 0;
            ops3.sem_op = 0;
            ops3.sem_flg = 0;

            semop(id, &ops2, 1);
            semop(id, &ops3, 1);


            printf("GO player %d\n", getpid());
            exit(0);
        }
    }
    for (int i = 0; i <= n; i++) {
        wait(NULL);
    }
    semctl(id, 0, IPC_RMID, 0);
    return 0;
}
