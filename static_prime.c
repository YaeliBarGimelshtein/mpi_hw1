#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define ROOT 0
#define MAX 50

int gcd(int a, int b)
{
    if (a == 0)
        return b;
    return gcd(b%a, a);
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
        getchar();
    
        arr = (int*)malloc(sizeof(int)*input_size*2); //ARRAY OF COUPLES
        char str[MAX];
        char check;

        for (int i = 0; i < input_size; i++)
        {
            printf("Enter the %d pair of numbers separated by a tab\n", i+1);
            fgets(str, MAX, stdin);
            if(sscanf(str, "%d\t%d%s", &arr[2*i], &arr[2*i+1], &check)!= 2 || check != '\0')
            {
                printf("illegal input at line %d\n", i+1);
                exit(0);
            }
        }
    }
    
    MPI_Bcast(&input_size, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    //SET THE TIME
    double t = MPI_Wtime();
    
    //CALCULATING FOR EACH PROCESS HOW MANY NUMBERS TO CHECK
    int num_work_for_each = input_size / num_procs;
    int extra_work = 0;
    if(input_size % num_procs != 0)
        extra_work = input_size % num_procs;
    
    //BUFFER THAT HOLD A SUBSET OF THE INPUT FOR EACH PROCESS 
    int* work_arr = (int*)malloc(sizeof(int)*num_work_for_each*2);
    int* res_arr = (int*)malloc(sizeof(int)*num_work_for_each);
    
    //THE MASTER DIVIDES THE WORK BETWEEN THE PROCESSES
    MPI_Scatter(arr,num_work_for_each*2,MPI_INT,work_arr,num_work_for_each*2,MPI_INT,ROOT,MPI_COMM_WORLD); 
    
    //COMPUTE THE PRIMES OF EACH SUBSET
    for (int i = 0; i < num_work_for_each; i++)
    {
        res_arr[i]=gcd(work_arr[2*i], work_arr[2*i + 1]);
    }

    //GATHER ALL PARTIAL DEVISIONS DOWN TO THE ROOT PROCESS
    int* final_gcd;
   
    if(my_rank == ROOT)
    {
        final_gcd = (int*)malloc(sizeof(int)*input_size);
    }

    MPI_Gather(res_arr, num_work_for_each, MPI_INT, final_gcd, num_work_for_each, MPI_INT, ROOT, MPI_COMM_WORLD); 

    //TAKE CARE OF THE RESIDUAL 
     if(my_rank == ROOT && extra_work != 0 )
    {
        for (int i = num_procs * num_work_for_each; i < input_size; i++)
        {
            final_gcd[i]=gcd(arr[2*i], arr[2*i + 1]);
        }
    }
    
    //PRINT THE RESULTS
    if(my_rank == ROOT)
    {
        printf("Time: %lf\n",MPI_Wtime()-t);
        printf("here are all the numbers and their gcd\n");
        for (int i = 0; i < input_size; i++)
        {
            printf("%d\t%d\t", arr[2*i], arr[2*i+1]);
            printf("gcd: %d\n", final_gcd[i]);
        }
    }

    //FREE ALL MEMORY AND MPI AND FINISH THE PROG
    if(my_rank == ROOT)
    {
        free(arr);
        free(final_gcd);
    } 
    
    free(work_arr);
    MPI_Finalize();
    return 0;
}