CC=mpic++
CFLAGS=-std=c++11 -lboost_system -lboost_filesystem  

all: clean mpi.o graph.o
	$(CC) mpi.o graph.o $(CFLAGS) -l sqlite3 -o a.out
mpi.o:
	$(CC) -c hashselection.cpp $(CFLAGS) -l sqlite3 -o mpi.o
graph.o:
	g++ -c graph.cpp $(CFLAGS) -o graph.o  
run: all
	mpirun -np 20 a.out      
clean:
	rm -f mpi.o graph.o a.out
