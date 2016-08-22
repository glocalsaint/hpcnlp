#include "src/utility.hpp"
#include "mpi.h"
void process_firstlevel(int &myrank , int &size){

    char* recvdata;
    int sizeofint = sizeof(int);
    int recvtwords_count=0;

    clock_t hashingtime = clock();
    vector<vector<string>> vecgrouped(size, vector<string>(0));

    for(auto &entry: localmap){
        vecgrouped[std::hash<string>()(entry.first)%size].push_back(entry.first);    
    }

    for( int i=0 ; i < size ; i++ )
    {
        //if ( myrank ==0 ) 
            //cout << "Process: " << myrank << " Entered the loop. Round: "<< i << endl;
        auto &tosend_twords = vecgrouped[i];
        int data_size = 0;
        for( auto& entry: tosend_twords ) 
        {
            data_size+=localmap[entry].size();
        }
        //Allocate memory
        int allocsize = (tosend_twords.size() + data_size) * (STRING_LENGTH + sizeofint) + ( tosend_twords.size() * sizeofint );
        char* memptr_head = new char[ allocsize ];
        char* memptr = memptr_head;
        
        //Add the data to the allocated memory
        const int N = tosend_twords.size();
        int torecv=0;
        MPI_Reduce(&N ,    &recvtwords_count,    1,    MPI_INT,    MPI_SUM,  i,    MPI_COMM_WORLD);
        // if ( myrank == i ) 
        //     cout << "Process: " << myrank << " receive size: "<< recvtwords_count << endl;
        //memcpy( memptr , &N , sizeof(int));
        //memptr += sizeofint;
        
        for( auto &entry : tosend_twords )
        {
            memcpy( memptr , entry.c_str() , entry.size() + 1);
            memptr += STRING_LENGTH;
            
            int N = frequencymap[entry];        
            memcpy( memptr , &N , sizeof(int) );
            memptr += sizeofint;

            N = localmap[entry].size();
            memcpy( memptr , &N , sizeof(int) );
            memptr += sizeofint;
            
            for( auto &entry_cword : localmap[entry] )
            {
                memcpy( memptr , entry_cword.first.c_str() , entry_cword.first.size() + 1);            
                memptr += STRING_LENGTH;
                
                const int N = entry_cword.second;
                memcpy( memptr , &N , sizeofint );
                memptr += sizeofint;
            }
        }
        //After copying data in the ptr, the corresponding data in localmap can be deleted
        for( auto& entry: tosend_twords ) 
        {
            localmap.erase(entry);
        }
        tosend_twords.clear();
        
        int *allocsizes= new int[size];
        MPI_Gather(&allocsize,  1,  MPI_INT,  allocsizes,  1,  MPI_INT,  i,  MPI_COMM_WORLD);
        

        
        int *allocdisplacements = new int[size];
        allocdisplacements[0] = 0;
        for( int i=0 ; i < size ; i++ )
        {
            allocdisplacements[i]=allocdisplacements[i-1]+allocsizes[i-1];
        }
        int totalrecvsize = allocdisplacements[size-1]+ allocsizes[size-1];
        
        if ( myrank == i )
            recvdata = new char[totalrecvsize];
        
        MPI_Gatherv(memptr_head, allocsize, MPI_CHAR, recvdata,  allocsizes, allocdisplacements, MPI_CHAR , i ,  MPI_COMM_WORLD);
        
        delete[](memptr_head);
        // if ( myrank == 0 ) 
        //     cout << "Process: " << myrank << "Finished round: " << i << "\n";
    }
    
    localmap.clear();
    frequencymap.clear();
    //copy the received data into localmap
    char *recvdata_head = recvdata;
    for( int i=0 ; i < recvtwords_count ; i++ )
    {
        char tword[STRING_LENGTH];
        memcpy( tword , recvdata , STRING_LENGTH);
        recvdata += STRING_LENGTH;
        
        int frequency;
        memcpy( &frequency , recvdata , sizeof(int) );
        recvdata += sizeofint; 

        frequencymap[tword] += frequency;       

        int cwords_size;
        memcpy( &cwords_size , recvdata , sizeof(int) );
        recvdata += sizeofint;
        
        auto &submap = localmap[tword];
        for( int i=0 ; i < cwords_size ; i++ )
        {
            char cword[STRING_LENGTH];
            memcpy( cword , recvdata , STRING_LENGTH);            
            recvdata += STRING_LENGTH;
            
            int ccount;
            memcpy( &ccount , recvdata , sizeofint );
            recvdata += sizeofint;
            
            submap[cword] += ccount;
        }
    }

    delete[] recvdata_head;
    if ( myrank == 0 ) 
        cout << "Process: " << myrank << ". Hashing done. " << "\n";
    if(myrank==0) cout<<" Time taken for hashing FL words: "<< (clock()-hashingtime)/(double) CLOCKS_PER_SEC<<"\n";
}