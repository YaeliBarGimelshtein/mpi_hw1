#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


enum ranks{ROOT};
enum task{WORK, STOP};



int checkPrime(int num)
{
    if (num <= 1)
        return 0; 
   
   for (int j = 2; j <= num/2; j++)
   {
      if (num % j == 0)
        return 0;
   }
   return 1;
}


void workerProcess(int size)
{
    int* work_arr = (int*)malloc(sizeof(int)*size);
    int* res_arr = (int*)malloc(sizeof(int)*size);
    int tag;
    MPI_Status status;
    do
    {
        MPI_Recv(work_arr,size,MPI_INT,ROOT,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        tag = status.MPI_TAG;

        for (int i = 0; i < size; i++)
        {
            res_arr[i] = checkPrime(work_arr[i]);
        }

        MPI_Send(res_arr,size,MPI_INT,ROOT,tag,MPI_COMM_WORLD);
    }
    while(tag != STOP);
    //FREE ALL MEMORY AND MPI AND FINISH THE PROG
    free(work_arr);
    free(res_arr);
}


void masterProcess(int num_procs, int chunk_size)
{
    MPI_Status status;
    int* arr, input_size, tag, jobs_sent=0, num_workers, worker_id, jobs_total, jobs_left, *prime_or_not;

    
    //GET THE INPUT
    printf("Enter the size of input\n");
    scanf("%d",&input_size);
    arr = (int*)malloc(sizeof(int)*input_size);
    for (int i = 0; i < input_size; i++)
    {
        printf("Enter the %d number\n", i+1);
        scanf("%d",&arr[i]);
    }
    
    int chunck = chunk_size;
    
    prime_or_not = (int*)malloc(sizeof(int)*input_size);

    //SET THE NUMBERS
    jobs_total = input_size / chunck;
    num_workers = num_procs-1;
    int pointerToRecive = 0;

    //START WORKERS
    for(worker_id =1; worker_id<num_procs; worker_id++)
    {
        MPI_Send(arr+(jobs_sent*chunck), chunck ,MPI_INT, worker_id, WORK, MPI_COMM_WORLD);
        jobs_sent ++;
    }
    
    //RECIVE AND SEND MORE WORK
    for(;jobs_sent < jobs_total;)
    {
        //CHECK IF LAST ROUND
        jobs_left = jobs_total-jobs_sent;
        if(jobs_left <= num_workers)
            tag = STOP;
        else
            tag = WORK;

        //GET THE DATA AND SEND MORE
        MPI_Recv(prime_or_not+(pointerToRecive*chunck),chunck,MPI_INT,MPI_ANY_SOURCE,WORK,MPI_COMM_WORLD,&status);
        MPI_Send(arr+(jobs_sent*chunck), chunck ,MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
        jobs_sent ++;
        pointerToRecive ++;
    }

    //RECIVE FINAL WORK
    for(worker_id =1; worker_id<num_procs; worker_id++)
    {
        MPI_Recv(prime_or_not+(pointerToRecive*chunck),chunck,MPI_INT,MPI_ANY_SOURCE,STOP,MPI_COMM_WORLD,&status);
        pointerToRecive ++;
    }
    
    printf("here are all prime numbers in this list\n");
    for (int i = 0; i < input_size; i++)
    {
        if(prime_or_not[i] == 1)
        {
            printf("%d\n", arr[i]);
        }
    }
    //FREE ALL MEMORY AND MPI AND FINISH THE PROG
    free(arr);
    free(prime_or_not);
}


int main(int argc, char *argv[])
{
    //GENERAL INTEGERS NEEDED
    int my_rank, num_procs;
    
    //MPI INIT
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); 

    //GET THE CHUNCK SIZE FOR EACH PROCESS
    int chunk;
    if(my_rank == ROOT)
    {
        printf("Enter the size of chunk\n");
        scanf("%d",&chunk);
    }
    MPI_Bcast(&chunk, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    
   
    if(my_rank == ROOT)
        masterProcess(num_procs, chunk);
    else
        workerProcess(chunk);

    MPI_Finalize();
    return 0;
}