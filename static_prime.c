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


int main(int argc, char *argv[])
{
    //GENERAL INTEGERS NEEDED
    int my_rank, num_procs, input_size;
    int* arr;
    
    //MPI INIT
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); 

    //GET THE INPUT
    if(my_rank == ROOT)
    {
        printf("Enter the size of input\n");
        scanf("%d",&input_size);
    
        arr = (int*)malloc(sizeof(int)*input_size);
        for (int i = 0; i < input_size; i++)
        {
            printf("Enter the %d number\n", i+1);
            scanf("%d",&arr[i]);
        }
    }
    MPI_Bcast(&input_size, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    
    //CALCULATING FOR EACH PROCESS HOW MANY NUMBERS TO CHECK
    int num_work_for_each = input_size / num_procs;

    //BUFFER THAT HOLD A SUBSET OF THE INPUT FOR EACH PROCESS 
    int* work_arr = (int*)malloc(sizeof(int)*num_work_for_each);
    
    //THE MASTER DIVIDES THE WORK BETWEEN THE PROCESSES
    MPI_Scatter(arr,num_work_for_each,MPI_INT,work_arr,num_work_for_each,MPI_INT,ROOT,MPI_COMM_WORLD); //each process got totalNums/numProcresses numbers to check
    
    //COMPUTE THE PRIMES OF EACH SUBSET
    for (int i = 0; i < num_work_for_each; i++)
    {
        work_arr[i]=checkPrime(work_arr[i]);
    }
    
    //GATHER ALL PARTIAL PRIMES DOWN TO THE ROOT PROCESS ----> ARRAY OF 1'S AND 0'S: 1 MEANS THE NUMBER IS PRIME
    int* prime_or_not;
   
    if(my_rank == ROOT)
    {
        prime_or_not = (int*)malloc(sizeof(int)*input_size);
    }

    MPI_Gather(work_arr, num_work_for_each, MPI_INT, prime_or_not, num_work_for_each, MPI_INT, ROOT, MPI_COMM_WORLD); //STUCK HERE, WHY???
    

    if(my_rank == ROOT)
    {
        printf("here are all prime numbers in this list\n");
        for (int i = 0; i < input_size; i++)
        {
            if(prime_or_not[i] == 1)
            {
                printf("%d\n", arr[i]);
            }
        }
    }

    //FREE ALL MEMORY AND MPI AND FINISH THE PROG
    if(my_rank == ROOT)
    {
        free(arr);
        free(prime_or_not);
    }  
    free(work_arr);
    MPI_Finalize();
    return 0;
}