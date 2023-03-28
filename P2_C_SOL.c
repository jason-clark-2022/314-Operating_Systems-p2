#define NUM_REPEAT 50 // each boiler-man repeats *CHANGE LATER

#define BATHER_TIME_01_A 300000 // 300ms = 0.3 seconds
#define BATHER_TIME_01_B 800000 // 800ms = 0.8 seconds

#define BATHER_TIME_02_A 300000 // 300ms = 0.3 seconds
#define BATHER_TIME_02_B 800000 // 800ms = 0.8 seconds

#define BATHER_TIME_03_A 300000 // 300ms = 0.3 seconds
#define BATHER_TIME_03_B 800000 // 800ms = 0.8 seconds

#define BOILERMAN_TIME_01_A 1200000 // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_01_B 1600000 // 1600ms = 1.6 seconds

#define BOILERMAN_TIME_02_A 1200000 // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_02_B 1600000 // 1600ms = 1.6 seconds

#define SAFEGUARD_TIME_A 1200000 // 1200ms = 1.2 seconds
#define SAFEGUARD_TIME_B 1600000 // 1600ms = 1.6 seconds

#define SEM_KEY1 5770
#define SEM_KEY2 5771
#define SEM_KEY3 5772
#define SHM_KEY 7570

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void boiler(int boilerID, int semTwo, struct sembuf operations[1]);
void bather(int batherID, int semTwo, struct sembuf operations[1]);
void millisleep(unsigned ms);

//MAIN
int main(void)
{
    pid_t process_id; // process_id holder

    int shmID;   // the shared memory ID
    int shmSize; // the size of the shared memoy
    struct my_mem *pshm;
    struct sembuf operations[1]; // Define semaphore operations

    int semOne; // wait semaphore
    int semTwo; // critical section semaphore
    int semThree;
    int returnVal;

    // Semaphore control data structure
    union semun
    {
        int val;
        struct semid_ds *buf;
        ushort *arry;
    } argument;
    argument.val = 1; // the initial value of the semaphore

    // shared memory definition
    struct my_mem
    {
        int counter;
        int aCounter;
    };

    // find the shared memory size in bytes
    shmSize = sizeof(struct my_mem);
    if (shmSize <= 0)
    {
        printf("sizeof error in acquiring the shared memory size. Terminating...\n");
        exit(0);
    }

    // create semaphore 1
    semOne = semget(SEM_KEY1, 1, 0666 | IPC_CREAT);
    if (semOne < 0)
    {
        printf("Failed to create semaphore one. Terminating...\n");
        exit(0);
    }
    // create semaphore 2
    semTwo = semget(SEM_KEY2, 1, 0666 | IPC_CREAT);
    if (semTwo < 0)
    {
        printf("Failed to create semaphore two. Terminating...\n");
        exit(0);
    }
    // create semaphore 3 for exiting
    semThree = semget(SEM_KEY3, 1, 0666 | IPC_CREAT);
    if (semThree < 0)
    {
        printf("Failed to create semaphore three. Terminating...\n");
        exit(0);
    }

    // initialize semaphore 1
    if (semctl(semOne, 0, SETVAL, argument) < 0)
    {
        printf("Failed to initialize semaphore one. Terminating...\n");
        exit(0);
    }

    // initialize semaphore2
    if (semctl(semTwo, 0, SETVAL, argument) < 0)
    {
        printf("Failed to initialize semaphore two. Terminating...\n");
        exit(0);
    }
    argument.val = 0;
    // initialize semaphore 3
    if (semctl(semThree, 0, SETVAL, argument) < 0)
    {
        printf("Failed to initialize semaphore three. Terminating...\n");
        exit(0);
    }

    // create a shared memory
    shmID = shmget(SHM_KEY, shmSize, 0666 | IPC_CREAT);
    if (shmID < 0)
    {
        printf("Failed to create the shared memory. Terminating...\n");
        exit(0);
    }
    // attach the new shared memory
    pshm = (struct my_mem *)shmat(shmID, NULL, 0);
    if (pshm == (struct my_mem *)-1)
    {
        printf("Failed to attach the shared memory.  Terminating...\n");
        exit(0);
    }

    pshm->counter = 0;

    //Create first child process
    process_id = fork();

    if (process_id == 0)
    {
        printf("B1 Process Created...\n");
        printf("B1 is waiting for other processes to become active...\n");
        pshm->counter = pshm->counter + 1;

        operations[0].sem_num = 0;
        operations[0].sem_op = -1;
        operations[0].sem_flg = 0;
        returnVal = semop(semOne, operations, 1);
        if (returnVal != 0)
        {
            printf("Wait failure on B1...\n");
        }

        boiler(1, semTwo, operations);

        pshm->counter = pshm->counter - 1;
        operations[0].sem_num = 0;
        operations[0].sem_op = 1;
        operations[0].sem_flg = 0;
        returnVal = semop(semThree, operations, 1);
        if (returnVal != 0)
        {
            printf("Signal failure on semThree...\n");
        }
        printf("        B1 Finished...\n");
        exit(0);
    }
    else
    {
        process_id = fork();
        if (process_id == 0)
        {
            printf("B2 Process Created...\n");
            printf("B2 is waiting for other processes to become active...\n");
            pshm->counter = pshm->counter + 1;
            operations[0].sem_num = 0;
            operations[0].sem_op = -1;
            operations[0].sem_flg = 0;
            returnVal = semop(semOne, operations, 1);
            if (returnVal != 0)
            {
                printf("Wait failure on B2...\n");
            }

            boiler(2, semTwo, operations);

            pshm->counter = pshm->counter - 1;
            operations[0].sem_num = 0;
            operations[0].sem_op = 1;
            operations[0].sem_flg = 0;
            returnVal = semop(semThree, operations, 1);
            if (returnVal != 0)
            {
                printf("Signal failure on semThree...\n");
            }
            printf("        B2 Finished...\n");
            exit(0);
        }
        else
        {
            process_id = fork();
            if (process_id == 0)
            {
                printf("A1 Process Created...\n");
                printf("A1 is waiting for other processes to become active...\n");
                operations[0].sem_num = 0;
                operations[0].sem_op = -1;
                operations[0].sem_flg = 0;
                returnVal = semop(semOne, operations, 1);
                if (returnVal != 0)
                {
                    printf("Wait failure on A1...\n");
                }

                while (pshm->counter > 0)
                {
                    bather(1, semTwo, operations);
                }
                operations[0].sem_num = 0;
                operations[0].sem_op = 1;
                operations[0].sem_flg = 0;
                returnVal = semop(semThree, operations, 1);
                if (returnVal != 0)
                {
                    printf("Signal failure on semThree...\n");
                }
                printf("        A1 Finished...\n");
                exit(0);
            }
            else
            {
                process_id = fork();
                if (process_id == 0)
                {
                    printf("A2 Process Created...\n");
                    printf("A2 is waiting for other processes to become active...\n");
                    operations[0].sem_num = 0;
                    operations[0].sem_op = -1;
                    operations[0].sem_flg = 0;
                    returnVal = semop(semOne, operations, 1);
                    if (returnVal != 0)
                    {
                        printf("Wait failure on A2...\n");
                    }

                    while (pshm->counter > 0)
                    {
                        bather(2, semTwo, operations);
                    }
                    operations[0].sem_num = 0;
                    operations[0].sem_op = 1;
                    operations[0].sem_flg = 0;
                    returnVal = semop(semThree, operations, 1);
                    if (returnVal != 0)
                    {
                        printf("Signal failure on semThree...\n");
                    }
                    printf("        A2 Finished...\n");
                    exit(0);
                }
                else
                {
                    process_id = fork();
                    if (process_id == 0)
                    {
                        printf("A3 Process Created...\n");
                        printf("A3 is waiting for other processes to become active...\n");
                        operations[0].sem_num = 0;
                        operations[0].sem_op = -1;
                        operations[0].sem_flg = 0;
                        returnVal = semop(semOne, operations, 1);
                        if (returnVal != 0)
                        {
                            printf("Wait failure on A3...\n");
                        }

                        while (pshm->counter > 0)
                        {
                            bather(3, semTwo, operations);
                        }
                        operations[0].sem_num = 0;
                        operations[0].sem_op = 1;
                        operations[0].sem_flg = 0;
                        returnVal = semop(semThree, operations, 1);
                        if (returnVal != 0)
                        {
                            printf("Signal failure on semThree...\n");
                        }
                        printf("        A3 Finished...\n");
                        exit(0);
                    }
                    else
                    {
                        printf("S Process Created...\n");
                        for (int i = 0; i < 5; i++)
                        {
                            operations[0].sem_num = 0;
                            operations[0].sem_op = 1;
                            operations[0].sem_flg = 0;
                            returnVal = semop(semOne, operations, 1);
                            if (returnVal != 0)
                            {
                                printf("Signal failure on activating children processes...\n");
                            }
                        }
                        while (pshm->counter > 0)
                        {
                            millisleep(SAFEGUARD_TIME_A);
                            operations[0].sem_num = 0;
                            operations[0].sem_op = -1;
                            operations[0].sem_flg = 0;
                            returnVal = semop(semTwo, operations, 1);
                            if (returnVal != 0)
                            {
                                printf("Wait failure on S...\n");
                            }
                            printf("S starts inspection...\n");
                            millisleep(SAFEGUARD_TIME_B);
                            printf("    S finishes inspection...\n");
                            operations[0].sem_num = 0;
                            operations[0].sem_op = 1;
                            operations[0].sem_flg = 0;
                            returnVal = semop(semTwo, operations, 1);
                            if (returnVal != 0)
                            {
                                printf("Signal Failure S...\n");
                            }
                        }
                        for (int i = 0; i < 5; i++)
                        {
                            operations[0].sem_num = 0;
                            operations[0].sem_op = -1;
                            operations[0].sem_flg = 0;
                            returnVal = semop(semThree, operations, 1);
                            if (returnVal != 0)
                            {
                                printf("Signal failure on S...\n");
                            }
                        }

                        returnVal = shmdt(pshm);
                        if (returnVal != 0)
                        {
                            printf("Detach Memory Failure...\n");
                        }
                        returnVal = shmctl(shmID, IPC_RMID, 0);
                        if (returnVal != 0)
                        {
                            printf("Shared Memory Deletion Failure...\n");
                        }
                        returnVal = semctl(semOne, IPC_RMID, 0);
                        if (returnVal != 0)
                        {
                            printf("Semaphore One Deletion Failure...\n");
                        }
                        returnVal = semctl(semTwo, IPC_RMID, 0);
                        if (returnVal != 0)
                        {
                            printf("Semaphore Two Deletion Failure...\n");
                        }
                        returnVal = semctl(semThree, IPC_RMID, 0);
                        if (returnVal != 0)
                        {
                            printf("Semaphore Three Deletion Failure...\n");
                        }

                        exit(0);
                    }
                }
            }
        }
    }
}
//sleep function
void millisleep(unsigned targetRand)
{
    long int myRand;
    long int temp1;

    if (targetRand >= 1000)
    {
        temp1 = (targetRand / 1000);
        myRand = (rand() % temp1);
        myRand = myRand * 1000;
    }
    else
    {
        // for any crazy inputs, set target to 1000
        if (targetRand < 0)
        {
            targetRand = 1000;
        }

        myRand = (rand() % targetRand);
        myRand = myRand * 1000;
    }

    usleep(myRand);
}

void boiler(int boilerID, int semTwo, struct sembuf operations[1])
{
    int returnVal;

    for (int i = 0; i < NUM_REPEAT; i++)
    {
        millisleep(BOILERMAN_TIME_01_A);
        operations[0].sem_num = 0;
        operations[0].sem_op = -1;
        operations[0].sem_flg = 0;
        returnVal = semop(semTwo, operations, 1);
        if (returnVal != 0)
        {
            printf("Wait Failure B%d Critical Section...\n", boilerID);
        }
        printf("B%d starts his water heater...\n", boilerID);
        millisleep(BOILERMAN_TIME_01_B);
        printf("    B%d finishes water heating...\n", boilerID);

        operations[0].sem_num = 0;
        operations[0].sem_op = 1;
        operations[0].sem_flg = 0;
        returnVal = semop(semTwo, operations, 1);
        if (returnVal != 0)
        {
            printf("Failure B%d Critical Section...\n", boilerID);
        }
    }
}

void bather(int batherID, int semTwo, struct sembuf operations[1])
{
    int returnVal;

    millisleep(BATHER_TIME_01_A);
    operations[0].sem_num = 0;
    operations[0].sem_op = -1;
    operations[0].sem_flg = 0;
    returnVal = semop(semTwo, operations, 1);
    if (returnVal != 0)
    {
        printf("Wait Failure A%d Critical Section...\n", batherID);
    }
    printf("A%d is entering the swimming pool...\n", batherID);
    millisleep(BATHER_TIME_01_B);
    printf("    A%d is leaving the swimming pool...\n", batherID);
    operations[0].sem_num = 0;
    operations[0].sem_op = 1;
    operations[0].sem_flg = 0;
    returnVal = semop(semTwo, operations, 1);
    if (returnVal != 0)
    {
        printf("Signal Failure A%d Critical Section...\n", batherID);
    }
}