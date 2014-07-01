#pragma once
#include <string>
#include <iostream>
#include <map>
#include <string>
using namespace std;


class rule{
public:
	rule();
	rule(string filename);
	~rule();

	void load(string filename);


	int cells, dx[20], dy[20], neighbours;
	map <string, char> lookup;
	
	char result(int current, int n[8]);
};