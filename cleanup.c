#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>

struct msg_buffer
{
    long msg_type;
    int dep_airport;
    int arr_airport;
    int planeID;
    int total_weight;
    int planeType;
    int num_pass;
    int fin_runway;
};

int main()
{
    char n, t;

    while (1)
    {
        printf("Do you want the Air Traffic Control System to terminate?(Y for Yes and N for No)");
        scanf("%c", &n);
        scanf("%c", &t);
        fflush(stdin);

        if (n == 'Y' || n == 'y')
            break;
    }
    struct msg_buffer message;
    key_t key;
    int msgid;
    system("touch msgq.txt"); // to create the file

    key = ftok("msgq.txt", 'L');
    if (key == -1)
    {
        printf("error in creating unique key\n");
        exit(1);
    }

    msgid = msgget(key, 0644 | IPC_CREAT);
    if (msgid == -1)
    {
        printf("error in creating message queue\n");
        exit(1);
    }
    // printf("Now sending termination messages to everyone first atc\n");
    message.msg_type = 14;
    if (msgsnd(msgid, (void *)&message, sizeof(message), 0) == -1)
    {
        printf("error in sending message");
        exit(1);
    }
    else
    {
        // printf("message sent to ATC with message type: %ld\n", message.msg_type);
    }
}
