#pragma once
#include "control.h"
#include <set>
#include <map>

enum AutomataTool{
	tool_toggle = 0,
	tool_revive = 1,
	tool_kill = 2,
};

class AutomataEngine:public control{
public:
	AutomataEngine(form* parent, int x, int y, int w, int h, int columns, int rows);
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

	*/
	struct block{
		short N0s;
		char data[4][4];
	}empty;
	set <pair<short,short> > fieldChange;
	map <pair<short,short>, block> state;
	int columns, rows;
	AutomataTool tool; int team;
	bool warp;
	void simulate(int steps);
	void generate();
	int determine(char current, char neighbours[8]);
	~AutomataEngine(void);

	//interface
	int colourtable[10];
	float panx, pany, zoom;
	void affect(int x, int y);
	int get(int x, int y);

	void panLeft();
	void panRight();
	void panUp();
	void panDown();
	void zoomIn();
	void zoomOut();
};

void downHandler(SDL_Event *e, void *p);
