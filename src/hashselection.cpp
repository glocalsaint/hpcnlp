#include "utility.hpp"
//#include "mpi_datatypes.hpp"
#include "mpi.h"   
//#include "graph.hpp"
#include "config.hpp"

using namespace std;
using namespace config;
std::unordered_map<string,int> frequencymap;
std::unordered_map<string,std::unordered_map<string, int>> localmap;
std::unordered_map<string,std::unordered_map<string, int>> localsecondlevelmap;
std::vector<std::tuple<unsigned char*, long unsigned int, long unsigned int>> compressedvector;
std::unordered_map<string, Graph*> stringtographmap;
std::set<string> mystringset;
std::ofstream outputstream;
sqlite3 *db;

int main(int argc, char *argv[]) 
{ 
    
    MPI::Init(argc, argv); 
    MPI::Status status; 
/*//////Datatype for Sending words*/
    // MPI_Datatype MPI_Customword;
    // MPI_Datatype type0[1] = { MPI_CHAR };
    // int blocklen0[1] = { STRING_LENGTH};
    // MPI_Aint disp0[1];
 
    // disp0[0] = 0;
    
    // MPI_Type_create_struct(1, blocklen0, disp0, type0, &MPI_Customword);
    // MPI_Type_commit(&MPI_Customword);
/*//////Datatype for Sending words*/
/*//////Declarations//////*/
    
    int myrank = MPI::COMM_WORLD.Get_rank(); 
    int size = MPI::COMM_WORLD.Get_size(); 


    if(WRITE_OUTPUT_TO_FILE == 1)
        outputstream.open (outputfiles_location+"/out"+to_string(myrank)+".txt", std::ofstream::out | std::ofstream::app);

    clock_t fileprocessing_timestart;    
    if(ALLOW_TIME_LOGGING == 1)
        fileprocessing_timestart = clock();
    
    double processingtime=0;
    process_files(processingtime);
    if( ALLOW_TIME_LOGGING == 1)
        if(WRITE_OUTPUT_TO_FILE == 1)
                outputstream << "Process: " << myrank << " Individual Time taken to process the files: "<< processingtime<<"\n";
        else   
                cout<< "Process: " << myrank << " Individual Time taken to process the files: "<< processingtime<<"\n";
    
    
    
    MPI::COMM_WORLD.Barrier();
    if(myrank==0) 
    {
	if(WRITE_OUTPUT_TO_FILE == 1)
            outputstream << " Total Time taken to process the files: "<< (clock()-fileprocessing_timestart)/(double) CLOCKS_PER_SEC<<"\n";
	else	
	    cout<<" Total Time taken to process the files: "<< (clock()-fileprocessing_timestart)/(double) CLOCKS_PER_SEC<<"\n";
	
    }	
    
    if(myrank==0) 
    {
	if(WRITE_OUTPUT_TO_FILE == 1)
	    outputstream << "Process: " << myrank << " Before FL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;
	else	
	    cout << "Process: " << myrank << " Before FL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;
	
    }

    process_firstlevel(myrank, size);

    //removing FL words which are co-occuring less than 5 times.
    // for(auto it = frequencymap.begin(); it!= frequencymap.end();)
    // {
    //     if(it->second < 10){
    //         if(localmap.find(it->first)!=localmap.end())localmap.erase(it->first);
    //         it = frequencymap.erase(it);
    //     } 
    //     else it++;
    // }

    
    //words cooccuring less than MIN_FIRST_ORDER_COOCCURRENCE_COUNT times were eliminated
    for(auto &entry: localmap)
    {
        auto &submap = entry.second;
        for(auto it=submap.begin();it!=submap.end();)
        {
            if(it->second <= MIN_FIRST_ORDER_COOCCURRENCE_COUNT) it = submap.erase(it);
            else it++;
        }
    }     
    //Words with less than MIN_NUM_OCCURRENCES frequency in the entire subcorpus were also discarded. 
    for(auto it = frequencymap.begin();it!=frequencymap.end();)
    {
        if((it->second) < MIN_NUM_OCCURRENCES)
        {
            localmap.erase(it->first); 
            it = frequencymap.erase(it); 
        }
        else it++;
    }

    /*typedef struct{string flword; int cooccurance;} fl;
    std::unordered_map<string, fl*> localmapstruct;
    std::unordered_map<string, int> localmapsizes;
    for(auto &entry: localmap)
    {
        auto &submap = entry.second;
        fl *flptr = new fl[submap.size()];
        int index=0;
        for(auto it=submap.begin();it!=submap.end();it++)
        {
            flptr[index].flword=it->first;
            flptr[index++].cooccurance=it->second;            
        }
        localmapstruct[entry.first] = flptr;
        localmapsizes[entry.first] = submap.size();
    }
    
    for(int l=0;l<size;l++){
        int max=0, arr[150];
        int chunksize = localmapstruct.size()/150;
        auto iter = localmapstruct.begin();
        for(int i=0;i<150;i++)
        {
            int temp=0;
            for(int j=0;j<chunksize;j++)
            {
                //fl *flptr = iter->second;
                int size = localmapsizes[iter->first];
                temp+=size;
                if(iter==localmapstruct.end())break;
                iter++;
            }
            arr[i]=temp;if(temp>max)max=temp;
        }
        int divideby = max/10;
        for(int i=0;i<150;i++)
            arr[i]=(arr[i]*10)/max;
        for(int i=10;i>0;i--)
        {
            for(int j=0;j<150;j++)
            {
                if(arr[j]==i){cout<<"*";arr[j]--;}
                else cout<<"_"; 
            }
            cout<<endl;
        }
        MPI::COMM_WORLD.Barrier();
    }
    */
    if(myrank == 0)
    {
    if(WRITE_OUTPUT_TO_FILE == 1)
        outputstream << "Process: " << myrank << " Before SL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;
    else   
        cout << "Process: " << myrank << " Before SL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;
    }
    //if(myrank==0) cout << "Process: " << myrank << " Before SL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;
    processingtime = 0;
    process_secondlevel(myrank, size, processingtime);
    cout<< "Process: " << myrank << " Time taken for SL: "<< processingtime<<"\n";

    if(myrank == 0)
    {
    if(WRITE_OUTPUT_TO_FILE == 1)
        outputstream << "Process: " << myrank << " After SL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;
    else   
        cout << "Process: " << myrank << " After SL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;
    
    }
    //if(myrank==0) cout << "Process: " << myrank << " After SL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;

    clock_t disambiguation_timestart;
    if( ALLOW_TIME_LOGGING == 1)
        disambiguation_timestart = clock();
    
    
    disambiguate(myrank, size);

    if( ALLOW_TIME_LOGGING == 1)
        if (WRITE_OUTPUT_TO_FILE == 1)
            outputstream <<" Total Time taken to disambiguate: "<< (clock()-fileprocessing_timestart)/(double) CLOCKS_PER_SEC<<"\n";
        else   
            cout <<" Total Time taken to disambiguate: "<< (clock()-fileprocessing_timestart)/(double) CLOCKS_PER_SEC<<"\n";
    
          
    MPI::Finalize(); 
    if( WRITE_OUTPUT_TO_FILE == 1)
        outputstream.close();
    
    return 0; 
} 
