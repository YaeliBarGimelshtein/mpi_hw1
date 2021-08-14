#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ROOT 0




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


void workerProcess(int size, int input_size)
{
    int* work_arr = (int*)malloc(sizeof(int)*size);
    int tag;
    MPI_Status status;

    while(tag < (input_size+1))
    {
        MPI_Recv(work_arr,size,MPI_INT,ROOT,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        tag = status.MPI_TAG;

        for (int i = 0; i < size; i++)
        {
            work_arr[i]=checkPrime(work_arr[i]);
        }

        MPI_Send(work_arr,size,MPI_INT,ROOT,tag,MPI_COMM_WORLD);
    }

    //FREE ALL MEMORY AND MPI AND FINISH THE PROG
    free(work_arr);
}


void masterProcess(int num_procs, int chunk_size, int input)
{
    MPI_Status status;
    int* arr, input_size, jobs_sent=0, num_workers, worker_id, jobs_total, *prime_or_not;

    
    //GET THE INPUT
    input_size = input;
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
    


    int tag_id_of_arr_index = 0;
    
    int sent = 0;

    //START WORKERS
    for(worker_id =1; worker_id<num_procs; worker_id++)
    {
        MPI_Send(arr+(jobs_sent*chunck), chunck ,MPI_INT, worker_id, tag_id_of_arr_index, MPI_COMM_WORLD);
        jobs_sent ++;
        tag_id_of_arr_index += chunck;
        sent ++;
    }
    
    
    //RECIVE AND SEND MORE WORK
    while(sent != 0)
    {
        //GET THE DATA AND SEND MORE
        MPI_Recv(prime_or_not+status.MPI_TAG,chunck,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        sent --;
        
        if(jobs_sent < jobs_total)
        {
            MPI_Send(arr+(jobs_sent*chunck), chunck ,MPI_INT, status.MPI_SOURCE, tag_id_of_arr_index, MPI_COMM_WORLD);
            jobs_sent ++;
            tag_id_of_arr_index += chunck;
            sent ++;
        } 
    }
    
    
    //SEND THE SIGNAL FOR PROCESSES TO DIE
    int stop = input_size + 1;
    for(worker_id =1; worker_id<num_procs; worker_id++)
    {
        MPI_Send(&tag_id_of_arr_index, 1 ,MPI_INT, worker_id, stop, MPI_COMM_WORLD);
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
    int chunk, input_size;
    if(my_rank == ROOT)
    {
        printf("Enter the size of chunk\n");
        scanf("%d",&chunk);
        printf("Enter the size of input\n");
        scanf("%d",&input_size);
    }
    MPI_Bcast(&chunk, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    
   
    if(my_rank == ROOT)
        masterProcess(num_procs, chunk, input_size);
    else
        workerProcess(chunk, input_size);

    MPI_Finalize();
    return 0;
}