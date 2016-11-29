 #include "utility.hpp"
#include "mpi.h"
#include "graph.hpp"
#include <unordered_set>
#include "config.hpp"
using namespace std;
using namespace config;
//extern sqlite3 *db;

void create_open_db(int &myrank)
{
   char *zErrMsg = 0;
   int  rc;
   string sql;

   /* Open database */
   string dbname = "score" + to_string(myrank)+".db";
   rc = sqlite3_open(dbname.c_str(), &db);
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return;
   }else{
      //fprintf(stdout, "Opened database successfully\n");
   }

   /* Create SQL statement */
   sql = "CREATE TABLE if not exists scoreroothub("  \
         "TWORD TEXT      NOT NULL," \
         "CWORD TEXT      NOT NULL," \
         "SCORE            REAL     NOT NULL," \
         "ROOTHUBNUM        INT NOT NULL);" ;
         

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      //fprintf(stdout, "Table created successfully\n");
   }
   //sqlite3_close(db);
   return ;
}

void close_db()
{
    sqlite3_close(db);
}

void process_secondlevel(int &myrank , int &size, double &processingtime){
    clock_t sltime = clock();
   
    processingtime = 0;
    int round =0;
    
    vector<string> twords(localmap.size());
    for(auto &entry:localmap)
        twords.push_back(entry.first);

    std::random_shuffle ( twords.begin(), twords.end() );

    clock_t graphclock = clock();
    double graphtime = 0;

    auto it = twords.begin();
    double vm, rss;
    process_mem_usage(vm, rss);
    if(WRITE_OUTPUT_TO_FILE == 1)
        outputstream << "P:"<< myrank << " before SL VM: " << vm << "; RSS: " << rss << endl<<endl;
    else   
        cout << "P:"<< myrank << " before SL VM: " << vm << "; RSS: " << rss << endl<<endl;
    
        
    //create_open_db(myrank);
    //sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    while(1)
    {
        clock_t starttime = clock();

        //if ( myrank == 0 )
         if(WRITE_OUTPUT_TO_FILE == 1)
            outputstream << "P:" << myrank << ". Round Number: " << round++ << endl;
        else   
            cout <<"P:" << myrank << ". Round Number: " << round++ << endl;
        
         

        /////////////way to know if all can exit the loop; all have processed SL/////////
        int amidone = 0 , alldone = 0 ;
        if(it == twords.end()) amidone = 1;
        MPI_Allreduce(&amidone,    &alldone,    1,    MPI_INT,    MPI_SUM,    MPI_COMM_WORLD);
        if( alldone == size ) break;
        /////////////way to know if all can exit the loop; all have processed SL/////////

        int noof_twords = 0;
        int noof_flwords = 0;
        std::map < string , vector<std::pair<string, int>> > tword_flwords_map;
        std::unordered_map<string, int> fl_frequency_map;
        while( noof_flwords < NUMOF_FLWORDS )
        {
            if ( it == twords.end() ) break;
            string tword = *it;
            if(tword.back()!='N'){it++; continue;}
            auto &v = tword_flwords_map[tword];
            for( auto &entry : localmap[tword] ){
                v.push_back(std::make_pair(entry.first, entry.second));
            }
            noof_flwords += localmap[tword].size();
            noof_twords++;
            it++;
        }
        // for(int l=0;l<size;l++){
        // int max=0, arr[70];
        // int chunksize = localmap.size()/70;
        // auto iter = localmap.begin();
        // for(int i=0;i<70;i++)
        // {
        //     int temp=0;
        //     for(int j=0;j<chunksize;j++)
        //     {
        //         temp+=iter->second.size();
        //         if(iter==localmap.end())break;
        //         iter++;
        //     }
        //     arr[i]=temp;if(temp>max)max=temp;
        // }
        // int divideby = max/10;
        // for(int i=0;i<70;i++)
        //     arr[i]=(arr[i]*10)/max;
        // for(int i=10;i>0;i--)
        // {
        //     for(int j=0;j<70;j++)
        //     {
        //         if(arr[j]==i){cout<<"*";arr[j]--;}
        //         else cout<<"_"; 
        //     }
        //     cout<<endl;
        // }
        // MPI::COMM_WORLD.Barrier();
        // }
        int vsize=0;
        for(auto &entry : tword_flwords_map)
        {
            vsize+=entry.second.size();
        }
        //if(myrank == 0) 
        if(WRITE_OUTPUT_TO_FILE == 1)
            outputstream << "P:" << myrank <<" .Sending till" << std::distance(twords.begin(), it) <<"/"<<twords.size()<<" flwords size: " << vsize<<endl;
        else   
            cout << "P:" << myrank <<" .Sending till" << std::distance(twords.begin(), it) <<"/"<<twords.size()<<" flwords size: " << vsize<<endl;
        
        //cout << "P:" << myrank <<" .Sending " << std::distance(twords.begin(), it) <<"/"<<twords.size()<<" flwords size: " << vsize<<endl;
        MPI::COMM_WORLD.Barrier();
        int sendsize = 4 * noof_twords + STRING_LENGTH * noof_flwords;


        char *flwords_sendbuffer = new char[sendsize];
        int tempsize = 0 , sizeofint = sizeof(int);
        char *memptr = flwords_sendbuffer;
        for( auto &entry : tword_flwords_map)
        {
            tempsize = (entry.second).size();
            memcpy(memptr, &tempsize , sizeofint);
            memptr += sizeofint; 
            //if(myrank == 0) cout << "Process: 0 no of flwords" << entry.second.size() << endl;
            for( auto &flword : entry.second)
            {
                memcpy( memptr , flword.first.c_str() , flword.first.size()+1);
                memptr += STRING_LENGTH;
            }
        }
        int all_sendsizes[size], all_noof_twords[size];

        MPI_Allgather(&sendsize , 1 , MPI_INT , all_sendsizes , 1 , MPI_INT , MPI_COMM_WORLD);
        MPI_Allgather(&noof_twords , 1 , MPI_INT , all_noof_twords , 1 , MPI_INT , MPI_COMM_WORLD);
        
        int *recv_edgedata_gather = nullptr;
        char *recv_frequency_gather = nullptr;

        int freqencysizegather=0;
        for(int current_process = 0 ; current_process < size ; current_process++)
        {
            char *flwords_recvbuffer = new char[1];
            if(all_sendsizes[current_process] == 0) continue;
            if(myrank != current_process)
            {
                flwords_recvbuffer = new char[all_sendsizes[current_process]];
            }
            else
            {
                flwords_recvbuffer = flwords_sendbuffer;
            }
            //cout << all_sendsizes[current_process] << " " ;
            MPI::COMM_WORLD.Barrier();
            MPI_Bcast(flwords_recvbuffer , all_sendsizes[current_process] , MPI_CHAR , current_process , MPI_COMM_WORLD);
            //if(myrank == current_process) delete [] flwords_recvbuffer;
            int edgessize = 0;
            vector<std::map<int, std::vector<int>>> tosend_edges(all_noof_twords[current_process]);
            int *edgedata= nullptr;
            std::unordered_map<string, int> fmap;
            //if(myrank != current_process)
            //{
                std::vector<std::vector<string>> vecofvecs(all_noof_twords[current_process]);
                memptr = flwords_recvbuffer;
                for(int tword_index = 0 ; tword_index < all_noof_twords[current_process] ; tword_index++)
                {
                    int count=0;
                    memcpy(&count ,memptr , sizeof(int));
                    //if(myrank==0 && current_process==0)cout << count << ";";
                    memptr += sizeofint;
                    for( int fl_index = 0 ; fl_index < count ; ++fl_index)
                    {
                        char flword[STRING_LENGTH];
                        memcpy(flword, memptr, STRING_LENGTH);
                        memptr += STRING_LENGTH;
                        vecofvecs[tword_index].push_back(string(flword));
                    }
                }
                delete [] flwords_recvbuffer;
                int index = 0;
                
                for(auto &vecentry : vecofvecs)
                {
                    auto &currentmap = tosend_edges[index++];
                    int fromstr = 0;
                    for(auto &flword : vecentry)
                    {
                        if(std::hash<string>()(flword) % size == myrank && localmap.find(flword) != localmap.end())
                        {
                            fl_frequency_map[flword] = frequencymap[flword];
                            int tostr = 0;
                            auto &submap = localmap[flword];
                            for(auto &flword_second : vecentry)
                            {
                                if(submap.find(flword_second) != submap.end())
                                {
                                    currentmap[fromstr].push_back(tostr); 
                                    currentmap[fromstr].push_back(submap[flword_second]);//cooccurance count
                                }
                                tostr++;
                            }
                        }
                        fromstr++;
                    }
                }

                edgessize += tosend_edges.size();
                for(auto &entrymap : tosend_edges)
                {   
                    edgessize += entrymap.size() * 2;
                    for(auto &edgeentry: entrymap)
                        edgessize += edgeentry.second.size();
                }

                edgedata = new int[edgessize];
                int edgedataindex=0;
                int k=0;
                for(auto &entrymap : tosend_edges)
                {   
                    auto &v = vecofvecs[k++];

                    edgedata[edgedataindex++] = entrymap.size();
                    for(auto &edgeentry: entrymap)
                    {
                        edgedata[edgedataindex++] = edgeentry.first;
                        edgedata[edgedataindex++] = (edgeentry.second).size();
                        for(auto &edge : edgeentry.second)
                            edgedata[edgedataindex++] = edge;    
                    }
                    
                }   
                for(auto &entry : vecofvecs) entry.clear();
                vecofvecs.clear();                    
            //}   

            int recv_frequency_sizes[size];

            int fdata_size = (STRING_LENGTH + sizeof(int)) * fl_frequency_map.size(); 
            MPI_Gather( &fdata_size , 1 , MPI_INT , recv_frequency_sizes , 1 , MPI_INT , current_process , MPI_COMM_WORLD);
            char *tosendfrequency_data = new char[fdata_size];
            memptr = tosendfrequency_data;
            for(auto &fentry : fl_frequency_map)
            {
                memcpy(memptr, fentry.first.c_str(), fentry.first.size()+1);
                memptr += STRING_LENGTH;
                int freqency = fentry.second;
                memcpy(memptr, &freqency, sizeof(int));
                memptr += sizeof(int);
            }

            int  recvdisplacements[size];
            
            if(myrank == current_process)
            {
                recvdisplacements[0] = 0;
                for( int j = 1 ; j < size ; j++ )
                {
                    recvdisplacements[j] = recvdisplacements[j-1] + recv_frequency_sizes[j-1];
                }
                freqencysizegather = recvdisplacements[size-1] + recv_frequency_sizes[size-1];
                recv_frequency_gather = new char[ freqencysizegather ];                
            }   
            MPI_Gatherv( tosendfrequency_data , fdata_size , MPI_CHAR , recv_frequency_gather , recv_frequency_sizes, recvdisplacements, MPI_CHAR , current_process ,  MPI_COMM_WORLD);
            delete [] tosendfrequency_data;
            int recv_edge_sizes[size];
            MPI_Gather( &edgessize , 1 , MPI_INT , recv_edge_sizes , 1 , MPI_INT , current_process , MPI_COMM_WORLD);
                     
            
            int totalgathersize;
            if(myrank == current_process)
            {
                recvdisplacements[0] = 0;
                for( int j = 1 ; j < size ; j++ )
                {
                    recvdisplacements[j] = recvdisplacements[j-1] + recv_edge_sizes[j-1];
                }
                totalgathersize = recvdisplacements[size-1] + recv_edge_sizes[size-1];

                // #if WRITE_TO_OUTPUT_FILE == 1
                //     outputstream << "P:"<< myrank << " Total edge size : " << (double)totalgathersize/(1024*1024*1024)<<endl;
                // else   
                //     cout<< "P:"<< myrank << " Total edge size : " << (double)totalgathersize/(1024*1024*1024)<<endl;
                // 
                //cout<< "P:"<< myrank << " Total edge size : " << (double)totalgathersize/(1024*1024*1024)<<endl;
                recv_edgedata_gather = new int[ totalgathersize ];                
            }   
            MPI_Gatherv( edgedata , edgessize , MPI_INT , recv_edgedata_gather , recv_edge_sizes, recvdisplacements, MPI_INT , current_process ,  MPI_COMM_WORLD);

            //Virtual Memory
            // double vm, rss;
            // process_mem_usage(vm, rss);
            // if(myrank == current_process)
            // {                
            //     #if WRITE_TO_OUTPUT_FILE == 1
            //         outputstream << "P:"<< myrank << " SL 1 loop VM: " << vm << "; RSS: " << rss << endl<<endl;
            //     else   
            //         cout << "P:"<< myrank << " SL 1 loop VM: " << vm << "; RSS: " << rss << endl<<endl;
            //                     
            // }

            delete [] edgedata;

        }//End for loop

        //cout <<"P:" << myrank << ". out of size loop. "  << endl;
        int dataindex = 0;
        char *freqmemptr = recv_frequency_gather;
        freqencysizegather /= STRING_LENGTH + sizeof(int);
        
        while(freqencysizegather--)
        {
            char flword[STRING_LENGTH];
            memcpy(flword, freqmemptr, STRING_LENGTH);
            freqmemptr += STRING_LENGTH;
            int frequency;
            memcpy(&frequency, freqmemptr, sizeof(int));
            freqmemptr += sizeof(int);
            fl_frequency_map[flword] = frequency;            
        }
                
        map<string,  map<int, std::vector<int> >> mapedgelists;
        //if(myrank ==0)
        for(int l = 0; l < size ; l++)
        {
            int index = 0;
            for(auto &twordentry: tword_flwords_map)                
            {     
                //cout << twordentry.first<<" ";           

                int noofedgelists = recv_edgedata_gather[dataindex++];
                auto & edgelists = mapedgelists[twordentry.first];
                for(int i = 0 ; i < noofedgelists ; i++)
                {
                    int fromstr = recv_edgedata_gather[dataindex++];
                    int edgescount = recv_edgedata_gather[dataindex++];
                    for(int j = 0 ; j < edgescount ; j++)
                    {
                        int edge = recv_edgedata_gather[dataindex++];
                        edgelists[fromstr].push_back(edge);
                        // int ccount = recv_edgedata_gather[dataindex++];
                        // edgelists[fromstr].push_back(ccount);
                    }
                }
            }
        }

        delete[] recv_frequency_gather;
        delete[] recv_edgedata_gather;
        //cout <<"Process: " << myrank << ". Edges gathered. "  << endl;
        // int l=0;;
        // //if(myrank ==0){auto it = tword_flwords_map.begin();auto v = it -> second; cout << v[0]<<endl;}
        // for(auto &twordentry: tword_flwords_map)
        // {

        //     if(mapedgelists[twordentry.first].size() > twordentry.second.size())
        //     {
        //         cout <<" Dude super dead " << mapedgelists[twordentry.first].size() <<"  "<< twordentry.second.size() << endl;
        //         for(auto &entry : mapedgelists[twordentry.first])
        //         {
        //             string str="";
        //             for(auto ventry : entry.second) {str += to_string(ventry) + " ";}
        //             cout << entry.first <<" :: " << str<<endl;
        //         }
        //     }
        // }
        int index = 0;
        graphclock = clock();
        // std::ofstream fs;
        // fs.open (outputfiles_location+"/wsi_process"+to_string(myrank)+".txt", std::ofstream::out | std::ofstream::app);

        
        for(auto &twordentry: tword_flwords_map)
        {                
            clock_t singlegraphclock = clock();
            auto &tword = twordentry.first;
            //if(tword.compare("priceN")!=0)continue;
            auto &flwords = twordentry.second;
            //cout << "P:"<<myrank << " tword:" << tword << ":"<<(clock()-graphtime)/(double) CLOCKS_PER_SEC<<endl;
            Graph *g = new Graph(tword);
            g->create_graphwithedgelists(flwords , mapedgelists[tword] , fl_frequency_map, myrank);
            mapedgelists[tword].clear();
            flwords.clear();
            mapedgelists.erase(tword);
            //cout << "P:"<<myrank << " donegraph:" <<tword <<":"<<(clock()-graphtime)/(double) CLOCKS_PER_SEC<<endl;
            std::vector<string> roothubs;
            g->get_roothubs(roothubs);
            //cout << "P:"<<myrank << " doneroothubs:" <<tword <<":"<<(clock()-graphtime)/(double) CLOCKS_PER_SEC<<endl;
            double vm, rss;
            
            g->performMST();
            //cout << "P:"<<myrank << "doneMST:" <<tword <<":"<<(clock()-graphtime)/(double) CLOCKS_PER_SEC<<endl;
            if(WRITE_OUTPUT_TO_FILE == 1)
                outputstream << " T: "<<(clock()-singlegraphclock)/(double) CLOCKS_PER_SEC<<endl;
            else   
                cout << " T: "<<(clock()-singlegraphclock)/(double) CLOCKS_PER_SEC<<endl;
        
            stringtographmap[tword] = g;    
            
            ///////////////////////////////////
            //g->roothubs.clear();        
             g->nodelist.clear();
            for(auto &entry : g->stringtonode_map)
            {
                auto nodeptr = entry.second;
                delete nodeptr;
            }
            g->stringtonode_map.clear();
            ///////////////////////////////////

            // string s = tword + "  ";
            // for(auto &entry: roothubs)
            // {
            //     //cout << entry.first<< " :: ";
            //     s+= entry + "  ";
            //     // for(auto &root: entry.second)
            //     //     cout << root << " ";
                
            // }
            // fs << s << "\n";
            // cout <<s <<endl;
            // if(flwords.size() != mapedgelists[tword].size()) 
            //     cout << "Difference: " << flwords.size() << " - " << mapedgelists[tword].size() << " = " <<flwords.size() - mapedgelists[tword].size()<<endl;
        }
        
         if(ALLOW_TIME_LOGGING == 1)
            graphtime += (clock()-graphclock)/(double) CLOCKS_PER_SEC;
        
        process_mem_usage(vm, rss);
        if(WRITE_OUTPUT_TO_FILE == 1)
            outputstream << "P:"<< myrank << " VM: " << vm << "; RSS: " << rss << endl<<endl;
        else   
            cout << "P:"<< myrank << " VM: " << vm << "; RSS: " << rss << endl<<endl;
        
        
        //fs.close();
        //if( myrank == size-1 )
       
        if(WRITE_OUTPUT_TO_FILE == 1)
            outputstream <<endl<<"Time taken for graph" << graphtime<< " SL " << (clock()-sltime)/(double) CLOCKS_PER_SEC << "\n";
        else   
            cout <<endl<<"Time taken for graph" << graphtime<< " SL " << (clock()-sltime)/(double) CLOCKS_PER_SEC << "\n";
            
        cout<<"P: "<<myrank<<" round:"<<round-1<<" Time taken for graph" << graphtime<< " SL " << (clock()-sltime)/(double) CLOCKS_PER_SEC << "\n";
        fl_frequency_map.clear();
        
        for(auto &entry : tword_flwords_map)
        {
            entry.second.clear();
        }
        tword_flwords_map.clear();
        if (ALLOW_TIME_LOGGING==1)
            processingtime += (clock()-starttime)/(double) CLOCKS_PER_SEC;
    }//End while loop    
    MPI::COMM_WORLD.Barrier();   

    
    if (ALLOW_TIME_LOGGING==1)
     if(WRITE_OUTPUT_TO_FILE == 1)
            outputstream << endl << "Time taken for graph" << graphtime<< " SL " << (clock()-sltime)/(double) CLOCKS_PER_SEC << "\n";
        else   
            cout << endl << "Time taken for graph" << graphtime<< " SL " << (clock()-sltime)/(double) CLOCKS_PER_SEC << "\n";
    
    if (ALLOW_TIME_LOGGING==1)
    if(WRITE_OUTPUT_TO_FILE == 1)
        outputstream <<endl<<"P:" << myrank <<" Individual time taken for SL: "<< processingtime << "\n";
    else   
        cout <<endl<<"P:" << myrank <<" Individual time taken for SL: "<< processingtime << "\n";


    
    for(auto &entry : localmap )
    {
        entry.second.clear();        
    }
    localmap.clear();
    frequencymap.clear();
    //sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    //close_db();
    //if( myrank == size-1 )cout<<endl<<"Time taken for SL2 " << (clock()-sltime)/(double) CLOCKS_PER_SEC << "\n";        

}
