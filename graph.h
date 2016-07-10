 #include <string>
#include <stdlib.h>
#include <algorithm>
#include <unordered_map>
 struct node
 {
    string str;
    int frequency;
    std::vector<shared_ptr<edge>> edges; 
 };

 struct edge
 {
    int weight;
    int isExisting;
    shared_ptr<node> node1;
    shared_ptr<node> node2;
 };

 




