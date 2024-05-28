#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <semaphore.h>

sem_t mutex[12];

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

struct runway
{
    int arr_ori[11];
    int index[11];
    int size;
    int total_weight;
    int planeID;
    int num_airport;
    int msgid;
    struct msg_buffer main_message;
};

void *runway_allocate_depart(void *param)
{

    struct runway *runway_depart = (struct runway *)param;
    int fin_runway;
    for (int i = 0; i < runway_depart->size; i++)
    {
        if (runway_depart->arr_ori[runway_depart->index[i]] < runway_depart->total_weight)
        {
            continue;
        }
        else
        {
            fin_runway = runway_depart->index[i];
            break;
        }
    }

    sem_wait(&mutex[fin_runway]);

    sleep(3); // boarding/loading process
    sleep(2); // takeoff

    runway_depart->main_message.fin_runway = fin_runway;

    runway_depart->main_message.msg_type = 12;

    if (msgsnd(runway_depart->msgid, (void *)&runway_depart->main_message, sizeof(runway_depart->main_message), 0) == -1)
    {
        printf("error in sending message");
        exit(1);
    }
    else
    {
        printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n", runway_depart->planeID, fin_runway + 1, runway_depart->num_airport);
    }
    sem_post(&mutex[fin_runway]);
}
void *runway_allocate_arrive(void *param)
{

    struct runway *runway_arrive = (struct runway *)param;
    int fin_runway;
    for (int i = 0; i < runway_arrive->size; i++)
    {
        if (runway_arrive->arr_ori[runway_arrive->index[i]] < runway_arrive->total_weight)
        {
            continue;
        }
        else
        {
            fin_runway = runway_arrive->index[i];
            break;
        }
    }

    sleep(30); // flight simulation
    // printf("30 second khatam hue\n");
    sem_wait(&mutex[fin_runway]);

    sleep(2); // plane’s landing
    sleep(3); // deboarding/unloading
    runway_arrive->main_message.fin_runway = fin_runway;

    runway_arrive->main_message.msg_type = 13;

    if (msgsnd(runway_arrive->msgid, (void *)&runway_arrive->main_message, sizeof(runway_arrive->main_message), 0) == -1)
    {
        printf("error in sending message");
        exit(1);
    }
    else
    {
        printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n", runway_arrive->planeID, fin_runway + 1, runway_arrive->num_airport);
    }
    sem_post(&mutex[fin_runway]);
}

int main()
{
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
    printf("Enter Airport Number: ");
    int num_airport;
    scanf("%d", &num_airport);

    printf("Enter number of Runways: ");
    int num_runways;
    scanf("%d", &num_runways);

    printf("Enter loadCapacity of Runways (give as a space separated list in a single line): ");
    int load_capacity[num_runways + 1];
    for (int i = 0; i < num_runways; i++)
    {
        scanf("%d", &load_capacity[i]);
    }
    load_capacity[num_runways] = 15000;

    int n = num_runways + 1;
    int lcc[n];
    int lcc_original[n];
    for (int k = 0; k < n; k++)
    {
        lcc[k] = load_capacity[k];
        lcc_original[k] = load_capacity[k];
    }
    // for (int i = 0; i < n; i++) // printing input/copy array
    // {
    //     printf("%d ", lcc[i]);
    // }
    // printf("\n");
    int i;
    int j;

    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (load_capacity[j] > load_capacity[j + 1])
            {
                int temp = load_capacity[j];
                load_capacity[j] = load_capacity[j + 1];
                load_capacity[j + 1] = temp;
            }
        }
    }
    // for (int i = 0; i < n; i++) // printing sorted array
    // {
    //     printf("%d ", load_capacity[i]);
    // }

    // printf("\n");
    int index_array[n];
    for (int j = 0; j < n; j++)
    {
        for (int k = 0; k < n; k++)
        {
            if (load_capacity[j] == lcc[k])
            {
                index_array[j] = k;
                lcc[k] = -1;
                break;
            }
        }
    }
    // for (int i = 0; i < n; i++) // printing sorted array
    // {
    //     printf("%d ", index_array[i]);
    // }

    for (int i = 0; i < 12; i++)
    {
        sem_init(&mutex[i], 0, 1);
    }

    pthread_t tid[20];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int thr_index = 0;
    struct runway argu[20];
    int argu_index = 0;
    while (1)
    {
        if (msgrcv(msgid, (void *)&message, sizeof(message), 16, IPC_NOWAIT) == -1)
        {
        }
        else
        {
            for (int i = 0; i < thr_index; i++)
            {
                pthread_join(tid[i], NULL);
            }
            break;
        }

        if (msgrcv(msgid, (void *)&message, sizeof(message), num_airport, 0) == -1)
        {
            exit(1);
        }

        // printf("\nMessage received from message queue: %d %d %d %d %d %d\n", message.planeID, message.planeType, message.num_pass, message.total_weight, message.arr_airport, message.dep_airport);

        for (int i = 0; i < n; i++)
        {
            argu[argu_index].arr_ori[i] = lcc_original[i];
            argu[argu_index].index[i] = index_array[i];
        }

        argu[argu_index].size = n;
        argu[argu_index].total_weight = message.total_weight;
        argu[argu_index].planeID = message.planeID;
        argu[argu_index].num_airport = num_airport;
        argu[argu_index].msgid = msgid;
        argu[argu_index].main_message = message;

        if (message.dep_airport == num_airport)
        {
            pthread_create(&tid[thr_index], &attr, runway_allocate_depart, (void *)&argu[argu_index]);
            thr_index++;
        }
        if (message.arr_airport == num_airport)
        {
            pthread_create(&tid[thr_index], &attr, runway_allocate_arrive, (void *)&argu[argu_index]);
            thr_index++;
        }
        argu_index++;
    }

    // system("rm msgq.txt"); //to remove file
    for (int i = 0; i < 12; i++)
    {
        sem_destroy(&mutex[i]);
    }

    // printf("message queue: done receiving messages.\n");
    printf("airport has terminated\n");
    return 0;
}