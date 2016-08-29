#include "utility.hpp"
#include "mpi.h"
#include "zlib-1.2.8/zlib.h"
//#include "graph.hpp"
void process_files()
{
    std::vector<string> files=getallfilenames("/work/scratch/vv52zasu/inputfiles/");
    //std::vector<string> files=getallfilenames("/home/vv52zasu/mpi/inputfiles/");
    MPI::Status status; 
    int myrank = MPI::COMM_WORLD.Get_rank();
    int size = MPI::COMM_WORLD.Get_size();
    int filecount=0;
/*//////Read files in a loop and write initial data to localmap/////*/

    for(std::vector<string>::iterator it = files.begin(); it != files.end(); ++it)
    {
        if(myrank ==0) std::cout<<"Processing file:"<<(*it).c_str()<<endl;

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

        char * pch, *lastsentence;
        pch=strchr(bufchar,'\n');
        while (pch!=NULL)
        {
            if(*(pch+1)=='\n'){ lastsentence = pch+2;}
            pch=strchr(pch+1,'\n');
        }

        int sendcharcount = count -( lastsentence - bufchar );
        if(sendcharcount < 0 || sendcharcount > 300000) sendcharcount =0;
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
        // cout << "Final count: " << finalcount << " total allocated: " << CHUNKSIZE<< " count + recvcount - sendcharcount: " << count+ recvcount -sendcharcount<< endl;
        if( finalcount > CHUNKSIZE+300000) {
            cout << "Process: " << myrank <<  " exceeded size limit"<< "Final Count: "<<finalcount<<endl;
            // if(myrank==32||myrank ==94){
            // for(int i=0;i<100; i++)
            //     cout<<bufchar[i];
            // cout <<endl;
            //  }
        }

        long unsigned int destsize = compressBound(finalcount);
        unsigned char *compressedstr = new unsigned char[destsize];
        int result = compress(compressedstr, &destsize, (unsigned char*)bufchar, finalcount);
        
        compressedvector.push_back(std::make_tuple(compressedstr, destsize, finalcount));
        //string finalstr(bufchar, finalcount );
        // //cout << recvcount << endl;
        delete[] recvptr;
        // //cout << finalstr<<endl;
        //process_string(finalstr, localmap, frequencymap);
        process_buffer(bufchar, finalcount, localmap, frequencymap);
        int msize = (int)mapsize(localmap)+(int)((frequencymap.size()* 20)/(1024*1024));
        int max;
        MPI_Reduce(&msize, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        
        delete[] (bufchar_header);
        if(myrank ==0){
            cout<<"MaxMapsize: "<<max<<endl;    
            std::cout<<"Processing file Ended: "<<(*it).c_str()<<endl;
        }
        filecount++;
        //if(filecount%100 == 0)process_firstlevel(myrank, size);

         
    }
}
