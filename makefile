build:
	mpicc -c static_prime.c
	mpicc -o exec static_prime.o  

clean:
	rm -f *.o exec

run:
	mpiexec -n 2 ./exec 