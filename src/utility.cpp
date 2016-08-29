#include "utility.hpp"

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
        //int myrank = MPI::COMM_WORLD.Get_rank(); 
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
            // //processes local map that has the string entry
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

bool contains_punctordigit(const string &str)
{
    for(auto it = str.begin(); it!=str.end(); it++)
    {
        if(ispunct(*it) || isdigit(*it))
            return true;
    }
    return false;
}

/*///////////////Process String Function/////////////////*/              
     void process_string(string &str, std::unordered_map<string,std::unordered_map<string,int>> &localmap, std::unordered_map<string, int> &frequencymap)
     {
            
            // int myrank = MPI::COMM_WORLD.Get_rank(); 
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
                    if(i==2) 
                    {
                        if(contains_punctordigit(t))
                            break;
                        two.assign(t);                        
                        
                    }//}
                    if(i==3)
                    {
                        if(t.compare("NN")==0 || t.compare("ADJ")==0)
                        {
                            string uniqueword(two+t.at(0));
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

void process_buffer(char *str, int length,std::unordered_map<string,std::unordered_map<string,int>> &localmap, std::unordered_map<string, int> &frequencymap)
{
    
    // int myrank = MPI::COMM_WORLD.Get_rank(); 
    
    char *buffer = str;

    std::set<string> uniquewords;
    int k=0;
    while(str-buffer < length)
    {
        char * sentenceend = strchr(str+1,'\n');
        if(sentenceend==NULL) break;

        string line = string(str, sentenceend-str);
        boost::char_separator<char> sep("\t ");
        boost::tokenizer<boost::char_separator<char>> tokens(line, sep);
        int i=0; string lemma, pos;
        for (const auto& token : tokens) {
            if(i==2) 
            {
                if(contains_punctordigit(token))
                    break;
                lemma.assign(token);
            }
            if(i==3)
            {
                if(token.compare("NN")==0 || token.compare("ADJ")==0)
                {
                    string uniqueword(lemma+token.at(0));
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
        if((*(str+1)=='\n' ||str-buffer==length-1) && !uniquewords.empty())
        {   
            insert_to_localmap(uniquewords,localmap);  
            uniquewords.clear();                              
        }
        str=sentenceend;
    }
}
