CC=mpic++
CFLAGS=-std=c++11 
BFLAGS=-lboost_system -lboost_filesystem  
 
all: clean graph.o utility.o fileprocessing.o firstlevel.o secondlevel.o mpi.o
	$(CC) mpi.o fileprocessing.o firstlevel.o secondlevel.o utility.o graph.o  $(CFLAGS) $(BFLAGS) -l sqlite3 -o a.out
mpi.o:
	$(CC) -c hashselection.cpp $(CFLAGS) -o mpi.o
utility.o:
	$(CC) -c utility.cpp $(CFLAGS) $(BFLAGS) -o utility.o
fileprocessing.o:
	$(CC) -c fileprocessing.cpp $(CFLAGS) -o fileprocessing.o
firstlevel.o:
	$(CC) -c firstlevel.cpp $(CFLAGS) -o firstlevel.o
secondlevel.o:
	$(CC) -c secondlevel.cpp $(CFLAGS) -o secondlevel.o
graph.o:
	g++ -c graph.cpp $(CFLAGS) -o graph.o  
run: all
	mpirun -np 64 a.out      
clean:
	rm -f mpi.o graph.o firstlevel.o secondlevel.o fileprocessing.o utility.o a.out
