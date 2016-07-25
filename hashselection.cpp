#include "src/utility.hpp"
#include "src/mpi_datatypes.hpp"
#include "mpi.h"   

using namespace std;

std::unordered_map<string,int> frequencymap;
std::unordered_map<string,std::unordered_map<string, int>> localmap;
std::unordered_map<string,std::unordered_map<string, int>> localsecondlevelmap;
std::set<string> mystringset;


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

    clock_t fileprocessing_timestart = clock();
    process_files();
    
    if(myrank==0) cout<<" Time taken to process the files: "<< (clock()-fileprocessing_timestart)/(double) CLOCKS_PER_SEC<<"\n";
    
    if(myrank==0) cout << "Process: " << myrank << " Before FL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;

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
    
    if(myrank==0) cout << "Process: " << myrank << " Before SL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;
    
    // for(auto &entry: localmap)
    // {
    //     auto &submap = entry.second;
    //     for(auto it=submap.begin();it!=submap.end();)
    //     {
    //         if(it->second<5)it = submap.erase(it);
    //         else it++;
    //     }
    // }    
    // for(auto it = localmap.begin();it!=localmap.end();)
    // {
    //     if((it->second).size()<5) it = localmap.erase(it);
    //     else it++;
    // }
    

    process_secondlevel(myrank, size);

    if(myrank==0) cout << "Process: " << myrank << " After SL Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;


    // //if ( myrank == 0 ) cout << "Process: " << myrank << " writing to file started.." << "\n";
          
    MPI::Finalize(); 
    return 0; 
} 
