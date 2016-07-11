 #include "src/utility.hpp"
#include "mpi.h"
void writetofile(int &myrank, int &size){
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
}