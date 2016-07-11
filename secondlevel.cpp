 #include "src/utility.hpp"
#include "mpi.h"
void process_secondlevel(int &myrank , int &size){


    auto it = localmap.begin();
    std::set<string> fl_wordset;
    int round =0;
    while(1)
    {
        if ( myrank == 0 ) cout <<"Process: " << myrank << ". Round Number: " << round++ << endl;
        /////////////way to know if all can exit the loop; all have processed SL/////////
        int amidone = 0 , alldone = 0 ;
        if(it == localmap.end()) amidone = 1;
        MPI_Allreduce(&amidone,    &alldone,    1,    MPI_INT,    MPI_SUM,    MPI_COMM_WORLD);
        if( alldone == size ) break;
        /////////////way to know if all can exit the loop; all have processed SL/////////

        int nooftwords=0;
        while( fl_wordset.size() < 50000 )
        {
            if ( it == localmap.end() ) break;
            for( auto entry : it->second ){
                fl_wordset.insert( entry.first );
            }
            nooftwords++;
            it++;
        }
        cout << "Process: " << myrank << " No. of twords sending: " << nooftwords << endl;
        vector<string> fl_vector(fl_wordset.size());
        vector<string> fl_recvvector;
        vector<std::pair<int,int>> sendedgesvector;
        std::set<string> fl_recv_wordset;
        int sendsize = fl_wordset.size() * STRING_LENGTH;
        int all_sendsizes[size];

        MPI_Allgather(&sendsize , 1 , MPI_INT , all_sendsizes , 1 , MPI_INT , MPI_COMM_WORLD);
        //if (myrank == 0) cout << "Process: " << myrank << ""
        //Everyone will broadcast their set of Firstlevel words
        for ( int current_process = 0 ; current_process < size ; current_process++ )
        {
            //if ( myrank == 0 ) cout <<"Process: " << myrank << ". Round Number: " << round++ << ". Processing no.: " << current_process << ". Came1 "<<endl;
            char *bcast_buffer = new char[ all_sendsizes[current_process] ];
            char *memptr = bcast_buffer;
            if( myrank == current_process )
            {
                for(auto &str : fl_wordset)
                {
                    fl_vector.push_back( str );
                    memcpy( memptr , str.c_str() , str.size() + 1);
                    memptr += STRING_LENGTH;        
                }    
                fl_wordset.clear();
            }            
            
            //if ( myrank == 0 ) cout <<"Process: " << myrank << ". Round Number: " << round++ << ". Processing no.: " << current_process << ". Came2 "<<endl;
            MPI_Bcast(bcast_buffer , all_sendsizes[current_process] , MPI_CHAR , current_process , MPI_COMM_WORLD);
            
            if ( myrank != current_process )
            {
                fl_recvvector.resize( all_sendsizes[current_process]/STRING_LENGTH );
                while(memptr < bcast_buffer + all_sendsizes[current_process] )
                {
                    char fl_word[STRING_LENGTH];
                    memcpy( fl_word , memptr , STRING_LENGTH);
                    //fl_recvvector.push_back(fl_word);
                    fl_recv_wordset.insert(fl_word);
                    memptr += STRING_LENGTH;
                }
                if(myrank ==0) cout << "Process: " << myrank << " . number of flwords received: " << fl_recvvector.size() << endl;
                //if ( myrank == 10 ) cout <<"Process: " << myrank << ". Round Number: " << round++ << ". Processing no.: " << current_process << ". Came3 "<<endl;
                int current_index = 0 , word_index = 0;
                //for( current_index = 0 ; current_index < fl_recvvector.size() ; current_index++ )
                //while( current_index < fl_recv_wordset.size() )
                clock_t startsearchtime = clock();
                for(auto &current_string : fl_recv_wordset)
                {
                    //string current_string = fl_recvvector[current_index];
                    //string current_string = fl_recv_wordset[current_index];
                    if ( (std::hash<string>()(current_string )) % size  == myrank )
                    {
                        //if(myrank ==10) cout << "Process: " << myrank << " . came into hash " << endl;
                        for( auto &entry : localmap[current_string])
                        {                            
                            auto it = fl_recv_wordset.find(entry.first);
                            if(it != fl_recv_wordset.end())
                                sendedgesvector.push_back(std::make_pair(current_index,  std::distance(fl_recv_wordset.begin(), it) ));
                        }
                    }
                    current_index++;
                }
                if( myrank == size-1 )cout<<"Time taken for search edges " << (clock()-startsearchtime)/(double) CLOCKS_PER_SEC << "\n";        
            }
            //if ( myrank == 0 ) cout <<"Process: " << myrank << ". Round Number: " << round++ << ". Processing no.: " << current_process << ". Came4 "<<endl;
            fl_recvvector.clear();
            int numberofedges = sendedgesvector.size() * 2 * 4;
            int recvedgecounts[size];
            MPI_Gather( &numberofedges , 1 , MPI_INT , recvedgecounts , 1 , MPI_INT , current_process , MPI_COMM_WORLD);

            int  recvdisplacements[size];         
            char *recvdata_gather= new char[1];
            if(myrank == current_process)
            {
                recvdisplacements[0] = 0;
                for( int j = 1 ; j < size ; j++ )
                {
                    recvdisplacements[j] = recvdisplacements[j-1] + recvedgecounts[j-1];
                }
                recvdata_gather = new char[recvdisplacements[size-1] + recvedgecounts[size-1] ];                
                cout << "Process: " << myrank << " . number of edges: " << recvdisplacements[size-1] + recvedgecounts[size-1]<<endl;
            }   
            char *senddata = new char[1];
            if(myrank != current_process)
            {    
                senddata = new char[sendedgesvector.size() * 2 * 4];
                memptr = senddata;
                for( auto &pair : sendedgesvector)
                {
                    int first = pair.first;
                    int second = pair.second;
                    memcpy(memptr , &first , sizeof(int));
                    memptr+=sizeof(int);
                    memcpy(memptr , &second , sizeof(int));
                    memptr+=sizeof(int);
                }
            }
            MPI_Gatherv( senddata , sendedgesvector.size() * 2 * 4 , MPI_CHAR , recvdata_gather , recvedgecounts, recvdisplacements, MPI_CHAR , current_process ,  MPI_COMM_WORLD);
            delete[] senddata;
            delete[] recvdata_gather;
            sendedgesvector.clear();
            fl_recvvector.clear();

        }  

    }




    // auto it = localmap.begin();
    // //auto it = mystringset.begin();
    // int l=0;
    // int itcount=-1 * NUMWORDS;
    // while( 1 )
    // {
    //     itcount+=NUMWORDS;
    //     l++;
    //     if(myrank == 0) cout << "Process: " << myrank << " itcount: " << itcount <<". Distance: "<< std::distance(localmap.begin(), it) <<endl;
    //     clock_t roundtime = clock();
    //     int sendwords_sl = 1;
    //     if ( it == localmap.end() ) sendwords_sl = 0;
    //     //if ( it == mystringset.end() ) sendwords_sl = 0;
    //     int allsendwords_sl[ size ], allsendwordssum;
    //     MPI_Allgather( &sendwords_sl , 1 , MPI_INT , allsendwords_sl , 1 , MPI_INT , MPI_COMM_WORLD);
    //     MPI_Allreduce(&sendwords_sl,    &allsendwordssum,    1,    MPI_INT,    MPI_SUM,    MPI_COMM_WORLD);
    //     if ( allsendwordssum == 0 ) break;
        
    //     std::set<string> fl_words;
    //     std::vector<string> target_words;
    //     int loopcounter =0;
    //     while( fl_words.size() < 5000 )
    //     {
    //         if ( it == localmap.end() ) break;
    //         //if ( it == mystringset.end() ) break;
    //         target_words.push_back(it -> first);
    //         //target_words.push_back(*it);
    //         for( auto entry : it->second ){
    //             fl_words.insert( entry.first );
    //         }
    //         loopcounter++;
    //         it++;
    //     }
    //     cout << "Process: "<< myrank << ". FL Words Count: " << fl_words.size() << "of target_words count: " << loopcounter << endl;
    // }
//         vector< vector< string > > vecgrouped_sl( size , vector< string >(0));
//         for(auto entry: fl_words){
//             vecgrouped_sl[std::hash<string>()(entry)%size].push_back(entry);    
//         }
//         // if(myrank == 0){
//         // cout << "Process: " << myrank << " : " << vecgrouped_sl[0].size()  << " : " << vecgrouped_sl[1].size() <<endl;
//         // for(auto &entry : vecgrouped_sl[0])
//         //     cout << entry << " : ";
//         // cout << "-- " <<endl;
//         // }
//         int recvsize=0;
//         char *recvdata_scatter=new char[1];
//         char *recvdata_gather=new char[1];
//         char *senddata=new char[1];
//         vecgrouped_sl[myrank].clear();
//         for( int i = 0 ; i < size ; i++ )
//         {
//             //if( allsendwords_sl[i] == 0 ) continue;
            
//             int sendcounts[size] ={} , senddisplacements[size]={} ,recvcounts[size]={} , recvdisplacements[size]={} ;
//             int sendcount =0 ,  recvcount=0 , totalsendsize=0;
// /*////////////////////////SCATTERING///////////////////////////*/
//             if( myrank == i )
//             {
//                 sendcounts[0] = vecgrouped_sl[0].size() * STRING_LENGTH;
//                 senddisplacements[0] = 0;
//                 for( int j = 1 ; j < size ; j++ )
//                 {
//                     sendcounts[j] = vecgrouped_sl[j].size() * STRING_LENGTH;
//                     senddisplacements[j] = senddisplacements[j-1] + sendcounts[j-1];                    
//                 }
//                 totalsendsize = senddisplacements[size-1] + sendcounts[size-1];
//                 //cout <<"Process: " << myrank << ". Totalsendsize:"<<totalsendsize<<endl;
//                 senddata = new char[ totalsendsize  ];
//                 char *memptr = senddata;
//                 for( auto &v_entry : vecgrouped_sl)
//                 {
//                     for( auto str : v_entry )
//                     {
//                         memcpy( memptr , str.c_str() , str.size() + 1);
//                         memptr += STRING_LENGTH;        
//                     }
//                 }
//             }
//             //if(myrank == i) cout << "Process: " << myrank << " :totalsendsize: " << totalsendsize <<endl;
//             MPI_Scatter( sendcounts , 1 , MPI_INT , &recvcount , 1 , MPI_INT , i , MPI_COMM_WORLD );   
//             recvdata_scatter = new char[ recvcount ];
            
//             MPI_Scatterv( senddata , sendcounts , senddisplacements , MPI_CHAR , recvdata_scatter , recvcount , MPI_CHAR , i , MPI_COMM_WORLD );

// /*////////////////////////SCATTERING///////////////////////////*/
// /*////////////////////////GATHERING///////////////////////////*/
//             int tosendwordcount=0;
//             if ( myrank == i ) delete[] senddata;
//             char *memptr = recvdata_scatter;
//             int sendwords_sl;
//             std::vector<string> fl_words;
//             for ( int j = 0 ; j < recvcount / STRING_LENGTH ; j++ )
//             {
//                 char fl_word[STRING_LENGTH];
//                 memcpy( fl_word , recvdata_scatter , STRING_LENGTH);
//                 recvdata_scatter += STRING_LENGTH;
//                 fl_words.push_back(fl_word);
//             }

//             delete [] memptr;
//             int sendsize = 0;
            
//             for( auto entry : fl_words)
//             {
//                 if(localmap.find(entry)!=localmap.end()){
//                     tosendwordcount++;
//                     sendsize += localmap[entry].size();
//                 }
//             }
//             //sendcount = (sendsize + fl_words.size()) * (STRING_LENGTH + sizeofint) + (fl_words.size() * sizeofint);
//             sendcount = (sendsize + tosendwordcount) * (STRING_LENGTH + sizeofint) + (tosendwordcount * sizeofint);
//             senddata = new char [ sendcount ];
//             memptr = senddata;
//             for( auto entry : fl_words)
//             {
//                 if(localmap.find(entry) == localmap.end()) continue;
//                 memcpy( memptr , entry.c_str() , entry.size() + 1);
//                 memptr += STRING_LENGTH;
                
//                 int N = frequencymap[entry];        
//                 memcpy( memptr , &N , sizeof(int) );
//                 memptr += sizeofint;

//                 N = localmap[entry].size();
//                 memcpy( memptr , &N , sizeof(int) );
//                 memptr += sizeofint;
                
//                  for( auto &slword_entry : localmap[entry] )
//                  {
//                     memcpy( memptr , slword_entry.first.c_str() , slword_entry.first.size() + 1);            
//                     memptr += STRING_LENGTH;    

//                     const int N = slword_entry.second;
//                     memcpy( memptr , &N , sizeofint );
//                     memptr += sizeofint;    
//                  }
//             }
//              MPI_Reduce(&tosendwordcount ,    &recvtwords_count,    1,    MPI_INT,    MPI_SUM,  i,    MPI_COMM_WORLD);                
//              MPI_Gather( &sendcount , 1 , MPI_INT , recvcounts , 1 , MPI_INT , i , MPI_COMM_WORLD);
                        
//             if(myrank == i)
//             {
//                 recvdisplacements[0] = 0;
//                 for( int j = 1 ; j < size ; j++ )
//                 {
//                     recvdisplacements[j] = recvdisplacements[j-1] + recvcounts[j-1];
//                 }
//                 recvsize = recvdisplacements[size-1] + recvcounts[size-1];
//                 recvdata_gather = new char [ recvsize ];
//             }   
                
//             MPI_Gatherv( senddata , sendcount , MPI_CHAR , recvdata_gather , recvcounts, recvdisplacements, MPI_CHAR , i ,  MPI_COMM_WORLD);
//             delete [] senddata;
// /*////////////////////////GATHERING///////////////////////////*/    
//     if(myrank==0 && myrank ==i) cout<<" Time taken for hashing SL words round "<<l<<": "<< (clock()-hashingtime)/(double) CLOCKS_PER_SEC<<". Recv size: "<<recvsize<< "\n";        
//         }
//         char *recvdata_head = recvdata_gather;
// /*///////////////////////Each process processes the words and adds into its localmap//////////*/     
//             //cout << "Process: " << myrank << "checking before " << localsecondlevelmap.size() <<endl;
//         for ( int i = 0 ; i < recvtwords_count -1  ; i++)
//         {
//             // char tword[STRING_LENGTH];
//             // memcpy( tword , recvdata_gather , STRING_LENGTH);
//             // recvdata_gather += STRING_LENGTH;
//             // int frequency;
//             // memcpy( &frequency , recvdata_gather , sizeof(int) );
//             // recvdata_gather += sizeofint; 

//             // frequencymap[tword] += frequency;       

//             // int cwords_size;
//             // memcpy( &cwords_size , recvdata_gather , sizeof(int) );
//             // recvdata_gather += sizeofint;
            
//             // auto &submap = localsecondlevelmap[tword];

//             // for( int i=0 ; i < cwords_size ; i++ )
//             // {
//             //     char cword[STRING_LENGTH];
//             //     memcpy( cword , recvdata_gather , STRING_LENGTH);            
//             //     recvdata_gather += STRING_LENGTH;
                
//             //     int ccount;
//             //     memcpy( &ccount , recvdata_gather , sizeofint );
//             //     recvdata_gather += sizeofint;
                
//             //     submap[cword] += ccount;                
//             // }
//             //if(recvdata_gather < recvdata_head + recvsize) break;
//         }
//             //cout << "Process: " << myrank << "checking after " << localsecondlevelmap.size() << ". recvtwordscount is: " << recvtwords_count << endl;
//         delete [] recvdata_head;
// /*///////////////////////Each process processes the words and adds into its localmap//////////*/        
//         for( auto &v: vecgrouped_sl)
//             v.clear();
//         fl_words.clear();
//         //cout << "Process: " << myrank << " itcount Memory occupied: " << mapsize(localmap) + mapsize(localsecondlevelmap) << ". localsecondlevelmap size: "<<localsecondlevelmap.size()<< endl;
//         // for(auto tword : target_words)
//         // {
//         //     auto submap = localmap[tword];
//         //     localsecondlevelmap[tword] = submap;
//         //     Graph *graph = new Graph(tword);
//         //     graph -> create_graph(localsecondlevelmap, frequencymap);
//         //     delete graph;

//         // }
//         for(auto &entry : localsecondlevelmap)        
//             entry.second.clear();
//         localsecondlevelmap.clear();
            
//     }


    MPI::COMM_WORLD.Barrier();

}
