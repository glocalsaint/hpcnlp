
    #include "src/utility.hpp"
    #include "mpi.h"
   
void process_files()
{
    std::vector<string> files=getallfilenames("/work/scratch/vv52zasu/inputfiles/");
    //std::vector<string> files=getallfilenames("/home/vv52zasu/mpi/inputfiles/");
    int bufsize, *buf;
    string bufchar1;
    char filename[128]; 

    MPI::Status status; 
    int myrank = MPI::COMM_WORLD.Get_rank(); 
    int size = MPI::COMM_WORLD.Get_size(); 

/*//////Read files in a loop and write initial data to localmap/////*/
    
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
}