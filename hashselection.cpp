/*///////////Includes and defines/////////////*/
    #include <iostream> 
    #include "mpi.h"
    #include <string>
    #include <stdlib.h>
    #include <algorithm>
    #include <unordered_map>
    #include <boost/tokenizer.hpp>
    #include <set>
    #include <boost/filesystem.hpp>
    #include <boost/filesystem/operations.hpp>
    #include <boost/range/iterator_range.hpp>
    #include <iostream>
    #include <fstream>
    #include <map>
    #include <ctime>
    #include <ctype.h>
    #include <future>
    #include <thread>
    #include <fstream>
    #include <iomanip>
    #include <functional>
    #include "graph.hpp"

    //#include "graph.hpp"
    //#define CHUNKSIZE 100000
    #define ROUNDROBINSIZE 500
    #define STRING_LENGTH 30
    #define NUMWORDS 100
    using namespace std;
/*///////////Includes and defines/////////////*/

/*///////////////Static Declarations/////////////*/
    typedef std::unordered_map<string, std::unordered_map<string, int>> map_stringtostringint;

    unsigned long mapsize(const std::unordered_map<string,std::unordered_map<string,int>> &map){
        unsigned long mapelements = map.size();
        unsigned long submapelements =0;
        for(std::unordered_map<string,std::unordered_map<string,int>>::const_iterator it = map.begin(); it != map.end(); ++it){
            submapelements += (it->second).size();        
        }
        
        unsigned long mapsize = (mapelements+submapelements) *(STRING_LENGTH+4);
        //mapsize+=mapelements*STRING_LENGTH;
        //mapsize+=submapelements*92;
        mapsize = mapsize/(1024*1024);
        return mapsize;
    }

     std::vector<string> getallfilenames(boost::filesystem::path p)
    {
        std::vector<string> filepaths;
        boost::filesystem::directory_iterator end_ptr;      
        boost::filesystem::directory_iterator dir(p);   
        for (;dir != end_ptr; dir++) {
            p = boost::filesystem::path(*dir);
            if(is_directory(p))
            {
                getallfilenames(p);
            }
            else
            {
                    string dirpath(dir->path().parent_path().string() );
                    string filename(p.stem().string());
                    filepaths.push_back(dir->path().string());
            }
        }
        return filepaths;
    }
/*///////////////Static Declarations/////////////*/

/*///////////////Insert to Local Map Function/////////////////*/         
     void insert_to_localmap(std::set<string> &stringset, std::unordered_map<string,std::unordered_map<string,int>> &localmap)
     {
        
        std::set<string>::iterator it;
        std::set<string>::iterator iter;
        int j=0;
        it=stringset.begin();
        int myrank = MPI::COMM_WORLD.Get_rank(); 
        //processes local map that does not have the string entry
        for(int current_index=0; it!=stringset.end(); it++,current_index++,j=0)
        {
            string itstr(*it);
            if(localmap.find(itstr)==localmap.end())
            {
                std::unordered_map<string,int> newmap;
                for(j=0,iter = stringset.begin();iter != stringset.end();++iter)
                {
                    string iterstr(*iter);
                    //add all other words in the sentence as cooccuring words except for the current word.
                    //current_index==j refers the word which we are dealing with now.
                    if(current_index==j){j++; continue;}

                    newmap[iterstr]=1;
                    j++;
                }
                localmap[itstr]=newmap;
            }
            //processes local map that has the string entry
            else
            {        
                
                std::unordered_map<string,int> &stringmap = localmap[*it];
                for(j=0,iter = stringset.begin();iter != stringset.end();++iter)
                {
                    string iterstr(*iter);
                    //skip the current word
                    if(current_index==j) {j++;continue;}
                    if(stringmap.find(iterstr)==stringmap.end())
                    {
                        stringmap[iterstr]=1;
                    }
                    else
                    {
                        int stringcount = stringmap[iterstr]+1;
                        stringmap[iterstr] = stringcount;                    
                    }
                    j++;
                }
            }
        }
     }
/*///////////////Insert to Local Map Function/////////////////*/         


/*///////////////Process String Function/////////////////*/              
     void process_string(string &str, std::unordered_map<string,std::unordered_map<string,int>> &localmap, std::unordered_map<string, int> &frequencymap)
     {
            
            int myrank = MPI::COMM_WORLD.Get_rank(); 
            int current_index=0, index=0;
            int length = str.length();
            
            std::set<string> uniquewords;
            int k=0;
            while(index<length)
            {
                index = str.find('\n',index+1);
                if(current_index+1 == index) continue;
                if(index==string::npos) break;
                string line = str.substr(current_index,index - current_index);
                boost::char_separator<char> sep("\t ");
                boost::tokenizer<boost::char_separator<char>> tokens(line, sep);
                int i=0; string two,three;
                for (const auto& t : tokens) {
                    if(i==2) {two.assign(t); }//if(isdigit(two.at(0)))break;}
                    if(i==3)
                    {
                        if(t.compare("NN")==0 || t.compare("ADJ")==0)
                        {
                            string uniqueword(two+":"+t);
                            if(uniqueword.length()>STRING_LENGTH-2)break;
                            uniquewords.insert(uniqueword);
                            if(frequencymap.find(uniqueword)==frequencymap.end())
                            {
                                frequencymap[uniqueword]=1;
                            }
                            else
                            {
                                int stringcount = frequencymap[uniqueword]+1;
                                frequencymap[uniqueword] = stringcount;
                            }
                        }
                    }
                    i++;
                    if(i>3) break;
                }
                if((str[index+1]=='\n' ||index==str.size()-1) && !uniquewords.empty())
                {   
                    insert_to_localmap(uniquewords,localmap);  
                    uniquewords.clear();                              
                }
                current_index=index;
            }
     }
/*///////////////Process String Function/////////////////*/              

int main(int argc, char *argv[]) 
{ 

    MPI::Init(argc, argv); 
/*//////Datatype for Sending words*/
    MPI_Datatype MPI_Customword;
    MPI_Datatype type0[1] = { MPI_CHAR };
    int blocklen0[1] = { STRING_LENGTH};
    MPI_Aint disp0[1];
 
    disp0[0] = 0;
    
    MPI_Type_create_struct(1, blocklen0, disp0, type0, &MPI_Customword);
    MPI_Type_commit(&MPI_Customword);
/*//////Datatype for Sending words*/

/*//////Declarations//////*/
    MPI::Status status; 
    int myrank = MPI::COMM_WORLD.Get_rank(); 
    int size = MPI::COMM_WORLD.Get_size(); 

    std::vector<string> files=getallfilenames("/work/scratch/vv52zasu/inputfiles/");
    //std::vector<string> files=getallfilenames("/home/vv52zasu/mpi/inputfiles/");
    std::unordered_map<string,int> frequencymap;
    std::unordered_map<string,std::unordered_map<string, int>> localmap;
    std::unordered_map<string,std::unordered_map<string, int>> localsecondlevelmap;
    int bufsize, *buf;
    string bufchar1;
    char filename[128]; 
/*//////Declarations//////*/    

/*//////Read files in a loop and write initial data to localmap/////*/
    std::clock_t fileprocessing_timestart = clock();
    for(std::vector<string>::iterator it = files.begin(); it != files.end(); ++it)
    {
        if(myrank ==0)
            std::cout<<"Processing file:"<<(*it).c_str()<<endl;

        MPI::File thefile = MPI::File::Open(MPI::COMM_WORLD, (*it).c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL); 
        MPI::Offset filesize = thefile.Get_size();
       
        char *bufchar;  
        int CHUNKSIZE = (filesize/size)+1;
        CHUNKSIZE = std::max(CHUNKSIZE, 10000);
        bufchar =  new char[CHUNKSIZE+2]; 

        MPI_Status status1;
        int i=0;

        MPI_File_seek(thefile, (myrank)*CHUNKSIZE, MPI_SEEK_SET);
        MPI_File_read( thefile, bufchar, CHUNKSIZE, MPI_CHAR, &status1);
        int count=0;
        MPI_Get_count( &status1, MPI_CHAR, &count );
        string str(bufchar,bufchar+count);
        
        MPI::COMM_WORLD.Barrier();
        
        int occurrences = 0;
        string::size_type start = 0;
        
        int from =0, to=0,index=0;
        string tosend(str);
        while(1)
        {
            index=index+to+1;
            to = tosend.find("\n\n");      
            if(to==string::npos) break;        
            tosend=tosend.substr(to+1);
        }
        string trimstr=str.substr(0,index-1);        
        
        str="";
        MPI::COMM_WORLD.Barrier();

        int length = tosend.length();
        char *recvptr;
        recvptr = new char[CHUNKSIZE];
        
        int dest=0,src=0;
        if(myrank==size-1)
        {
            dest=0;
            src=myrank-1;
        }
        else if(myrank==0)
        {
            dest=1;src=size-1;
        }
        else 
        {
            dest=myrank+1;src=myrank-1;
        }
        
        MPI::COMM_WORLD.Sendrecv(tosend.c_str(), length, MPI_CHAR, dest, 123, recvptr, CHUNKSIZE, MPI_CHAR, src, 123, status);
        

        string finalstr="";
        for (int i=0; i<size; i++)
        {
            if (i == myrank) {
                string recvstr;
                if(myrank !=0)
                {        
                    recvstr.assign(recvptr,recvptr+status.Get_count(MPI_CHAR)); 
                    finalstr = recvstr+trimstr; 
                }
                else finalstr=trimstr;
            }
            MPI::COMM_WORLD.Barrier();
        }
        
        delete(recvptr);
        str="";
        trimstr="";
        tosend="";

        process_string(finalstr, localmap, frequencymap);
        if(myrank ==0)
            std::cout<<"Processing file Ended: "<<(*it).c_str()<<endl;
    }
    MPI::COMM_WORLD.Barrier();
    if(myrank==0) cout<<" Time taken to process the files: "<< (clock()-fileprocessing_timestart)/(double) CLOCKS_PER_SEC<<"\n";
/*//////Read files in a loop and write initial data to localmap/////*/    
/*////////////Sending the strings to hash%size process//////////*/
    vector<vector<string>> vecgrouped(size, vector<string>(0));

    for(auto &entry: localmap){
        vecgrouped[std::hash<string>()(entry.first)%size].push_back(entry.first);    
    }

    int locsize = localmap.size();
    for(auto &entry: localmap)
        locsize += entry.second.size();


    cout << "Process: " << myrank << " Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;

    char* recvdata;
    int sizeofint = sizeof(int);
    int recvtwords_count=0;
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

    // for(auto &entry : localmap)
    //     for(auto pair : entry.second)
    //         cout << pair.first <<":";

    
    
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
    cout << "Process: " << myrank << " Local map size: "<< localmap.size() <<" "<< ". Actual data size: "<<mapsize(localmap)<< endl;
    // for(auto &entry : localmap)
    // {
    //     cout << entry.first << " :: " ;
    //     for(auto pair : entry.second)
    //     {
    //        cout << pair.first <<": ";
    //     }
    //     cout<< endl;
    // }
/*//////////broadcasting flwords and getting the slwords in other processes////////*/
    
    auto it = localmap.begin();
    int l=0;
    int itcount=-1 * NUMWORDS;
    while( 1 )
    {
        itcount+=NUMWORDS;
        l++;
        if(myrank == 0) cout << "Process: " << myrank << " itcount: " << itcount <<". Distance: "<< std::distance(localmap.begin(), it) <<endl;
        int sendwords_sl = 1;
        if ( it == localmap.end() ) sendwords_sl = 0;
        int allsendwords_sl[ size ], allsendwordssum;
        MPI_Allgather( &sendwords_sl , 1 , MPI_INT , allsendwords_sl , 1 , MPI_INT , MPI_COMM_WORLD);
        MPI_Allreduce(&sendwords_sl,    &allsendwordssum,    1,    MPI_INT,    MPI_SUM,    MPI_COMM_WORLD);
        if ( allsendwordssum == 0 ) break;
        
        std::set<string> fl_words;
        std::vector<string> target_words;
        for( int i = 0 ; i < NUMWORDS ; i++ )
        {
            if ( it == localmap.end() ) break;
            target_words.push_back(it -> first);
            for( auto entry : it -> second ){
                fl_words.insert( entry.first );
            }
            
            it++;
        }

        vector< vector< string > > vecgrouped_sl( size , vector< string >(0));
        for(auto entry: fl_words){
            vecgrouped_sl[std::hash<string>()(entry)%size].push_back(entry);    
        }
        // if(myrank == 0){
        // cout << "Process: " << myrank << " : " << vecgrouped_sl[0].size()  << " : " << vecgrouped_sl[1].size() <<endl;
        // for(auto &entry : vecgrouped_sl[0])
        //     cout << entry << " : ";
        // cout << "-- " <<endl;
        // }
        int recvsize=0;
        char *recvdata_scatter=nullptr;
        char *recvdata_gather=nullptr;
        char *senddata=nullptr;
        vecgrouped_sl[myrank].clear();
        for( int i = 0 ; i < size ; i++ )
        {
            //if( allsendwords_sl[i] == 0 ) continue;
            
            int sendcounts[size] ={} , senddisplacements[size]={} ,recvcounts[size]={} , recvdisplacements[size]={} ;
            int sendcount =0 ,  recvcount=0 , totalsendsize=0;
/*////////////////////////SCATTERING///////////////////////////*/
            if( myrank == i )
            {
                sendcounts[0] = vecgrouped_sl[0].size() * STRING_LENGTH;
                senddisplacements[0] = 0;
                for( int j = 1 ; j < size ; j++ )
                {
                    sendcounts[j] = vecgrouped_sl[j].size() * STRING_LENGTH;
                    senddisplacements[j] = senddisplacements[j-1] + sendcounts[j-1];                    
                }
                totalsendsize = senddisplacements[size-1] + sendcounts[size-1];

                senddata = new char[ totalsendsize  ];
                char *memptr = senddata;
                for( auto &v_entry : vecgrouped_sl)
                {
                    for( auto str : v_entry )
                    {
                        memcpy( memptr , str.c_str() , str.size() + 1);
                        memptr += STRING_LENGTH;        
                    }
                }
            }
            //if(myrank == i) cout << "Process: " << myrank << " :totalsendsize: " << totalsendsize <<endl;
            MPI_Scatter( sendcounts , 1 , MPI_INT , &recvcount , 1 , MPI_INT , i , MPI_COMM_WORLD );   
            recvdata_scatter = new char[ recvcount ];
            
            MPI_Scatterv( senddata , sendcounts , senddisplacements , MPI_CHAR , recvdata_scatter , recvcount , MPI_CHAR , i , MPI_COMM_WORLD );

/*////////////////////////SCATTERING///////////////////////////*/
/*////////////////////////GATHERING///////////////////////////*/
            int tosendwordcount=0;
            if ( myrank == i ) delete[] senddata;
            char *memptr = recvdata_scatter;
            int sendwords_sl;
            std::vector<string> fl_words;
            for ( int j = 0 ; j < recvcount / STRING_LENGTH ; ++j )
            {
                char fl_word[STRING_LENGTH];
                memcpy( fl_word , recvdata_scatter , STRING_LENGTH);
                recvdata_scatter += STRING_LENGTH;
                fl_words.push_back(fl_word);
            }

            delete [] memptr;
            int sendsize = 0;
            
            for( auto entry : fl_words)
            {
                if(localmap.find(entry)!=localmap.end()){
                    tosendwordcount++;
                    sendsize += localmap[entry].size();
                }
            }
            //sendcount = (sendsize + fl_words.size()) * (STRING_LENGTH + sizeofint) + (fl_words.size() * sizeofint);
            sendcount = (sendsize + tosendwordcount) * (STRING_LENGTH + sizeofint) + (tosendwordcount * sizeofint);
            senddata = new char [ sendcount ];
            memptr = senddata;
            for( auto entry : fl_words)
            {
                if(localmap.find(entry) == localmap.end()) continue;
                memcpy( memptr , entry.c_str() , entry.size() + 1);
                memptr += STRING_LENGTH;
                
                int N = frequencymap[entry];        
                memcpy( memptr , &N , sizeof(int) );
                memptr += sizeofint;

                N = localmap[entry].size();
                memcpy( memptr , &N , sizeof(int) );
                memptr += sizeofint;
                
                 for( auto &slword_entry : localmap[entry] )
                 {
                    memcpy( memptr , slword_entry.first.c_str() , slword_entry.first.size() + 1);            
                    memptr += STRING_LENGTH;    

                    const int N = slword_entry.second;
                    memcpy( memptr , &N , sizeofint );
                    memptr += sizeofint;    
                 }
            }
            MPI_Reduce(&tosendwordcount ,    &recvtwords_count,    1,    MPI_INT,    MPI_SUM,  i,    MPI_COMM_WORLD);                
            MPI_Gather( &sendcount , 1 , MPI_INT , recvcounts , 1 , MPI_INT , i , MPI_COMM_WORLD);
                        
            if(myrank == i)
            {
                recvdisplacements[0] = 0;
                for( int j = 1 ; j < size ; j++ )
                {
                    recvdisplacements[j] = recvdisplacements[j-1] + recvcounts[j-1];
                }
                recvsize = recvdisplacements[size-1] + recvcounts[size-1];
                recvdata_gather = new char [ recvsize ];
            }   
                
            MPI_Gatherv( senddata , sendcount , MPI_CHAR , recvdata_gather , recvcounts, recvdisplacements, MPI_CHAR , i ,  MPI_COMM_WORLD);
            delete [] senddata;
/*////////////////////////GATHERING///////////////////////////*/            
        }
        char *recvdata_head = recvdata_gather;
/*///////////////////////Each process processes the words and adds into its localmap//////////*/     
            //cout << "Process: " << myrank << "checking before " << localsecondlevelmap.size() <<endl;
        for ( int i = 0 ; i < recvtwords_count -1  ; i++)
        {
            // char tword[STRING_LENGTH];
            // memcpy( tword , recvdata_gather , STRING_LENGTH);
            // recvdata_gather += STRING_LENGTH;
            // int frequency;
            // memcpy( &frequency , recvdata_gather , sizeof(int) );
            // recvdata_gather += sizeofint; 

            // frequencymap[tword] += frequency;       

            // int cwords_size;
            // memcpy( &cwords_size , recvdata_gather , sizeof(int) );
            // recvdata_gather += sizeofint;
            
            // auto &submap = localsecondlevelmap[tword];

            // for( int i=0 ; i < cwords_size ; i++ )
            // {
            //     char cword[STRING_LENGTH];
            //     memcpy( cword , recvdata_gather , STRING_LENGTH);            
            //     recvdata_gather += STRING_LENGTH;
                
            //     int ccount;
            //     memcpy( &ccount , recvdata_gather , sizeofint );
            //     recvdata_gather += sizeofint;
                
            //     submap[cword] += ccount;                
            // }
            //if(recvdata_gather < recvdata_head + recvsize) break;
        }
            //cout << "Process: " << myrank << "checking after " << localsecondlevelmap.size() << ". recvtwordscount is: " << recvtwords_count << endl;
        delete [] recvdata_head;
/*///////////////////////Each process processes the words and adds into its localmap//////////*/        
        for( auto &v: vecgrouped_sl)
            v.clear();
        fl_words.clear();
        cout << "Process: " << myrank << " itcount Memory occupied: " << mapsize(localmap) + mapsize(localsecondlevelmap) << ". localsecondlevelmap size: "<<localsecondlevelmap.size()<< endl;
        // for(auto tword : target_words)
        // {
        //     auto submap = localmap[tword];
        //     localsecondlevelmap[tword] = submap;
        //     Graph *graph = new Graph(tword);
        //     graph -> create_graph(localsecondlevelmap, frequencymap);
        //     delete graph;

        // }
        for(auto &entry : localsecondlevelmap)        
            entry.second.clear();
        localsecondlevelmap.clear();
            
    }


    MPI::COMM_WORLD.Barrier();

    if ( myrank == 0 )
        cout << "Process: " << myrank << " writing to file started.." << "\n";
     //----------------------------------------------------------------------------------------------------------
    ////writing to file
    //----------------------------------------------------------------------------------------------------------

    /*std::fstream fs;
    string filenamefs ="file"+to_string(myrank)+".txt";
    fs.open (filenamefs.c_str(), std::fstream::in | std::fstream::out | std::fstream::app);

    char *line = new char[20];
    int x=0;
    int totallength=0;
    int maxlocalmapsize=0;
    int localmap_size = localmap.size();
    MPI_Allreduce(&localmap_size,    &maxlocalmapsize,    1,    MPI_INT,    MPI_MAX,    MPI_COMM_WORLD);
     
    long long sizeofstr=0;
    int counter=0,counter1=0;
    it = localmap.begin();
    char *localstr= new char[20];
    int length=0;
    int length1=0;
    for(it = localmap.begin();it!=localmap.end();it++)
     {  
        if((it->first).substr( (it->first).length() - 2 ) == "NN")  
        {   
            string rootentry ="";//= it->first +"    ";
             auto &submap = it->second;

            string secondlevelstring="(";
            for(auto& secondlevelentry : submap)
            {
                //if(secondlevelentry.second>=6)
                    secondlevelstring+=secondlevelentry.first+"::"+std::to_string(secondlevelentry.second)+",";
            }                        
            secondlevelstring.replace(secondlevelstring.end()-1,secondlevelstring.end(),")\n"); 
            rootentry+=it->first +"    "+it->first+"::"+ std::to_string(frequencymap[it->first]) + secondlevelstring;
                        
            for(auto &firstlevelentry : submap)
            {
                //if(firstlevelentry.second>=6)
                {
                    //cout<<firstlevelentry.second<<"came"<<endl;
                    
                    if(localmap.find(firstlevelentry.first)!=localmap.end())
                    {   
                        auto &secondlevelsubmap = localmap[firstlevelentry.first];
                        string secondlevelstring="(";
                        
                        for(auto& secondlevelentry : secondlevelsubmap)
                        {
                            //if(secondlevelentry.second>=5)
                                secondlevelstring+=secondlevelentry.first+"::"+std::to_string(secondlevelentry.second)+",";
                        }     
                        secondlevelstring.replace(secondlevelstring.end()-1,secondlevelstring.end(),")\n"); 
                        rootentry+=it->first +"    "+firstlevelentry.first+"::"+ std::to_string(frequencymap[firstlevelentry.first]) + secondlevelstring;
                       
                    }
                    else if(localsecondlevelmap.find(firstlevelentry.first)!=localsecondlevelmap.end())
                    {
                        auto &secondlevelsubmap = localsecondlevelmap[firstlevelentry.first];
                        string secondlevelstring="(";
                        for(auto& secondlevelentry : secondlevelsubmap)
                        {
                            //if(secondlevelentry.second>=5)
                                secondlevelstring+=secondlevelentry.first+"::"+std::to_string(secondlevelentry.second)+",";
                        }
                        secondlevelstring.replace(secondlevelstring.end()-1,secondlevelstring.end(),")\n");
                        rootentry+=it->first +"    "+firstlevelentry.first+"::"+std::to_string(frequencymap[firstlevelentry.first]) +secondlevelstring;
                        
                    }
                    // else
                    //     if((it->first).compare("absence::NN")==0)counter1++;
                }
            }
            length = rootentry.length();
            free(localstr);
            
            localstr = new char[length+1];
            strcpy(localstr, rootentry.c_str());
            if(myrank==0)   
            counter++;
            fs<<localstr;
        }
    }
    
    fs.close();

    if(myrank==0)cout<<"writing to file done..size="<<sizeofstr<<"\n";
*/        
    MPI::Finalize(); 
    return 0; 
} 
