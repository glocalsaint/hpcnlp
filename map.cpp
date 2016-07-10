    #include <iostream> 
    #include <cstring>
    #include <string>
    #include <stdlib.h>
    #include <algorithm>
    #include <unordered_map>
    #include <set>
    #include <iostream>
    #include <fstream>
    #include <map>
    #include <ctime>
    #include <ctype.h>
    #include <future>
    #include <thread>
    #include <fstream>
    #include <iomanip>
    #include <functional>
using namespace std; 
int main()
{

	unordered_map<string , unordered_map<string,int>> map;
	cout << map.size() <<endl;
	char tword[40];
	string str("abcde");
	memcpy(tword, str.c_str(), str.size() +1);
	auto &submap = map[tword];
	submap[str]+=30;
	cout << map.size() <<endl;
}

