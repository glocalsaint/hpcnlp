CC=mpicxx
CFLAGS=-std=c++11 
BFLAGS=-lboost_system -lboost_filesystem  
 
all: clean wsd.o utility.o fileprocessing.o firstlevel.o secondlevel.o mpi.o
	$(CC) mpi.o wsd.o fileprocessing.o firstlevel.o secondlevel.o utility.o  $(CFLAGS) $(BFLAGS) -lz -l sqlite3 -o hash
mpi.o:
	$(CC) -c hashselection.cpp $(CFLAGS) -o mpi.o
utility.o:
	$(CC) -c utility.cpp $(CFLAGS) $(BFLAGS) -l sqlite3 -o utility.o
fileprocessing.o:
	$(CC) -c fileprocessing.cpp $(CFLAGS) -lz  -o fileprocessing.o
firstlevel.o:
	$(CC) -c firstlevel.cpp $(CFLAGS) -o firstlevel.o
secondlevel.o:
	$(CC) -c secondlevel.cpp $(CFLAGS) -l sqlite3 -o secondlevel.o
wsd.o:
	$(CC) -c wsd.cpp $(CFLAGS) -lz -l sqlite3 -o wsd.o  
run: all
	mpirun -np 64 hash > o.out      
clean:
	rm -f wsd.o mpi.o graph.o firstlevel.o secondlevel.o fileprocessing.o utility.o a.out hash
