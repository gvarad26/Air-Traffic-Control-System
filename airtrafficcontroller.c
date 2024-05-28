#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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
    // File Pointer declared
    FILE *fptr;

    // File opened
    fptr = fopen("./AirTrafficController.txt", "w+");

    // Failed Condition
    if (fptr == NULL)
    {
        printf("Error Occurred While writing to a text file !\n");
        exit(1);
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
    printf("Enter the number of airports to be handled/managed: ");
    int num_airport;
    scanf("%d", &num_airport);
    int count = 0;
    int flag = 0;
    while (1)
    {

        while (msgrcv(msgid, (void *)&message, sizeof(message), 11, IPC_NOWAIT) != -1)
        {

            // printf("Message received from message queue: %d %d %d %d %d %d\n", message.planeID, message.planeType, message.num_pass, message.total_weight, message.arr_airport, message.dep_airport);
            count++;
            message.msg_type = message.dep_airport;
            if (msgsnd(msgid, (void *)&message, sizeof(message), 0) == -1)
            {
                printf("error in sending message");
                exit(1);
            }
            else
            {
                // printf("message sent and departure airport: %d with plane id %d\n", message.dep_airport,message.planeID);
            }
        }

        while (msgrcv(msgid, (void *)&message, sizeof(message), 12, IPC_NOWAIT) != -1)
        {

            char str[100];
            memset(str, '\0', 100 * sizeof(char));
            sprintf(str, "Plane %d has departed from Airport %d and will land at Airport %d.\n", message.planeID, message.dep_airport, message.arr_airport);
            fputs(str, fptr);
            printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n", message.planeID, message.fin_runway+1, message.dep_airport);

            message.msg_type = message.arr_airport;
            if (msgsnd(msgid, (void *)&message, sizeof(message), 0) == -1)
            {
                printf("error in sending message");
                exit(1);
            }
            else
            {
                // printf("message sent and arrival airport: %d with plane id %d\n", message.arr_airport,message.planeID);
            }
        }

        while (msgrcv(msgid, (void *)&message, sizeof(message), 13, IPC_NOWAIT) != -1)
        {

            printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n", message.planeID, message.fin_runway+1, message.arr_airport);

            count--;

            message.msg_type = 35 + message.planeID;
            if (msgsnd(msgid, (void *)&message, sizeof(message), 0) == -1)
            {
                printf("error in sending message");
                exit(1);
            }
            else
            {
                // printf("message sent to plane for termination and message type is %ld\n", message.msg_type);
            }
        }

        while (msgrcv(msgid, (void *)&message, sizeof(message), 14, IPC_NOWAIT) != -1)
        {
            flag = 1;
        }

        if (flag == 1 && count == 0)
        {
            message.msg_type = 16;
            if (msgsnd(msgid, (void *)&message, sizeof(message), 0) == -1)
            {
                printf("error in sending message");
                exit(1);
            }
            else
            {
                // printf("message sent to airport for termination and message type is %ld\n", message.msg_type);
            }
            break;
        }
    }
    // printf("message queue: done receiving messages.\n");
    system("rm msgq.txt");

    if (msgctl(msgid, IPC_RMID, 0) == -1)
    {
        printf("error in deleting\n");
        exit(1);
    }
    printf("message queue: deleted\n");

    return 0;
}