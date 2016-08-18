#include<iostream>
#include<string>
#include<algorithm>
#include<stdlib.h>
#include<functional>
#include "zlib-1.2.8/zlib.h"
#include<iomanip>
#include<cstring>
#include<fstream>

using namespace std;
int main()
{
	ifstream infile;
	infile.open ("inputfiles/resource-dump.txt.1.tsv");
	string src="";
	string temp;
        while(std::getline(infile, temp)) // To get you all the lines.
   	{
	        getline(infile,temp); // Saves the line in STRING.
        	src+=temp;
	}
	infile.close();
	string source = "viswanath is a gThis happens when the pointer passed to free() is not valid or has  \
	been modified somehow. I don't really know the details here. The bottom line is that the pointer passed \
	 to free() must be the same as returned by malloc(), realloc() and their friends. It's not always easy to  \
	spot what the problem is for a novice in their own code or even deeper in a library. In my case, it was a simple \
	case of an undefined (uninitialized) pointer related to branching.ood boy";
	unsigned char  *bytesource = new unsigned char[src.size()+1];
	std::cout<< "hello "<<src.size()<<endl;
	memcpy(bytesource, src.c_str(), src.size()+1);
	std::cout<< "hello1"<<endl;
	unsigned char *dest= new unsigned char[src.size()+600];
	std::cout<< "hello2"<<endl;
	long unsigned int destsize;
	std::cout<< "hello3"<<endl;
	long unsigned int sourcesize = (long unsigned int)src.size()+1;
	std::cout<< "hello4"<<endl;
	int result = compress(dest, &destsize, bytesource, sourcesize);
	std::cout<< "hello5"<<" "<< sourcesize << " " <<  destsize <<endl;
}
