#pragma once
#include "control.h"
#include <set>
#include <map>
#define _XhashSize hashsize
enum XAutomataTool{
	xtool_toggle = 0,
	xtool_revive = 1,
	xtool_kill = 2,
};

enum AutomataTool;
class XAutomataEngine:public control{
public:
	int generation;
	XAutomataEngine(form* parent, int x, int y, int w, int h, int columns, int rows, bool keyBound = true);
	void draw(SDL_Surface *s);
	void frameTick(SDL_Event *e);
	float ticksPerFrame;
	bool paused;
	float currentTick;
	
	//cellular data. BIG DESIGN CHANGE.
	/*
	The field is split into 4x4 blocks. Each block has an address - a co-ordinate.
	
	An address is added to the fieldChange set if the cells have changed state
	in the previous generation. Only those in the fieldChange set and its neighbours
	are considered for computation in the next generation.

	Cellular data is added to the state map. Key: address, value: 16 bytes (I hope)
	representing the 4x4 cell states, up to 256 different states. Blocks containing
	only 0's are removed from the map, and any block not in the map contain all 0's.

	16/10/12
	shifting processing off-object for efficiency (NOPE)
	making storage of unsigned integers.
	*/
	map<short,char> state[100000];
	
	unsigned int *proc;
	int columns, rows;
	XAutomataTool tool; int team;
	bool warp;
	void simulate(int steps);
	void generate();
	bool ruleRestricted;
	int determine(char current, char neighbours[8]);
	~XAutomataEngine(void);
	
	//it will also now use hashing to calculate shit.
	int collisions;
	int hashKey(unsigned int &val){
		int a =( val>>16), b = val&0x0000ffff;
		unsigned int r = a*30000+b*2+b*10231+a*3;
		return  r%hashsize;
	}

	//anti vector optimization
	int NumInHash;

	pair<unsigned int,char> *toAdd;
	unsigned int *toChange;
	int toAddSize, toAdds, toChangeSize, toChanges;

	//end anti-vector optimization

	int hashsize, hashsizeindex, hashsizes[5];

	void store(unsigned int val){
		int key = hashKey(val);
		if(hashsize>2000){
			int i = 0;
		}
		while(proc[key]!=0xffffffff&&proc[key]!=val){
			key++;
			collisions++;
			//cout<<key<<"\n";
		}
		proc[key] = val;
		NumInHash ++;
	}

	void resetHash(){
		
		if(NumInHash<(hashsize>>8)){
			NumInHash = 0;
			resizeTable(hashsizeindex-1);
		}
		if(NumInHash>(hashsize>>4)) {
			NumInHash = 0;
			resizeTable(hashsizeindex+1);
		}
		for(int i = 0; i < _XhashSize+1000; i++) proc[i] = 0xffffffff;
		NumInHash = 0;
		
	}

	void resizeTable(int size){
		if(size<0||size>=5) return;
		if(NumInHash!=0){
			cout<<NumInHash<<"\n";
			//move everything.
			unsigned int *temp = new unsigned int[NumInHash];
			int index = 0;
			for(int i = 0; i < hashsize+1000; i++){
				if(proc[i]!=0xffffffff) temp[index++] = proc[i];
			}
			
			hashsize = hashsizes[size];
			hashsizeindex = size;
			int NumInTemp = NumInHash;
			for(int i = 0; i < _XhashSize+1000; i++) proc[i] = 0xffffffff;
			NumInHash = 0;
			for(int i = NumInTemp; i >0; i--){
				store(temp[i]);
			}
			delete[] temp;
		}else{
			cout<<"-"<<NumInHash<<"\n";
			hashsize = hashsizes[size];
			hashsizeindex = size;
		}
	}

	//interface
	int colourtable[10];
	int panx, pany, zoom;
	int startingX, startingY;
	void affect(int x, int y);
	int get(int x, int y);
	void getGridCoords(int mouseX, int mouseY, int *destX, int *destY);


	void panLeft();
	void panRight();
	void panUp();
	void panDown();
	void zoomIn();
	void zoomOut();
	void fit();
	//experiment
	char table[8][8][8][8][8];
};

void XKeyHandler(SDL_Event *e, void*p);

void XmoveHandler(SDL_Event *e, void *p);
void XdownHandler(SDL_Event *e, void *p);
