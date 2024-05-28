#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>
#define READ_END 0
#define WRITE_END 1
#define BUFFER_SIZE 25

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
    struct msg_buffer message;
    key_t key;
    int msgid;

    system("touch msgq.txt");

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

    printf("Enter Plane ID: ");
    int planeID;
    scanf("%d", &planeID);

    printf("Enter Type of Plane: ");
    int planeType;
    scanf("%d", &planeType);

    int num_pass = 0;
    int total_weight = 0;

    message.msg_type = 11;
    message.planeID = planeID;
    message.planeType = planeType;

    if (planeType == 1) // passenger plane
    {
        printf("Enter Number of Occupied Seats: ");
        scanf("%d", &num_pass);

        int pipe_fd[num_pass][2];

        int read_msg[num_pass][BUFFER_SIZE];
        for (int i = 0; i < num_pass; i++)
        {
            if (pipe(pipe_fd[i]) == -1) // Create pipes for communication
            {
                perror("pipe");
                return 1;
            }
            else
            {
                // printf("\npipe created for child : %d\n", i + 1);
            }
            pid_t child_pid = fork();

            if (child_pid == -1)
            {
                // Error handling
                perror("fork");
                return 1;
            }
            else if (child_pid == 0) // This is the child/passenger  process
            {
                close(pipe_fd[i][READ_END]);
                int write_msg[BUFFER_SIZE];

                printf("Enter Weight of Your Luggage: ");
                int lug_weight;
                scanf("%d", &lug_weight);

                printf("Enter Your Body Weight: ");
                int body_weight;
                scanf("%d", &body_weight);

                write_msg[0] = lug_weight;
                write_msg[1] = body_weight;

                // checking message to be sent

                // printf("%d ", write_msg[0]);
                // printf("%d\n", write_msg[1]);

                write(pipe_fd[i][WRITE_END], write_msg, sizeof(write_msg));

                // Close the write end of the pipe before exiting
                close(pipe_fd[i][WRITE_END]);
                return 0;
            }
            else
            {
                // plane process
                wait(NULL);
                close(pipe_fd[i][WRITE_END]); // Close unused write end
                read(pipe_fd[i][READ_END], read_msg[i], BUFFER_SIZE);
                close(pipe_fd[i][READ_END]);
            }
        }
        // checking message that is received
        // printf("Read:\n");
        // for (int k = 0; k < num_pass; k++)
        // {
        //     printf("Passenger %d luggage: %d ", k + 1, read_msg[k][0]);
        //     printf("Passenger %d body: %d\n", k + 1, read_msg[k][1]);
        // }
        for (int k = 0; k < num_pass; k++)
        {
            total_weight += read_msg[k][0] + read_msg[k][1];
        }
        total_weight += (75 * 7);
    }

    else // cargo plane
    {
        printf("Enter Number of Cargo Items: ");
        int cargo_num;
        scanf("%d", &cargo_num);

        printf("Enter Average Weight of Cargo Items: ");
        int cargo_weight;
        scanf("%d", &cargo_weight);

        total_weight = (cargo_num * cargo_weight) + (75 * 2);
    }

    printf("Enter Airport Number for Departure: ");
    int dep_airport;
    scanf("%d", &dep_airport);

    printf("Enter Airport Number for Arrival: ");
    int arr_airport;
    scanf("%d", &arr_airport);

    if (dep_airport == arr_airport)
    {
        printf("Arrival and Departure Airport cannot be the same");
        return 0;
    }
    message.arr_airport = arr_airport;
    message.dep_airport = dep_airport;
    message.num_pass = num_pass;
    message.total_weight = total_weight;

    if (msgsnd(msgid, (void *)&message, sizeof(message), 0) == -1)
    {
        printf("error in sending message");
        exit(1);
    }
    else
        // printf("message sent and arrival airport: %d\n", message.arr_airport);

        while (1)
        {

            if (msgrcv(msgid, (void *)&message, sizeof(message), 35 + planeID, 0) == -1)
            {
                printf("error in sending message");
                exit(1);
            }
            else
            {
                printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n", message.planeID, message.dep_airport, message.arr_airport);
                break;
            }
        }

    return 0;
}