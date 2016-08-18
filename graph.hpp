 #include <string>
#include <stdlib.h>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <list>
#include <map>
#include <set>
#include <ctime>
#include<iostream>
using namespace std;

struct node
 {
    string str;
    int frequency;
    int cooccurance_count;
    int isGoodCandidate;
    int roothutnum;
    double score;
    std::vector<std::pair<node *,double>> edges;
    std::pair<node*, double> tree_edge;
    bool isroothub;


    node():isGoodCandidate(0),roothutnum(-1),score(-1),isroothub(false),tree_edge(std::make_pair(nullptr,2.0)){}
    void insert_edge(node *newnode,double weight)
    {
        edges.push_back(std::make_pair(newnode,weight));
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

struct edge_comparatordecreasing 
{
    inline bool operator() (const std::pair<node*, double> &one, const std::pair<node*, double> &two)
    {
        return one.second > two.second;
    }
};

struct edge_comparatorincreasing
{
    inline bool operator() (const std::pair<node*, double> &one, const std::pair<node*, double> &two)
    {
        return one.second < two.second;
    }
};

struct node_comparator
{
    inline bool operator() (const node* one, const node* two)
    {
        return one->cooccurance_count > two->cooccurance_count;
    }
};

class Graph
{

    string target_word;
    std::vector<node*> nodelist;
    std::vector<node*> roothubs;
    std::unordered_map<string, node*> stringtonode_map;
    int no_of_nodes=0;
    int no_of_edges=0;
    int degree_of_graph=0;

    public:
    Graph(string t_word)
    {
        target_word=t_word;

    }

    void create_graphwithedgelists(vector<std::pair<string,int>> &flwords, map<int, std::vector<int>> &edgelists, unordered_map<string, int> &fl_frequencymap)
    {
        clock_t begin = clock();
        //cout << "Creating graph for the word: " << target_word << endl;
        //auto &submap=localmap[target_word];
        int degree_of_node=0;
        //creating nodes for the first level nodes
        

        for(auto &entry: flwords)
        {

            node *node_ptr=new node();
            //string entrystr(entry.first);
            node_ptr->str=entry.first;
            node_ptr->frequency=fl_frequencymap[entry.first];
            node_ptr->cooccurance_count=entry.second;
            stringtonode_map[entry.first]=node_ptr;
            nodelist.push_back(node_ptr);
            no_of_nodes++;
        }
        for(auto &entry : edgelists)
        {
            string fromstr = flwords[entry.first].first;
            auto &first_node = stringtonode_map[fromstr];
            auto &edgelist = entry.second;
            for(int i =0 ; i < entry.second.size() ; i=i+2)
            {
                //cout<"JJ";
                int to = edgelist[i];
                int cooccurance_count = edgelist[i+1];
                string tostr = flwords[to].first;
                node *second_node = stringtonode_map[tostr];
                //max of cooccurance count divided by frequency of the two.
                double weight= 1-std::max((double)cooccurance_count/first_node->frequency , (double)cooccurance_count/second_node->frequency);
                first_node->insert_edge(second_node, weight);
                no_of_edges++;
                //degree_of_node++;
            }
            //degree_of_graph=std::max(degree_of_graph,degree_of_node);
            
            
            clock_t insertstart = clock();
            
        }

        //going through all second level nodes and if there is string which is in first level node then form an edge.
        clock_t firstend = clock();
    }
    void get_roothubs()
    {        
        std::sort(nodelist.begin(), nodelist.end(), node_comparator());
        for(node* nodeptr: nodelist)
        {
            vector<string> roothubedges;
            if(nodeptr->isGoodCandidate==-1)continue;
            auto &edges = nodeptr->edges;
            double average=0.0;int i=0;
            std::sort(edges.begin(), edges.end(), edge_comparatorincreasing());
            for(auto it = edges.begin();it!=edges.end();it++)
            {
                //if(it->first->isGoodCandidate!=-1)
                {
                    average+=it->second;
                    i++;
                    if(i==6) break;
                }
            }
            if(i!=6) continue;
            //if(edges.size()<6)continue; 
            average=average/6.0;
            if(average<=0.8)
            {                i=0;
                for(auto edge: nodeptr->edges)
                {
                    //if(edge.first->isGoodCandidate!=-1)
                    {
                        edge.first->isGoodCandidate=-1;
                        //roothubedges.push_back(string(edge.first->str)+"("+to_string(edge.second)+")");
                        //i++;
                    }
                    //if(i==6)break;
                }    
                //roothubs[node->str + to_string(edges.size())]=roothubedges;
                roothubs.push_back(nodeptr);
                nodeptr->isGoodCandidate=-1;
                //roothubedges.push_back(node->str)
                //roothubedges.clear();
            }
            
        }
        
    }
    void get_roothubs(std::vector<string> &roothubsreturn, int &dog, int &non, int &noe)
    {
        //cout<<"T("<<target_word<<") : ";
        get_roothubs();
        for(auto nodeptr: roothubs)roothubsreturn.push_back(nodeptr->str);
        dog=degree_of_graph;
        non=no_of_nodes;
        noe=no_of_edges;
    }
    
    ~Graph()
    {
        nodelist.clear();
        stringtonode_map.clear(); 
    }
    void performMST()
    {
        //what is needed
        //1. we can start from the list of roothubs as the set of vertices already in the MST(lets call it treeset).
        //2. sort the edges of the vertices in the treeset in ascending order and 
        //3. pick the minimum weight edge from all the first edges of all the treeset vertices and erase it from edgelist.
        //4. then check if the other end of the edge is already in the treeset,
        //5. if its there continue from step 3.
        //6. otherwise add the other end to the treeset and sort its edges in ascending order.
        //7. continue from step 3 until all the vertices are part of the tree set.
        //8. (we can verify this by checking the sizes of the treeset and the stringtonodemap or the flwords set)
        //9. once we are done with mst we need score for that we need distances.
        //10. for roothubs score is 1.
        //11. for all others travel until you find roothub.. add weights and calculate score.
        //12. for 11 everyone should store the edge to the 'from where it came.'
        //13. for 12 the first level nodes of roothubs will store edge to roothub.. the second level to first level.. and so on.

        //1
        std::set<string> treeset;
        
        //2. all edges are already sorted.
        int hubindex=0;
        for(auto &entry: roothubs)
        {
            treeset.insert(entry->str);
            entry->roothutnum=hubindex++;
            entry->score = 1;
            entry->isroothub=true;
        }
        //cout<<"Came1"<<endl;
        //3
        std::pair<node*, double> minweightedge=std::make_pair(nullptr, 2.0);
        node* from_node=nullptr;
        int prevsize=0;;
        while(treeset.size()!=prevsize)//treeset.size()!=stringtonode_map.size())
        {
            //minweightedge=std::make_pair(nullptr, 2.0);
            prevsize=treeset.size();
            from_node=nullptr;
            for(auto &entry: treeset)
            {
                auto& nodeptr = stringtonode_map[entry];
                if(nodeptr->edges.size()==0)continue;
                auto firstedge = nodeptr->edges.begin();
                if(treeset.find(firstedge->first->str)!=treeset.end())
                    nodeptr->edges.erase(firstedge);
                else if(minweightedge.second>firstedge->second){
                    minweightedge = *firstedge;
                    from_node = nodeptr;
                }
            }
            treeset.insert(minweightedge.first->str);
            if(from_node!=nullptr)
                minweightedge.first->tree_edge=std::make_pair(from_node,minweightedge.second);
            else if(treeset.size()!=prevsize)
                break;
        }
        //cout<<"Came2"<<endl;
        for(auto &entry: stringtonode_map)
        {
            auto nodeptr = entry.second;
            if(!(nodeptr->isroothub))
            {
                auto tree_edge = nodeptr->tree_edge;
                double totalweight = 0;
                if(tree_edge.first==nullptr)continue;
                while(1)
                {
                    totalweight+=tree_edge.second;
                    if(tree_edge.first -> isroothub)
                    {
                        nodeptr->score = 1/(1+totalweight);
                        nodeptr->roothutnum=tree_edge.first->roothutnum;
                        break;
                    }
                    else
                        tree_edge = tree_edge.first->tree_edge;
                }
            }
        }
        //cout<<"Came3"<<endl;
        cout<< "treeset size:"<< treeset.size() << " total nodes: "<< stringtonode_map.size()<<endl;

    }

 };

// class Nodelist
 // {
 //    std::vector<node*> nodelist;
    
 // public:
 //    ~Nodelist()
 //    {
        
 //    }
 //    void get_roothubs(std::map<string,std::vector<string>> &roothubs)
 //    {        
 //        std::sort(nodelist.begin(), nodelist.end(), node_comparator());
 //        for(auto &node: nodelist)
 //        {
 //            vector<string> roothubedges;
 //            if(node->isGoodCandidate==-1)continue;
 //            auto &edges = node->edges;
 //            double average=0.0;int i=0;
 //            // std::sort(edges.begin(), edges.end(), edge_comparator());
 //            // for(auto it = edges.begin();it!=edges.end();it++)
 //            // {
 //            //     //if(it->first->isGoodCandidate!=-1)
 //            //     {
 //            //         average+=it->second;
 //            //         i++;
 //            //         if(i==6) break;
 //            //     }
 //            // }
 //            // if(i!=6) continue;
 //            //if(edges.size()<6)continue; 
 //            // average=average/6.0;
 //            //if(average<=0.4)
 //            {                i=0;
 //                for(auto edge: node->edges)
 //                {
 //                    if(edge.first->isGoodCandidate!=-1)
 //                    {
 //                        edge.first->isGoodCandidate=-1;
 //                        //roothubedges.push_back(string(edge.first->str)+"("+to_string(edge.second)+")");
 //                        //i++;
 //                    }
 //                    //if(i==6)break;
 //                }    
 //                roothubs[node->str + to_string(edges.size())]=roothubedges;
 //                node->isGoodCandidate=-1;
 //                //roothubedges.push_back(node->str)
 //                //roothubedges.clear();
 //            }
            
 //        }
        
 //    }
 //     void insert_node(node *newnode)
 //    {        
 //        // //cout<<"Inserting node into list:"<<newnode->str<<endl<<flush;
 //        // if(!nodelist.empty() && nodelist.back()->frequency < newnode->frequency)
 //        // {            
 //        //     for (std::list<node*>::iterator it = nodelist.begin(); it != nodelist.end(); it++)
 //        //     {
 //        //         if((*it)->frequency<=newnode->frequency)
 //        //         { 
 //        //             nodelist.insert (it,newnode);   
 //        //             break;
 //        //         }
 //        //     }
 //        // }
 //        // else
 //            nodelist.push_back(newnode);
 //    }
 //    void print()
 //    {
 //        for (auto it = nodelist.begin(); it != nodelist.end(); it++)
 //        {
 //            cout<<(*it)->str<<"("<<(*it)->frequency<<")";
 //        }
 //        cout<<endl;
 //    }

 //    void delete_node(int &frequency, node *deletenode)
 //    {
 //        deletenode->str="";
 //        deletenode->frequency=-1;
 //        deletenode->edges;
 //    }
 //    void clear()
 //    {
 //        for(auto entry: nodelist)
 //        {
 //            delete entry;
 //        }
 //        nodelist.clear();

 //    }
 // };