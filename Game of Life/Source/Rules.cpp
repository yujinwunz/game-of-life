//IN PROGRESS
//to be able to read rules from a transition file format.
#include "Rules.h"
#include <fstream>
rule::rule(){

}

rule::rule(string filename){
	load(filename);
}

rule::~rule(){};

void rule::load(string filename){
	ifstream in(filename);
	string file[10000];
	int line = 0;
	while(!in.eof()){
		string input;
		getline(in,input);
		if(input=="") continue;
		if(input[0]=='#') continue;
		
		file[line++] = input;
	}
}

char rule::result(int current, int n[8]){
	return 0;
}