#include <string>
#include <stdlib.h>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <list>
#include <map>
#include <set>
#include <ctime>
#include <iostream>
#include "mpi.h"
using namespace std;

//The datastructure to hold the information about each first level word of target word
struct node
 {
    //First level word 
    string str;

    //Frequency of the firstlevel word
    int frequency;

    //co occurance count
    int cooccurance_count;

    //Used while extracting roothubs. All neighbours of roothubs are set to '-1' so that 
    //they are not considered as roothubs
    int isGoodCandidate;

    //the roothub this node belongs to. this number is the index of roothub in the vector of roothubs.
    int roothubnum;

    //score is calculated while performing MST. score = 1/(1+distance_from_roothub). 
    //roothubs will have score of 1. other nodes score is inversely proportional to the distance from roothub.
    double score;

    //Edge list of the node. node* is the other node which it is connected to. double is the weight of edge.
    std::vector<std::pair<node *,double>> edges;

    //This tree_edge is set while doing MST. It either points to the roothub it belongs to or to the node in the
    //direction of the roothub
    std::pair<node*, double> tree_edge;

    //flag to know if this node is a roothub
    bool isroothub;

    //constructor
    node():isGoodCandidate(0),roothubnum(-1),score(0),isroothub(false),tree_edge(std::make_pair(nullptr,2.0)){}

    //inserts an edge with the given other node and the edges weight.
    void insert_edge(node *newnode,double weight)
    {
        edges.push_back(std::make_pair(newnode,weight));
    }

    //for debugging purposes//prints the nodes' edges.
    void printedges()
    {
        for (std::vector<std::pair<node*,double>>::iterator it = edges.begin(); it != edges.end(); it++)
        {
            cout<<(it->first)->str<<"("<<(it->first)->frequency<<")";
        }
        cout<<endl;
    }

 };

//functor to help in ordering the edges of a nodes in the decreasing order of frequency.
struct edge_comparatordecreasing 
{
    inline bool operator() (const std::pair<node*, double> &one, const std::pair<node*, double> &two)
    {
        return one.second > two.second;
    }
};

//functor to help in ordering the edges of a nodes in the decreasing order of frequency.
struct edge_comparatorincreasing
{
    inline bool operator() (const std::pair<node*, double> &one, const std::pair<node*, double> &two)
    {
        return one.second < two.second;
    }
};

//functor to help in ordering nodes of a target word in the decreasing order of co-occurance count with targetword.
struct node_comparator
{
    inline bool operator() (const node* one, const node* two)
    {
        return one->cooccurance_count > two->cooccurance_count;
    }
};

class Graph
{
public:
    //The target word to which the graph belongs to.
    string target_word;

    //The list of all first level words(wrapped as nodes) of the target word
    std::vector<node*> nodelist;

    //The list of roothubs of the target word
    std::vector<node*> roothubs;

    //The Map to link the string form of the first level words to the wrapped node form.
    std::unordered_map<string, node*> stringtonode_map;
    
    int no_of_nodes=0;
    int no_of_edges=0;
    int degree_of_graph=0;

    //Constructor
    Graph(string t_word)
    {
        target_word=t_word;
    }

    //Input: flwords: vector of first level words of the target word
    //Input: edgelists: map linking the 'flword' to a 'list of other flwords' it has edges to.
    //(edgelists is represented in numbers, this numbers correspond to the index of the flword in flwords vector)
    //(edgelists also contains co_occurance count of the flwords)
    //(edgelists format 1 -> 3 55 4 21 8 33 11 8 )
    //(meaning the flword at index '1' in flwords vector has edges with flwords at indices 3,4,8,11 with
    //co_occuring counts 55 21 33 8 respectively.)
    //Input: fl_frequency_map frequencies of the firstlevel words.
    void create_graphwithedgelists(vector<std::pair<string,int>> &flwords, map<int, std::vector<int>> &edgelists, unordered_map<string, int> &fl_frequencymap)
    {
        //cout << "Creating graph for the word: " << target_word << endl;
        //auto &submap=localmap[target_word];

        clock_t begin = clock();
        
        int degree_of_node=0;
        

        //creating nodes for the all the first level words with information about
        //the frequency, co_occurance count.       
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

        //Go through Edgelists vector to set the edges of the nodes and also set the edge weights
        //by calculating using cooccurance counts and frequency of the nodes involved.
        for(auto &entry : edgelists)
        {
            //get first flwords string by looking for the the index in the flwords vector.
            int from_index = entry.first;
            string fromstr = flwords[from_index].first;
            //Get from node using the from string
            auto &from_node = stringtonode_map[fromstr];
            //get the edgelist of the fromnode.
            auto &edgelist_of_fromnode = entry.second;
            for(int i =0 ; i < edgelist_of_fromnode.size() ; i=i+2)
            {
                //To node index
                int to_index = edgelist_of_fromnode[i];
                //co_occurance count between the fromnode and the tonode
                int cooccurance_count = edgelist_of_fromnode[i+1];
                //get second flwords string by looking for the the index in the flwords vector.
                string tostr = flwords[to_index].first;
                //Get the to_node using the to_string
                node *to_node = stringtonode_map[tostr];
                //calculate the edgeweight
                //max of (cooccurance count divided by frequency) of the two.
                double weight= 1-std::max((double)cooccurance_count/from_node->frequency , (double)cooccurance_count/to_node->frequency);
                //insert the edge into the from node along with the node and weight information.
                from_node->insert_edge(to_node, weight);
                no_of_edges++;
                //degree_of_node++;
            }
            //degree_of_graph=std::max(degree_of_graph,degree_of_node);
            
            
            //clock_t insertstart = clock();
            
        }
        //Clear the edgelists data. Once the graph is formed edgelists is no longer required.
        edgelists.clear();

        
        //clock_t firstend = clock();
    }
    void get_roothubs(std::vector<string> &roothubsreturn)
    {        
        //sort the nodes in the descending order of cooccurance counts with the target word.
        std::sort(nodelist.begin(), nodelist.end(), node_comparator());
        //loop through all first level nodes.
        for(node* nodeptr: nodelist)
        {
            vector<string> roothubedges;

            //Get the edges of the node
            auto &edges = nodeptr->edges;

            //Sort the edges in the decreasing order of frequency            
            std::sort(edges.begin(), edges.end(), edge_comparatorincreasing());
            
            //If the candidate is already removed when a roothub was extracted, it is a bad candidate.
            if(nodeptr->isGoodCandidate==-1)continue;
            
            double average=0.0;
            int i=0;
            
            //Criteria for good candidate: The average edge weights of the top 6 most frequent neighbours 
            //should be atleast 0.8
            for(auto it = edges.begin();it!=edges.end();it++)
            {
                //if(it->first->isGoodCandidate!=-1)
                {
                    average+=it->second;
                    i++;
                    if(i==6) break;
                }
            }
            //If the node doesnot have atleat 6 neighbours, then this is a bad candidate.
            if(i!=6) continue;
            //if(edges.size()<6)continue; 
            average=average/6.0;
            if(average<=0.8)
            {   
                //i=0;
                //This is an Good candidate to select as roothub, so set all its neighbours as bad candidates
                //which is equivalent to removing from the graph.
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
                //Add the node to the roothub
                roothubs.push_back(nodeptr);

                //Exclude this node from future consideration as roothub
                nodeptr->isGoodCandidate=-1;
                //roothubedges.push_back(node->str)
                //roothubedges.clear();
            }
            
        }
        for(auto nodeptr: roothubs)roothubsreturn.push_back(nodeptr->str);
        
    }
    // void get_roothubs(std::vector<string> &roothubsreturn, int &dog, int &non, int &noe)
    // {
    //     //cout<<"T("<<target_word<<") : ";
    //     get_roothubs();
    //     for(auto nodeptr: roothubs)roothubsreturn.push_back(nodeptr->str);
    //     dog=degree_of_graph;
    //     non=no_of_nodes;
    //     noe=no_of_edges;
    // }
    
    ~Graph()
    {
        nodelist.clear();
        stringtonode_map.clear(); 
    }

    // 1. we can start from the list of roothubs as the set of vertices already in the MST(lets call it treeset).
    // 2. sort the edges of the vertices in the treeset in ascending order and 
    // 3. pick the minimum weight edge from all the first edges of all the treeset vertices and erase it from edgelist.
    // 4. then check if the other end of the edge is already in the treeset,
    // 5. if its there continue from step 3.
    // 6. otherwise add the other end to the treeset 
    // 7. continue from step 3 until all the vertices are part of the tree set.
    // 8. (we can verify this by checking the sizes of the treeset and the stringtonodemap or the flwords set)
    // 9.  once we are done with mst we need score for that we need distances.
    // 10. for roothubs score is 1.
    // 11. for all others travel until you find roothub.. add weights and calculate score.
    // 12. for 11 everyone should store the edge to the 'from where it came.'
    // 13. for 12 the first level nodes of roothubs will store edge to roothub.. the second level to first level.. and so on.
    void performMST()
    {
        int myrank = MPI::COMM_WORLD.Get_rank(); 
        //Start from the list of roothubs as the set of vertices already in the MST(treeset).
        //words in the treeset are in MST.
        std::set<string> treeset;
                
        int hubindex=0;
        for(auto &entry: roothubs)
        {
            treeset.insert(entry->str);
            entry->roothubnum=hubindex++;
            entry->score = 1;
            entry->isroothub=true;
        }
        
        //All edges of all the nodes are already sorted in the increasing manner.

        //min_weight_edge stores the next minimum weighted edge among all the nodes in the treeset
        std::pair<node*, double> min_weight_edge=std::make_pair(nullptr, 2.0);
        node* from_node=nullptr;
        int allzeroedges=0;

        std::vector<std::pair<node *,double>>::iterator min_edge_iterator;

        //Exit condition for the MST is
        //1. when all the first level nodes are in the treeset.
        //2. when all the edges of the nodes already in treeset are done checking for next minweight edge.
        while(treeset.size()!=stringtonode_map.size() && treeset.size()!=allzeroedges)
        {
            min_weight_edge=std::make_pair(nullptr, 2.0);
            //prevsize=treeset.size();
            from_node=nullptr;
            allzeroedges=0;

            //Loop through the words already in treeset.
            for(auto &entry: treeset)
            {
                auto& nodeptr = stringtonode_map[entry];
                //If there are number of edges is 0, this node is completely checked for min edges
                if(nodeptr->edges.size()==0){allzeroedges++;continue;}
                //Since the edges are already sorted, the first edge will be the minweight edge.
                auto firstedge = nodeptr->edges.begin();
                //If the other end of the minweight edge is already in the tree, then this edge
                //can be deleted from the nodes edgelist
                if(treeset.find(firstedge->first->str)!=treeset.end())
                    nodeptr->edges.erase(firstedge);
                //Otherwise if the selected edge has a weight less than the previously selected min edge
                //then pick this edge as the min_weight_edge
                else if(min_weight_edge.second>firstedge->second){
                    //from_node to keep track of the node from which the edge has come from
                    //min_edge_iterator to keep track of the edge in the edglist of fromnode                     
                    min_weight_edge = *firstedge;
                    from_node = nodeptr;
                    //nodeptr->edges.erase(firstedge);
                    min_edge_iterator= firstedge;
                }
            }
            //If some edge is selected to be part of the MST then from_node will be set
            if(from_node!=nullptr){
                treeset.insert(min_weight_edge.first->str);
                //set the tree edge of this node to the node from which we reached here.
                min_weight_edge.first->tree_edge=std::make_pair(from_node,min_weight_edge.second);
                //Once the node gets added to the tree, the edge can be deleted using the min_edge_iterator.
                from_node->edges.erase(min_edge_iterator);
            }
            // else if(treeset.size()==prevsize)
            //     break;
        }
        cout << "Process: "<<myrank << "done whileloopMST:" <<target_word <<endl;

        //Once the MST is done. We can set the roothubnum and scores of all the nodes.        
        for(auto &entry: stringtonode_map)
        {
            auto nodeptr = entry.second;
            if(!(nodeptr->isroothub))
            {
                auto tree_edge = nodeptr->tree_edge;
                double totalweight = 0;
                //if treeedge is nullptr then this node is not part of the MST.
                if(tree_edge.first==nullptr)continue;

                //traverse with the treeedge until you reach the roothub.
                while(1)
                {
                    //sum all the weights on the path to roothub.
                    totalweight+=tree_edge.second;
                    if(tree_edge.first -> isroothub)
                    {
                        nodeptr->score = 1/(1+totalweight);
                        nodeptr->roothubnum=tree_edge.first->roothubnum;
                        break;
                    }
                    else
                        tree_edge = tree_edge.first->tree_edge;
                }
            }
        }
        cout << "Process: "<<myrank << "done forloopMST:" <<target_word <<endl;
        //cout<< "treeset size:"<< treeset.size() << " total nodes: "<< stringtonode_map.size()<<endl;

    }

 };

