#include "utility.hpp"
#include "mpi.h"
#include "zlib-1.2.8/zlib.h"
#include "config.hpp"
using namespace config;
void process_files(double &processingtime)
{
    std::vector<string> files=getallfilenames(inputfiles_location);
    //std::vector<string> files=getallfilenames("/home/vv52zasu/mpi/inputfiles/");
    MPI::Status status; 
    int myrank = MPI::COMM_WORLD.Get_rank();
    int size = MPI::COMM_WORLD.Get_size();
    int filecount=0;
    processingtime = 0;

    clock_t compress_clock = clock();
    double compress_time = 0;
    

/*//////Read files in a loop and write initial data to localmap/////*/
    long int totalcompressedsize = 0;
    for(std::vector<string>::iterator it = files.begin(); it != files.end(); ++it)
    {
        if(myrank ==0)
        {
            //#if ALLOW_TIME_LOGGING == 1
                if(WRITE_OUTPUT_TO_FILE == 1)
                    outputstream <<"Processing file:"<<(*it).c_str()<<endl;
                else   
                    cout <<"Processing file:"<<(*it).c_str()<<endl;
                
            //#endif
        }
        clock_t starttime;
        if( ALLOW_TIME_LOGGING == 1)
             starttime = clock();
        
        
        MPI::File thefile = MPI::File::Open(MPI::COMM_WORLD, (*it).c_str(), MPI::MODE_RDONLY, MPI::INFO_NULL);
        MPI::Offset filesize = thefile.Get_size();

        char *bufchar, *bufchar_header;
        int CHUNKSIZE = (filesize/size)+1;
        CHUNKSIZE = std::max(CHUNKSIZE, 10000);
        bufchar =  new char[CHUNKSIZE+300000];
        bufchar_header = bufchar;
        bufchar = bufchar + 300000;
        MPI_Status status1;

        MPI_File_seek(thefile, (myrank)*CHUNKSIZE, MPI_SEEK_SET);
        MPI_File_read( thefile, bufchar, CHUNKSIZE, MPI_CHAR, &status1);
        int count=0;
        MPI_Get_count( &status1, MPI_CHAR, &count );

        MPI::COMM_WORLD.Barrier();

        char * pch, *lastsentence=nullptr;
        pch=strchr(bufchar,'\n');
        while (pch!=NULL)
        {
            if(*(pch+1)=='\n'){ lastsentence = pch+2;}
            pch=strchr(pch+1,'\n');
        }

        int sendcharcount = count -( lastsentence - bufchar );
        if(sendcharcount < 0 || sendcharcount > 300000 || lastsentence == nullptr) sendcharcount =0;
        //cout << "CHUNKSIZE: "<< CHUNKSIZE << "count: " << count << " sendcharcount: " << sendcharcount << endl;
        //cout << lastsentence << endl;
        char *recvptr;
        recvptr = new char[300000];

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
        //if(sendcharcount >= 300000) cout <<"Process: " << myrank << " sendcharcount:" << sendcharcount <<endl<< lastsentence<<endl;;
        MPI_Sendrecv(lastsentence, sendcharcount, MPI_CHAR, dest, 123, recvptr, CHUNKSIZE, MPI_CHAR, src, 123, MPI_COMM_WORLD, &status1);
        //MPI::COMM_WORLD.Sendrecv(lastsentence, sendcharcount, MPI_CHAR, dest, 123, recvptr, CHUNKSIZE, MPI_CHAR, src, 123, status);
        int recvcount=0;
        MPI_Get_count( &status1, MPI_CHAR, &recvcount );
        //cout << "Process: " << myrank << ". Recvcount: " << recvcount << endl;

        //int recvcount = strlen(recvptr);
        
        bufchar = bufchar -recvcount;
        //if(recvcount >= 300000) cout << "Process: " << myrank << " exceeded size limit"<<endl;
        memcpy(bufchar, recvptr, recvcount);
        int finalcount = lastsentence - bufchar;
        if(lastsentence==nullptr) finalcount = recvcount+CHUNKSIZE;
        // cout << "Final count: " << finalcount << " total allocated: " << CHUNKSIZE<< " count + recvcount - sendcharcount: " << count+ recvcount -sendcharcount<< endl;
        if( finalcount > CHUNKSIZE+300000) {
            cout << "Process: " << myrank <<  " exceeded size limit"<< "Final Count: "<<finalcount<<endl;
            // if(myrank==32||myrank ==94){
            // for(int i=0;i<100; i++)
            //     cout<<bufchar[i];
            // cout <<endl;
            //  }
        }

        if( ALLOW_TIME_LOGGING == 1)
            compress_clock = clock();
        
        

        long unsigned int destsize = compressBound(finalcount);
        unsigned char *compressedstr = new unsigned char[destsize];
        int result = compress(compressedstr, &destsize, (unsigned char*)bufchar, finalcount);
        unsigned char *newcompressedstr = new unsigned char[destsize+1];
        memcpy(newcompressedstr, compressedstr, destsize);
        newcompressedstr[destsize]='\0';
        delete[] compressedstr;
        compressedvector.push_back(std::make_tuple(newcompressedstr, destsize, finalcount));
        totalcompressedsize+=destsize;
        if( ALLOW_TIME_LOGGING == 1)
            compress_time += (clock()-compress_clock)/(double) CLOCKS_PER_SEC;
        
        
        // auto front = compressedvector.front();

        // compressedstr = std::get<0>(front);


        // auto compressedlen = std::get<1>(front);
        // cout <<" Process: " << myrank << " compressedsize: " << compressedlen << endl;
        // auto originallen = std::get<2>(front);
        // unsigned char *uncompressbuf = new unsigned char[(int)originallen];
        // long unsigned int uncompresslength;
        // result = uncompress(uncompressbuf, &uncompresslength, compressedstr, compressedlen);
        // cout <<" Process: " << myrank << " uncompressedsize: " << uncompresslength << endl;
        // char *outstring =new char[1000];
        // memcpy(outstring, uncompressbuf, 1000);
        // if(myrank == 0) cout << "Process: " << myrank << endl << string(outstring) << endl;


        //string finalstr(bufchar, finalcount );
        // //cout << recvcount << endl;
        delete[] recvptr;
        // //cout << finalstr<<endl;
        //process_string(finalstr, localmap, frequencymap);
        process_buffer(bufchar, finalcount, localmap, frequencymap);
        if (ALLOW_TIME_LOGGING == 1)
            processingtime += (clock()-starttime)/(double) CLOCKS_PER_SEC;
        

        int msize = (int)mapsize(localmap)+(int)((frequencymap.size()* 20)/(1024*1024));
        int max;
        MPI_Reduce(&msize, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        
        delete[] (bufchar_header);

        if(myrank ==0){
            cout<<"MaxMapsize: "<<max<<endl;    
            if (WRITE_OUTPUT_TO_FILE == 1)
                outputstream <<"Processing file Ended: "<<(*it).c_str()<<endl;
            else   
                cout <<"Processing file Ended: "<<(*it).c_str()<<endl;
            
            
        }
        filecount++;
        //if(filecount%100 == 0)process_firstlevel(myrank, size);

         
    }



    if (ALLOW_TIME_LOGGING == 1)
        if (WRITE_OUTPUT_TO_FILE == 1)
            outputstream <<"Process: " << myrank << ". Compress time: " << compress_time << ". Compression size: " << totalcompressedsize/(1024*1024) <<" MB."<<endl;
        else   
            cout <<"Process: " << myrank << ". Compress time: " << compress_time << ". Compression size: " << totalcompressedsize/(1024*1024) <<" MB."<<endl;
}
