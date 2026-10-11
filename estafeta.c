#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define MSG_REGISTER 1
#define MSG_START 2
#define MSG_FINISH 3

struct msgbuf {
    long mtype;
    int sid;
    int fid;
};

void runner(int i, int id)
{
    struct msgbuf msg;
    msg.mtype = MSG_REGISTER;
    msg.sid = 0;
    msg.fid = i;
    if (msgsnd(id, &msg, sizeof(msg)-sizeof(long), 0) == -1) { //RESISTRATION
        perror("msgsnd error");
        exit(1);
    }
    printf("%d arrived\n", i); //i REGISTERED
    if (msgrcv(id, &msg, sizeof(msg)-sizeof(long), MSG_START, 0) == -1) {
        perror("msgrcv start error");
        exit(1);
    }
    while(1) {
        if (msgrcv(id, &msg, sizeof(msg)-sizeof(long), MSG_FINISH, 0) == -1) { //RECIEVING
            perror("msgrcv finish error");
            exit(1);
        }
        if (msg.sid == i) { //IF OURS
            printf("%d has the stick\n", i);
            usleep(10000);

            int next = (i == 10) ? 0 : i + 1;
            msg.mtype = MSG_FINISH;
            msg.sid = next;
            msg.fid = i;
            printf("%d send the stick\n", i); //SENDING STICK TO NEXT
            if (msgsnd(id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
                perror("msgsnd finish error");
                exit(1);
            }
            break;
        } else { //IF NOT OURS
            if (msgsnd(id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
                perror("not for us error");
                exit(1);
            }
            usleep(10000);
        }

    }
}
void judge(int id)
{
    struct msgbuf msg;
    printf("waiting for 10 runners\n"); //WAITING
    for (int i = 1; i <= 10; i++) { //CHECK REGISTRATION
        if (msgrcv(id, &msg, sizeof(msg)-sizeof(long), MSG_REGISTER, 0) == -1) {
            perror("msgrcv register error");
            exit(1);
        }
        printf("%d arrived\n", msg.fid);
    }
    printf("all arrived\n"); //ARRIVAL COMFIRMATION

    for (int i = 1; i<=10; i++) { //SENDING SIGNAL START
        msg.mtype = MSG_START;
        msg.sid = 0;
        if (msgsnd(id, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
            perror("msgsnd start error");
            exit(1);
        }
    }
    printf("START: judge -> first\n");
    msg.mtype = MSG_FINISH;
    msg.sid = 1;
    msg.fid = 0;
    if (msgsnd(id, &msg, sizeof(msg)-sizeof(long), 0) == -1) { //SENDING THE STICK TO FIRST
        perror("judge failed -> first");
        exit(1);
    }

    while(1) { //WAITING FOR LAST STICK
        if (msgrcv(id, &msg, sizeof(msg)-sizeof(long), MSG_FINISH, 0) == -1) {
            perror("failed to recieve last");
            exit(1);
        }

        if (msg.sid == 0) { //IF OURS -> FINISH
            printf("FINISH: last runner -> judge\n");
            break;
        } else {
            if (msgsnd(id, &msg, sizeof(msg)-sizeof(long), 0) == -1) { //IF NOT OURS SEND
                perror("not judges stick");
                exit(1);
            }
            usleep(10000);
        }
    }
}

int main()
{
    key_t key = ftok("estafeta.c", 101);
    if (key == -1) {
        perror("ftok fail");
        return 1;
    }
    int id = msgget(key, IPC_CREAT | 0777);
    if (id < 0) {
        perror("msgget fail");
        return 1;
    }
    pid_t judge_pid = fork();
    if (judge_pid == 0) {
        judge(id);
        exit(0);
    }
    for (int i = 1; i <= 10; i++) {
        pid_t runner_pid = fork();
        if (runner_pid == 0) {
            runner(i, id);
            exit(0);
        }
    }
    for (int i = 0; i <= 10; i++) {
        wait(NULL);
    }

    if (msgctl(id, IPC_RMID, NULL) == -1) {
        perror("msgctl fail");
        return 1;
    }
    printf("SUCCESS\n");
    return 0;
}
