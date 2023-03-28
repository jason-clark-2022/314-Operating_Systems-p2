/* **************************************
* CS314 Project #2 solution             *
* Jason Clark                           *
* Your last-three:  442                 *
* Your course section #: 003            *
*                                       *
* Spring 2021                           *
*                                       *
* ***************************************/
//Semaphore keys 6030-6039	
//Shared-Memory keys 7630-7639

#define NUM_REPEAT                  50   // each boiler-man repeats
#define BATHER_TIME_01_A            300000 // 300ms = 0.3 seconds
#define BATHER_TIME_01_B            800000 // 800ms = 0.8 seconds
#define BATHER_TIME_02_A            300000 // 300ms = 0.3 seconds
#define BATHER_TIME_02_B            800000 // 800ms = 0.8 seconds
#define BATHER_TIME_03_A            300000 // 300ms = 0.3 seconds
#define BATHER_TIME_03_B            800000 // 800ms = 0.8 seconds
#define BOILERMAN_TIME_01_A         1200000 // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_01_B         1600000 // 1600ms = 1.6 seconds
#define BOILERMAN_TIME_02_A         1200000 // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_02_B         1600000 // 1600ms = 1.6 seconds
#define SAFEGUARD_TIME_A            1200000 // 1200ms = 1.2 seconds
#define SAFEGUARD_TIME_B            1600000 // 1600ms = 1.6 seconds

#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <unistd.h>  
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>   

#define SEM_KEY_1                   6030       // the semaphore 1 key
#define SEM_KEY_2                   6031       // the semaphore 2 key
#define SEM_KEY_3                   6032       // the semaphore 3 key
#define SHM_KEY                     7630       // the shared memory key 

void signalOperation(int num, int op, int flg);
void millisleep(unsigned ms);
void bather (int bather_ID);
void boiler (int boilerman_ID);

pid_t  process_id;            // process id
int    process_tag;         
int    sem_id_1;              // semaphore 1 ID  
int    sem_id_2;              // semaphore 2 ID  
int    sem_id_3;              // semaphore 3 ID  
struct sembuf operations[1];  // Define semaphore operations 
int    ret_val;               // system-call return value    
int    shm_id;                // the shared memory ID 
int    shm_size;              // the size of the shared memoy  
struct my_mem * p_shm; 
union semun {
    int    val;  
    struct semid_ds  *buf;  
    ushort * arry;
} argument; 
   
// shared memory definition ----   
struct my_mem {
    long int counter;
    int      boiler;
    int      bather;  
};        

int main (void)
{
    argument.val = 0;   // the initial value of the semaphore   
    sem_id_1 = SEM_KEY_1;           
    sem_id_2 = SEM_KEY_2;              
    sem_id_3 = SEM_KEY_3;             
   // Semaphore control data structure ----


   // find the shared memory size in bytes ----
   shm_size = sizeof(struct my_mem);   
   if (shm_size <= 0)
   {  
      fprintf(stderr, "sizeof error in acquiring the shared memory size. Terminating ..\n");
      exit(0); 
   }  

   // create new semaphore 1
   sem_id_1 = semget(SEM_KEY_1, 1, 0666 | IPC_CREAT); 
   if (sem_id_1 < 0)
   {
      fprintf(stderr, "Failed to create a new semaphore 1. Terminating ..\n"); 
      exit(0);
   }

    // create new semaphore 2
    sem_id_2 = semget(SEM_KEY_2, 1, 0666 | IPC_CREAT); 
    if (sem_id_2 < 0)
    {
        fprintf(stderr, "Failed to create new semaphore 2. Terminating ..\n"); 
        exit(0);
    }

    // create new semaphore 3
    sem_id_3 = semget(SEM_KEY_3, 1, 0666 | IPC_CREAT); 
    if (sem_id_3 < 0)
    {
        fprintf(stderr, "Failed to create new semaphore 3. Terminating ..\n"); 
        exit(0);
    }

    // initialzie the new semapahore 1 by 0
    if (semctl(sem_id_1, 0, SETVAL, argument) < 0)
    {
        fprintf(stderr, "Failed to initialize the semaphore (1) by 0. Terminating ..\n"); 
        exit(0);  
    }

    argument.val = 1;
    // initialzie the new semapahore 2 by 1
    if (semctl(sem_id_2, 0, SETVAL, argument) < 0)
    {
        fprintf(stderr, "Failed to initialize the semaphore (2) by 1. Terminating ..\n"); 
        exit(0);  
    }
    

    // initialzie the new semapahore 3 by 1
    if (semctl(sem_id_3, 0, SETVAL, argument) < 0)
    {
        fprintf(stderr, "Failed to initialize the semaphore (3) by 1. Terminating ..\n"); 
        exit(0);  
    }

    // create a shared memory ----
    shm_id = shmget(SHM_KEY, shm_size, 0666 | IPC_CREAT);         
    if (shm_id < 0) 
    {
        fprintf(stderr, "Failed to create the shared memory. Terminating ..\n");  
        exit(0);  
    } 

    // attach the new shared memory ----
    p_shm = (struct my_mem *)shmat(shm_id, NULL, 0);     
    if (p_shm == (struct my_mem*) -1)
    {
        fprintf(stderr, "Failed to attach the shared memory.  Terminating ..\n"); 
        exit(0);   
    }   

    // initialize the shared memory ----
    p_shm->counter = 0;
    p_shm->boiler  = 0;  
    p_shm->bather  = 0;   
    


    process_id = fork();

    if (process_id == 0)
    {
        printf("B1 is born...\n");
        printf("    B1 is waiting for other children to become active...\n");
        process_tag = 1;
        p_shm->boiler = p_shm->boiler + 1;
        signalOperation(0,-1,0);

        ret_val = semop(sem_id_1, operations, 1);
        if (ret_val != 0)
        {
            printf("Initial wait failure on B1...\n");
        }
    }
    else
    {
        usleep(100);
        process_id = fork();
        if (process_id == 0)
        {
            printf("B2 is born...\n");
            printf("    B2 is waiting for other children to become active...\n");
            process_tag = 2;
            p_shm->boiler = p_shm->boiler + 1;
            signalOperation(0,-1,0);
            ret_val = semop(sem_id_1, operations, 1);

            if (ret_val != 0)
            {
                printf("Initial wait failure on B2...\n");
            }
        }
        else
        {
            usleep(100);
            process_id = fork();
            if (process_id == 0)
            {
                printf("A1 is born...\n");
                printf("    A1 is waiting for other children to become active...\n");
                process_tag = 3;
                p_shm->bather = p_shm->bather + 1;
                signalOperation(0,-1,0);

                ret_val = semop(sem_id_1, operations, 1);
                if (ret_val != 0)
                {
                    printf("Initial wait failure on A1...\n");
                }
            }
            else
            {
                usleep(100);
                process_id = fork();
                if (process_id == 0)
                {
                    printf("A2 is born...\n");
                    printf("    A2 is waiting for other children to become active...\n");
                    process_tag = 4;
                    p_shm->bather = p_shm->bather + 1;
                    signalOperation(0,-1,0);

                    ret_val = semop(sem_id_1, operations, 1);
                    if (ret_val != 0)
                    {
                        printf("Initial wait failure on A2...\n");
                    }
                }
                else
                {
                    usleep(100);
                    process_id = fork();
                    if (process_id == 0)
                    {
                        printf("A3 is born...\n");
                        printf("    A3 is waiting for other children to become active...\n");
                        process_tag = 5;
                        p_shm->bather = p_shm->bather + 1;
                        signalOperation(0,-1,0);

                        ret_val = semop(sem_id_1, operations, 1);
                        if (ret_val != 0)
                        {
                            printf("Initial wait failure on A3...\n");
                        }
                    }
                    else
                    {
                        usleep(500);
                        process_tag = 6;
                        for(int i = 0; i < 5; i++)
                        {
                            signalOperation(0,1,0);
                            ret_val = semop(sem_id_1, operations, 1);
                            if (ret_val != 0)
                            {
                                printf("Signal failure on activating children processes...\n");
                            }
                        }
                    }
                }
            }
        }
    }

    
    if(process_tag <= 2) // boiler
    {
        boiler(process_tag);
        p_shm->boiler--;

        exit(0);
    }

    else if((process_tag > 2) && (process_tag <= 5)) // bather
    {
        while(p_shm->boiler>0)
        {
            bather(process_tag);
        }
        p_shm->bather--;

        exit(0);
    }

    else // Safeguard
    {
        while(p_shm->bather>0)
        {
            millisleep(SAFEGUARD_TIME_A);
            signalOperation(0,-1,0);
            ret_val = semop(sem_id_2, operations, 1);
            if (ret_val != 0)
            {
                printf("Wait failure on S...\n");
                
            }
            printf("S starts inspection. Everyone get out!!\n");
            millisleep(SAFEGUARD_TIME_B);
            printf("    S finishes inspection. Anyone can get in the pool!\n");
            signalOperation(0,1,0);

            ret_val = semop(sem_id_2, operations, 1);
            if (ret_val != 0)
            {
                printf("Wait failure on S...\n");   
            }
        }

        if(p_shm->boiler == 0 && p_shm->bather == 0)
        {
            ret_val = shmdt(p_shm);  
            if (ret_val != 0) 
            {  printf ("shared memory detach failed ....\n"); }

            ret_val = shmctl(shm_id, IPC_RMID, 0); 
            if (ret_val != 0)
            {  printf("shared memory remove ID failed ... \n"); } 

            ret_val = semctl(sem_id_1, IPC_RMID, 0);  
            if (ret_val != 0)
            {  printf("semaphore 1 remove ID failed ... \n"); }

            ret_val = semctl(sem_id_2, IPC_RMID, 0);  
            if (ret_val != 0)
            {  printf("semaphore 2 remove ID failed ... \n"); }

            ret_val = semctl(sem_id_3, IPC_RMID, 0);  
            if (ret_val != 0)
            {  printf("semaphore 3 remove ID failed ... \n"); }

            exit(0);
        }
    }
}

void bather (int bather_ID)
{
    int bather_time_a;
    int bather_time_b;
    int bather_id_temp;

    if(bather_ID == 3)
    {
        bather_time_a = BATHER_TIME_01_A;
        bather_time_b = BATHER_TIME_01_B;
        bather_id_temp = 1;
    }
    else if(bather_ID == 4)
    {
        bather_time_a = BATHER_TIME_02_A;
        bather_time_b = BATHER_TIME_02_B;
        bather_id_temp = 2;
    }
    else if(bather_ID == 5)
    {
        bather_time_a = BATHER_TIME_03_A;
        bather_time_b = BATHER_TIME_03_B;
        bather_id_temp = 3;
    }
    else
    {
        bather_time_a = -1;
        bather_time_b = -1;
        bather_id_temp = 0;
    }
 
    millisleep(bather_time_a);
    signalOperation(0,-1,0);
    ret_val = semop(sem_id_3, operations, 1);

    p_shm->counter++;

    if(p_shm->counter==1)
    {
        signalOperation(0,-1,0);
        ret_val = semop(sem_id_2, operations, 1);
    }
    
    printf("A%d is entering the swimming pool... \n", bather_id_temp);
    signalOperation(0,1,0);
    ret_val = semop(sem_id_3, operations, 1);
    
    millisleep(bather_time_b);


    signalOperation(0,-1,0);
    ret_val = semop(sem_id_3, operations, 1);
    p_shm->counter--;
    if(p_shm->counter==0)
    {
        signalOperation(0,1,0);
        ret_val = semop(sem_id_2, operations, 1);
    }
    printf("    A%d is leaving the swimming pool... \n", bather_id_temp);
    signalOperation(0,1,0);
    ret_val = semop(sem_id_3, operations, 1);
}

void boiler(int boilerman_ID)
{
    int boilerman_time_a;
    int boilerman_time_b;

    if(boilerman_ID == 1)
    {
        boilerman_time_a = BOILERMAN_TIME_01_A;
        boilerman_time_b = BOILERMAN_TIME_01_B;
    }
    else if(boilerman_ID == 2)
    {
        boilerman_time_a = BOILERMAN_TIME_02_A;
        boilerman_time_b = BOILERMAN_TIME_02_B;
    }
    else
    {
        boilerman_time_a = -1;
        boilerman_time_b = -1;
    }

    for(int i=0; i < NUM_REPEAT; i++)
    {
        millisleep(boilerman_time_a);

        signalOperation(0,-1,0);
        ret_val = semop(sem_id_2, operations, 1);

        if(ret_val != 0)
        {
            printf("Boiler B%d Initial Wait Failure...\n", boilerman_ID);
        }

        printf("B%d starts his water heater...\n", boilerman_ID);
        millisleep(boilerman_time_b);
        printf("    B%d finishes water heating...\n", boilerman_ID);
        
        signalOperation(0,1,0);
        ret_val = semop(sem_id_2, operations, 1);

        if(ret_val != 0)
        {
            printf("Boiler B%d Final Wait Failure...\n", boilerman_ID);
        }
    }
}


void millisleep(unsigned ms)
{
    long int my_rand;    // a random number
    long int temp1;

    temp1 = (ms/1000);
    my_rand = (rand() % temp1);
    my_rand =  my_rand * 1000;

    usleep(my_rand);
}

void signalOperation(int num, int op, int flg)
{
    operations[0].sem_num = num;
    operations[0].sem_op  = op;
    operations[0].sem_flg = flg;
}