    #ifndef UTILITY_HPP
    #define UTILITY_HPP
    #include <iostream> 
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
    #include <fstream>
    #include <iomanip>
    #include <functional>
    #include <vector>
    #include <iostream> 
    #include <ctime>
    #include <ctype.h>
    #include <future>
    #include <thread>
    #include <fstream>
    #include <iomanip>
    #include "graph.hpp"
    #include <sqlite3.h> 
    //#include "config.hpp"
     
    
    using namespace std;


    extern std::unordered_map<string,int> frequencymap;
    extern std::unordered_map<string,std::unordered_map<string, int>> localmap;
    extern std::unordered_map<string,std::unordered_map<string, int>> localsecondlevelmap;
    extern std::vector<std::tuple<unsigned char*, long unsigned int, long unsigned int>> compressedvector;
    //extern class Graph;
    extern std::unordered_map<string, Graph*> stringtographmap;
    extern std::ofstream outputstream;

    extern std::set<string> mystringset;
    extern sqlite3 *db;
   
    void process_files(double&);
    void process_firstlevel(int&, int&);
    void process_secondlevel(int&, int&, double&);
    void disambiguate(int &myrank , int &size);
  
    // void write_to_file(string &content)
    // {



    
    typedef std::unordered_map<string, std::unordered_map<string, int>> map_stringtostringint;

    unsigned long mapsize(const std::unordered_map<string,std::unordered_map<string,int>> &map);
    std::vector<string> getallfilenames(boost::filesystem::path p);
    void insert_to_localmap(std::set<string> &stringset, std::unordered_map<string,std::unordered_map<string,int>> &localmap);
    void process_string(string &str, std::unordered_map<string,std::unordered_map<string,int>> &localmap, std::unordered_map<string, int> &frequencymap);
    void process_buffer(char *str, int length, std::unordered_map<string,std::unordered_map<string,int>> &localmap, std::unordered_map<string, int> &frequencymap);
    bool contains_punctordigit(const string &str);


    void process_files();

    void process_mem_usage(double& vm_usage, double& resident_set);
    

    #endif
