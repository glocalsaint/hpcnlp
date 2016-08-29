#include "graph.hpp"
#include <map>
#include<iostream>
#include<vector>

using namespace std;



// std::shared_ptr<node> create_node(string str, int frequency)
//  {
//  	node newnode;
//  	newnode.str=str;
//  	newnode.frequency=frequency;
//  	return std::make_shared<node>(newnode);
//  }

//  void create_edge(string str1, string str2, std::vector<std::pair<string, shared_ptr<node>>> &nodemap)
//  {
//  	std::vector<std::pair<string, shared_ptr<node>>>::iterator it,it2;
//  	it=find_if( nodemap.begin(), nodemap.end(), [&str1](const pair<string, shared_ptr<node>>& entry)->bool{ return str1.compare(entry.first)==0 ;} );
//  	it2=find_if( nodemap.begin(), nodemap.end(), [&str2](const pair<string, shared_ptr<node>>& entry)->bool{ return str2.compare(entry.first)==0; } );
//  	if(it==nodemap.end() || it2==nodemap.end())
//  		return;
//  	std::shared_ptr<node> node1 = it->second;
//  	std::shared_ptr<node> node2 = it2->second;

//  	edge newedge;

//  	node1->edges.push_back(std::make_shared<edge> (newedge));
//  	node2->edges.push_back(std::make_shared<edge> (newedge));
//  	newedge.isExisting=1;
//  	newedge.node1= node1;
//  	newedge.node2= node2;
//  }

//  void construct_graph(string current_str, std::map<string, std::map<string, int>> &localmap, std::map<string,std::map<string, int>> & localsecondlevelmap, std::map<string, int> &frequencymap, std::vector<std::pair<string, shared_ptr<node>>> &nodemap)
//  {
//  	//get the first level nodes
//  	auto &firstlevelmap = localmap[current_str];
//  	for(auto it= firstlevelmap.begin(); it!=firstlevelmap.end(); it++ )
//  	{
//  		string str(it->first);
//  		nodemap.push_back(std::make_pair(str,create_node(str,frequencymap[str])));
//  	}

//  	//go through second level nodes
//  	for(auto it=firstlevelmap.begin();it!=firstlevelmap.end(); it++ )
//  	{
//  		string str(it->first); 		
//  		auto &secondlevelmap = localmap[str]; 
//  		for(auto it2 = secondlevelmap.begin();it2!=secondlevelmap.end();it2++)
//  		{
//  			string str2(it2->first);
//  			if(localmap.find(str2)!=localmap.end()) continue;
//  			create_edge(str, str2, nodemap);
//  		}
//  	}
//  }

//  void sort_graphnodes(vector<std::pair<string,std::shared_ptr<node>>> &nodemap)
//  {
//  	auto cmp = [](std::pair<string,std::shared_ptr<node>> const & a, std::pair<string,std::shared_ptr<node>> const & b) 
// 	{ 
// 		std::shared_ptr<node> node1 = a.second;
//  		std::shared_ptr<node> node2 = b.second;
// 	    return node1->frequency > node2->frequency;
// 	};
// 	std::sort(nodemap.begin(), nodemap.end(), cmp);
//  }
