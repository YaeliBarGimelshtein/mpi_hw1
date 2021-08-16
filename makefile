build:
	mpicc -c dynamic_prime.c
	mpicc -o exec dynamic_prime.o  

clean:
	rm -f *.o exec

run:
	mpiexec -n 3 ./exec 