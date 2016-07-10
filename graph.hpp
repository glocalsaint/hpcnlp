 #include <string>
#include <stdlib.h>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <list>
#include <map>
#include <ctime>
#include<iostream>
using namespace std;

struct node
 {
    string str;
    int frequency;
    int isGoodCandidate = 0;
    std::vector<std::pair<node *,double>> edges;

    void insert_edge(node *newnode,double weight)
    {
        edges.push_back(std::make_pair(newnode,weight));
        // //cout<<"Inserting node into list:"<<newnode->str<<endl<<flush;
        // if(edges.empty())
        // {
        //     edges.push_back(std::make_pair(newnode,weight));
        //     return;
        // }

        //  if(edges.back().first->frequency <= newnode->frequency)
        // {        
        //     std::list<std::pair<node*,double>>::iterator it = edges.begin();    
        //     for (; it != edges.end(); it++)
        //     {
        //         if(((*it).first)->frequency < newnode->frequency)
        //         { 
        //             edges.insert (it,std::make_pair(newnode,weight));
        //             return;
        //         }else if(((*it).first)->str.compare(newnode->str)==0)
        //         {
        //             return;
        //         }
        //     }
        //     if(it==edges.end())
        //         edges.insert (it,std::make_pair(newnode,weight));

        // }
        // else
        // {
        //     edges.push_back(std::make_pair(newnode,weight));
        // }
        
    }
    void printedges()
    {
        for (std::vector<std::pair<node*,double>>::iterator it = edges.begin(); it != edges.end(); it++)
        {
            cout<<(it->first)->str<<"("<<(it->first)->frequency<<")";
        }
        cout<<endl;
    }

 };

struct edge_comparator
{
    inline bool operator() (const std::pair<node*, double> &one, const std::pair<node*, double> &two)
    {
        return one.first->frequency > two.first->frequency;
    }
};

struct node_comparator
{
    inline bool operator() (const node* one, const node* two)
    {
        return one->frequency > two->frequency;
    }
};

 class Nodelist
 {

 	std::vector<node*> nodelist;
 public:
    ~Nodelist()
    {
        
    }
    void get_roothubs(std::map<string,std::vector<string>> &roothubs)
    {        
        std::sort(nodelist.begin(), nodelist.end(), node_comparator());
        for(auto &node: nodelist)
        {
            vector<string> roothubedges;
            if(node->isGoodCandidate==-1)continue;
            auto &edges = node->edges;
            double average=0.0;int i=0;
            std::sort(edges.begin(), edges.end(), edge_comparator());
            for(auto it = edges.begin();i<6 && it!=edges.end();it++)
            {
                if(it->first->isGoodCandidate!=-1)
                {
                    average+=it->second;
                    i++;
                }
            }
            if(i!=6) continue;
            average=average/6.0;
            if(average>=0.8)
            {                i=0;
                for(auto edge: node->edges)
                {
                    if(edge.first->isGoodCandidate!=-1)
                    {
                        edge.first->isGoodCandidate=-1;
                        roothubedges.push_back(string(edge.first->str)+"("+to_string(edge.second)+")");
                        i++;
                    }
                    if(i==6)break;
                }    
                roothubs[node->str]=roothubedges;
                roothubedges.clear();
            }
        }
        
    }
 	void insert_node(node *newnode)
    {        
        // //cout<<"Inserting node into list:"<<newnode->str<<endl<<flush;
        // if(!nodelist.empty() && nodelist.back()->frequency < newnode->frequency)
        // {            
        //     for (std::list<node*>::iterator it = nodelist.begin(); it != nodelist.end(); it++)
        //     {
        //         if((*it)->frequency<=newnode->frequency)
        //         { 
        //             nodelist.insert (it,newnode);   
        //             break;
        //         }
        //     }
        // }
        // else
            nodelist.push_back(newnode);
    }
    void print()
    {
        for (auto it = nodelist.begin(); it != nodelist.end(); it++)
        {
            cout<<(*it)->str<<"("<<(*it)->frequency<<")";
        }
        cout<<endl;
    }

    void delete_node(int &frequency, node *deletenode)
    {
        deletenode->str="";
        deletenode->frequency=-1;
        deletenode->edges;
    }
    void clear()
    {
        for(auto entry: nodelist)
        {
            delete entry;
        }
        nodelist.clear();

    }
 };

 class Graph
 {

    string target_word;
    Nodelist nodelist;
    std::unordered_map<string, node*> stringtonode_map;
    int no_of_nodes=0;
    int no_of_edges=0;
    int degree_of_graph=0;
    public:
    Graph(string t_word)
    {
        target_word=t_word;

    }
    void create_graph(std::unordered_map<string, std::unordered_map<string,int>> &localmap, std::unordered_map<string,int> &frequency_map)
    {
        clock_t begin = clock();
        //cout << "Creating graph for the word: " << target_word << endl;
        auto &submap=localmap[target_word];
        //creating nodes for the first level nodes
        for(auto &entry: submap)
        {
            node *node_ptr=new node();
            string entrystr(entry.first);
            node_ptr->str=entrystr;
            node_ptr->frequency=frequency_map[entrystr];
            stringtonode_map[entrystr]=node_ptr;
            nodelist.insert_node(node_ptr);
            no_of_nodes++;
        }

        //nodelist.print();
        //going through all second level nodes and if there is string which is in first level node then form an edge.
        clock_t firstend = clock();
        double timeforinserting =0;
        for(auto &entry: submap)
        {

            string fl_str(entry.first);
            if(localmap.find(fl_str)!=localmap.end())
            {
                auto &flsubmap = localmap[fl_str];
                node *first_node; 
                first_node= stringtonode_map[fl_str];
                int degree_of_node=0;
                for(auto &slentry: flsubmap)
                {   
                    
                    string sl_str(slentry.first);
                    if(stringtonode_map.find(sl_str)!=stringtonode_map.end())
                    {                        
                        node *second_node;
                        second_node = stringtonode_map[sl_str];
                        //max of cooccurance count divided by frequency of the two.
                        double weight= 1-std::max((double)slentry.second/first_node->frequency , (double)slentry.second/second_node->frequency);
                        clock_t insertstart = clock();
                        first_node->insert_edge(second_node, weight);
                        //second_node->insert_edge(first_node, weight); 
                        timeforinserting += double(clock() - insertstart);
                        no_of_edges++;
                        degree_of_node++;
                    }
                }
                degree_of_graph=std::max(degree_of_graph,degree_of_node);
                //first_node->printedges();firstend
            }
        }
        clock_t end = clock();
        cout << "Finished creating  graph for the word: " << target_word << ". Time elapsed: "<<double(end - begin) / CLOCKS_PER_SEC<< ". Insert time: "<< timeforinserting/CLOCKS_PER_SEC<<endl;
    }

    void get_roothubs(std::map<string,std::vector<string>> &roothubs, int &dog, int &non, int &noe)
    {
        //cout<<"T("<<target_word<<") : ";
        nodelist.get_roothubs(roothubs);
        dog=degree_of_graph;
        non=no_of_nodes;
        noe=no_of_edges;
    }
    
    ~Graph()
    {
        nodelist.clear();
        stringtonode_map.clear(); 
    }

 };

