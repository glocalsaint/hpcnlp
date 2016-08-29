#include "utility.hpp"
#include "mpi.h"
#include "zlib-1.2.8/zlib.h"
#include <cstdint>
#include <fstream>
//#include "graph.hpp"


void insert_to_vv( std::set<string> uniquewords, string sentenceid, vector<vector<string>> &vv, int size)
{
	string sentence=sentenceid;
	for(auto &word: uniquewords)
		sentence+=" "+word;
	//cout << endl<<sentence<<endl;
	for(auto &word : uniquewords)
	{	
		vv[std::hash<string>()(word) % size].push_back(sentence);
	}

}
 void extract_words(char *str, int length, vector<vector<string>> &vv, int size)
 {
        
    char *buffer = str;
    // int myrank = MPI::COMM_WORLD.Get_rank(); 
    int current_index=0, index=0;
        
    std::set<string> uniquewords;
    int k=0;

    string sentenceid;
    while(str-buffer < length)
    {
        //index = str.find('\n',index+1);
        char * sentenceend = strchr(str+1,'\n');
        if(sentenceend==NULL) break;
        //if(*(sentenceend+1) == '\n') continue;
        
        string line = string(str, sentenceend-str);//str.substr(current_index,index - current_index);
        //cout << line<<endl;
        boost::char_separator<char> sep("\t");
        boost::tokenizer<boost::char_separator<char>> tokens(line, sep);
        int i=0; 
        string lemma;
        
        vector<string> vtokens;
        std::copy(tokens.begin(), tokens.end(), std::back_inserter(vtokens)); 
        if( vtokens.size() > 4 ){
			sentenceid = vtokens.back();        
			//cout << "Sentenceid: " << sentenceid << endl;
        }
        for ( const auto& token : tokens ) 
        {
            if(i==2) {lemma.assign(token); if(ispunct(lemma.at(0)) || isdigit(lemma.at(0)))break;}//}
            if(i==3)
            {
                if(token.compare("NN") == 0 || token.compare("ADJ") == 0)
                {
                    string uniqueword(lemma+token.at(0));
                    if(uniqueword.length()>STRING_LENGTH-2)break;
                    uniquewords.insert(uniqueword);
                }
            }
            i++;
            //if(i>3) break;
            //if(i==5).assign(token);
        }
        if((*(str+1)=='\n' ||index==length-1) && !uniquewords.empty())
        {   
            insert_to_vv(uniquewords, sentenceid, vv, size);  
            uniquewords.clear();                              
        }
        str=sentenceend;
    }
    str = buffer;
}

int get_max_pos(double * array, int size)
{
    double max=array[0];
    int max_pos=0;

    int i;
    for (i=1; i<size; i++)
    {
        if (max<array[i])
        {
            max=array[i];
            max_pos=i;
        }
    }

    return max_pos;
}

void wsd(string &str, vector<std::tuple<string, string, string>> &vtuplestowrite)
{
	boost::char_separator<char> sep(" ");
    boost::tokenizer<boost::char_separator<char>> tokens(str, sep);
    int i=0; 
    string sentenceid;
    vector<string> words;
    for (const auto& token : tokens) {
    	if(i==0)
    	{
    		sentenceid.assign(token);
    		i++;
    		continue;
    	}
    	words.push_back(token);
    }
    for(auto &word : words)
    {
    	if(stringtographmap.find(word)!=stringtographmap.end())
    	{
    		
    		Graph *graph = stringtographmap[word];
    		int roothubcount = graph->roothubs.size();
    		if(roothubcount==0)continue;

    		double scorevector[roothubcount] = {0};


    		for(auto &word1 : words)
    		{
    			if(word1.compare(word)!=0)
    		 	{
    		 		if(graph->stringtonode_map.find(word1)!=graph->stringtonode_map.end())
    		 		{
    		 			//cout <<"came";
	    		 		auto nodeptr = graph->stringtonode_map[word1];
	    		 		double score = nodeptr->score;
	    		 		int closeroothub = nodeptr->roothubnum;
	    		 		scorevector[closeroothub] += score;
    		 		}
    		 	}
    		}
    		int indexofmaxscore = get_max_pos(scorevector, roothubcount);
    		// int indexofmaxscore = distance(scorevector, max_element(scorevector, scorevector + roothubcount));
    		string wordsense = (graph->roothubs[indexofmaxscore])->str;
    		vtuplestowrite.push_back(std::make_tuple(sentenceid, word, wordsense)); 
    	}

    }

}

void disambiguate(int &myrank , int &size)
{
	typedef uint8_t byte;
	//cout << "Came";
	vector<std::tuple<string, string, string>> vtuplestowrite;
	vector<vector<string>> vv(size);
	ofstream myfile;
  	myfile.open ("wsd"+to_string(myrank)+".txt");
	clock_t wsdtime = clock();
	for(auto &compressedentry : compressedvector)
	{
		auto compressedstr = std::get<0>(compressedentry);
		auto compressedlen = std::get<1>(compressedentry);
		auto originallen = std::get<2>(compressedentry);
		unsigned char *uncompressbuf = new unsigned char[(int)originallen];
		long unsigned int uncompresslength;
		int result = uncompress(uncompressbuf, &uncompresslength, compressedstr, compressedlen);
		MPI::COMM_WORLD.Barrier();

		extract_words((char*)uncompressbuf, uncompresslength, vv, size);
		char *recvdata = new char[1];
		int totalrecvsize=0, totalrecv_strings=0;


		for( int i=0 ; i < size ; i++ )
		{
			int sendsize = 0;
			char* sendbuffer = new char[1];
			int sendwords=0;
			if(myrank != i)
			{
				sendwords=vv[i].size();
				sendsize=vv[i].size()*sizeof(int);// + sizeof(int);
				for(auto &entry : vv[i])
					sendsize+=entry.size()+1;

				sendbuffer = new char[ sendsize ];
        		char* memptr = sendbuffer;
        		// int N = vv[i].size();
        		// memcpy( memptr , &N , sizeof(int));
        		// memptr += sizeof(int);
        		for( auto &entry : vv[i] )
		        {
		        	int b = entry.size()+1;
		            memcpy( memptr , &b , sizeof(int));
		            memptr += sizeof(int);
		            
		            memcpy( memptr , entry.c_str() , entry.size() + 1);            
		            memptr += entry.size()+1;
		        }
			}
			int *allocsizes= new int[size];
			MPI_Reduce(&sendwords ,    &totalrecv_strings,    1,    MPI_INT,    MPI_SUM,  i,    MPI_COMM_WORLD);
        	MPI_Gather(&sendsize,  1,  MPI_INT,  allocsizes,  1,  MPI_INT,  i,  MPI_COMM_WORLD);

        	int *allocdisplacements = new int[size];
	        allocdisplacements[0] = 0;
	        for( int p=0 ; p < size ; p++ )
	        {
	            allocdisplacements[p]=allocdisplacements[p-1]+allocsizes[p-1];
	        }
	         
	        
	        if ( myrank == i )
	        {
	        	totalrecvsize = allocdisplacements[size-1]+ allocsizes[size-1];
	            recvdata = new char[totalrecvsize];
	        }
	        
	        MPI_Gatherv(sendbuffer, sendsize, MPI_CHAR, recvdata,  allocsizes, allocdisplacements, MPI_CHAR , i ,  MPI_COMM_WORLD);
	        
	        delete[](sendbuffer);

		}
		char *memptr = recvdata;
		for(int p = 0 ; p < size ; p++ )
		{
			if(myrank != p)
				vv[p].clear();
		}
		// cout << totalrecv_strings<<endl;
		//if(myrank == 0)
		// for(int j =0; j< 200 ; j ++)
		// {
		// 	cout << recvdata[j];
		// }
			
		// 	cout << endl;
		//if(myrank==0)
		while(totalrecv_strings-- )
		{
			if(memptr - recvdata  > totalrecvsize) break;
			// int count;
			// memcpy( &count , memptr , sizeof(int) );
   //      	memptr += sizeof(int);
   //      	if(myrank==0)cout << "|"<<count<<"|";
   //      	for(int j = 0 ; j < count ; j++ )
   //      	{
        		int strlen;
				memcpy( &strlen , memptr , sizeof(int) );
	        	memptr += sizeof(int);
	        	//cout<< "|" << strlen << "|";

	        	char *str = new char[strlen];
	        	memcpy(str, memptr, strlen);
	        	memptr+=strlen;
	         	//cout << "|" << str << endl;
	         	vv[myrank].push_back(str);
	         	delete []str;
        	//}
		}

		for(auto &sentence : vv[myrank])
			wsd(sentence, vtuplestowrite);
		delete[] recvdata;
		// //write to file;
		for(auto &entry: vtuplestowrite)
		{
			//cout << std::get<0>(entry)<<" : " << std::get<1>(entry)<<" : " << std::get<2>(entry)<<" : " << endl;
			myfile << std::get<0>(entry)<<" : " << std::get<1>(entry)<<" : " << std::get<2>(entry)<<"\n";
		}


	}
	if( myrank == size-1 )cout<<endl<<"Time taken for wsd" << (clock()-wsdtime)/(double) CLOCKS_PER_SEC << "\n";
	myfile.close();
}
