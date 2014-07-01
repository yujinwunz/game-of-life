#include "AutomataEngine.h"


AutomataEngine::AutomataEngine(form* parent, int x, int y, int w, int h, int columns, int rows):control(parent,x,y,w,h){
	this->addHandler(event_mousebuttondown,downHandler);
	this->columns = columns; this->rows = rows;

	ticksPerFrame = 0.1;
	paused = true;
	currentTick = 1.0;

	//interface
	panx = pany = 0.0;
	zoom = 10.0;
	tool = tool_toggle;
	team = 0;
	//FILL COLOUR TABLE.
	colourtable[0] = (1<<24)-1;
	colourtable[1] = 0;
	//misc
	empty.N0s = 16;
	for(int i=0; i<16; i++) empty.data[i>>2][i&3] = 0;
}
void AutomataEngine::draw(SDL_Surface *s){
	SDL_FillRect(s,NULL,(200<<16)|(200<<8)|200);
	//find out what rectangle is shown.

	//draw cells.
	int topToBottom = height/zoom+1, leftToRight = width/zoom+1;
	if(rows==-1||columns==-1){
		//unlimited space
		for(int i = pany-1; i < pany+ topToBottom; i++){
			for(int j = panx-1; j < panx + leftToRight; j++){
				SDL_Rect r = {((float)j-panx)*zoom+1,((float)i-pany)*zoom+1,zoom-1,zoom-1};
				if(zoom<5.0){
					r.w +=1; r.h +=1;
				}
				SDL_FillRect(s,&r,colourtable[get(j,i)]);
			}
		}
	}else{
		//mere mortals
		for(int i = max(pany-1,0); i < min(pany+ topToBottom,rows); i++){
			for(int j = max(panx-1,0); j < min(panx + leftToRight,columns); j++){
				SDL_Rect r = {((float)j-panx)*zoom+1,((float)i-pany)*zoom+1,zoom-1,zoom-1};
				if(zoom<5.0){
					r.w +=1; r.h +=1;
				}
				SDL_FillRect(s,&r,colourtable[get(j,i)]);
			}
		}
	}
}

void AutomataEngine::frameTick(SDL_Event *e){
	if(!paused){
		currentTick+=ticksPerFrame;
		simulate((int)(currentTick+1.0));
		currentTick -= ((int)(currentTick+1.0));
	}else{
		currentTick = 0.0;
		//cout<<ticksPerFrame<<"\n";
	}
}

//cellular data
void AutomataEngine::simulate(int steps){
	if(steps==0) return;
	//simulation test
	//will just create a new data table
	for(int i = 0; i < steps; i++) generate();
}

void AutomataEngine::generate(){
	//cout<<fieldChange.size()<<"\n";
	int dx[8] = {-1,0,1,1,1,0,-1,-1}, dy[8] = {-1,-1,-1,0,1,1,1,0};
	//tricky.
	//iterate through all framechange keys.
	//first create a temp frame.
	map <pair<short,short>,block >toChange;
	vector<pair<short,short> > toAddToChange;

	vector<pair<short,short> > toRemoveFromChange;
	vector<pair<short,short> > toRemoveFromState;
	
	for(set<pair<short,short> >::iterator i = fieldChange.begin(); 
		i != fieldChange.end(); i++){
		block *data = &empty;
		if(state.count(*i)!=0) data = &state[*i];		

		//create grid.
		char grid[6][6];
		for(int j = 0; j < 4; j++) for(int k = 0; k < 4; k++)
			grid[j+1][k+1] = data->data[j][k];
		pair<short,short> key = (*i);
		//top border
		key.first --;
		if(state.count(key)!=0) data = &state[key];
		else data = &empty;
		for(int j = 0; j < 4; j++) grid[0][j+1] = data->data[3][j];
		//bottom border
		key.first +=2;
		if(state.count(key)!=0) data = &state[key];
		else data = &empty;
		for(int j = 0; j < 4; j++) grid[5][j+1] = data->data[0][j];
		//left border
		key.first --; key.second --;
		if(state.count(key)!=0) data = &state[key];
		else data = &empty;
		for(int j = 0; j < 4; j++) grid[j+1][0] = data->data[j][3];
		//right border
		key.second +=2;
		if(state.count(key)!=0) data = &state[key];
		else data = &empty;
		for(int j = 0; j < 4; j++) grid[j+1][5] = data->data[j][0];
		//corners
		grid[0][0] = get((i->second<<2)-1,(i->first<<2)-1);
		grid[0][5] = get((i->second<<2)+4,(i->first<<2)-1);
		grid[5][0] = get((i->second<<2)-1,(i->first<<2)+4);
		grid[5][5] = get((i->second<<2)+4,(i->first<<2)+4);
		//now process grid. If nothing has changed, remove from fieldchange.
		//otherwise, add it and neighbours to toChange.
		//if completely empty, remove from state.
		bool changed = false; int nonZeros = 0;
		toChange[*i] = *data;
		data = &toChange[*i];
		for(int y = 1; y <= 4; y++){
			for(int x = 1; x <= 4; x++){
				char neighbours[8], current = grid[y][x];
				for(int j = 0; j < 8; j++) neighbours[j] = grid[y+dy[j]][x+dx[j]];
				int result = determine(current,neighbours);
				if(result!=0) nonZeros ++;
				if(result!=current) changed = true;
				data->data[y-1][x-1] = result;
			}
		}
		if(!changed) toRemoveFromChange.push_back(*i);
		else{
			//add to toChange list.
			//add neighbours
			toAddToChange.push_back(*i);
			for(int j = 0; j < 8; j++){
				toAddToChange.push_back(make_pair(i->first+dy[j],i->second+dx[j]));
			}
		}
		if(nonZeros==0) toRemoveFromState.push_back(*i);
	}
	//impliment changes.
	for(map<pair<short,short>,block>::iterator i = toChange.begin();
		i!=toChange.end(); i++){
		state[i->first] = i->second;
		//must debug.
		
	}
	for(int i = 0; i < toRemoveFromChange.size(); i++) fieldChange.erase(toRemoveFromChange[i]);
	for(int i = 0; i < toAddToChange.size(); i++) fieldChange.insert(toAddToChange[i]);
	for(int i = 0; i < toRemoveFromState.size(); i++) state.erase(toRemoveFromState[i]);
}

int AutomataEngine::determine(char current, char neighbours[8]){
	//impliment conway
	int n = 0;
	for(int i = 0; i < 8; i++) n+= neighbours[i]&1;
	if(current&1){
		//alive.
		if(n<2||n>3) return 0;
		return 1;
	}else{
		//dead
		if(n==3) return 1;
		return 0;
	}
}
//editing tools
void AutomataEngine::affect(int x, int y){
	if((x<0||x>=columns||y<0||y>=rows)&&(columns!=-1&&rows!=-1)) return;
	//locate the block
	pair<short,short> key = make_pair(y>>2,x>>2);
	map<pair<short,short>,block>::iterator i = state.find(key);
	if(i==state.end()){
		state[key] = empty;
		i = state.find(key);
	}
	//edit the block.
	block *data = &(*i).second;
	if(tool==tool_toggle){
		if(data->data[y&3][x&3]&1){
			data->data[y&3][x&3]-=1;
			if(data->data[y&3][x&3]==0) data->N0s++;
		}else{
			if(data->data[y&3][x&3]==0) data->N0s--;
			data->data[y&3][x&3]+=1;
		}
	}
	//add the block to modify list
	fieldChange.insert(key);
	fieldChange.insert(make_pair(key.first-1,key.second-1));
	fieldChange.insert(make_pair(key.first-1,key.second));
	fieldChange.insert(make_pair(key.first-1,key.second+1));
	fieldChange.insert(make_pair(key.first,key.second-1));
	fieldChange.insert(make_pair(key.first,key.second+1));
	fieldChange.insert(make_pair(key.first+1,key.second-1));
	fieldChange.insert(make_pair(key.first+1,key.second));
	fieldChange.insert(make_pair(key.first+1,key.second+1));
}

int AutomataEngine::get(int x, int y){

	if((x<0||x>=columns||y<0||y>=rows)&&(columns!=-1&&rows!=-1)) return-1;
	//locate the block
	pair<short,short> key = make_pair(y>>2,x>>2);
	map<pair<short,short>,block>::iterator i = state.find(key);
	if(i==state.end()){
		return 0;
	}
	//edit the block.
	return i->second.data[y&3][x&3];
}

void AutomataEngine::panLeft(){
	//move a quarter left.
	panx = panx-width/zoom/4.0;
}

void AutomataEngine::panRight(){
	panx = panx+width/zoom/4.0;
}
void AutomataEngine::panUp(){
	pany = pany-height/zoom/4.0;
}

void AutomataEngine::panDown(){
	pany = pany+height/zoom/4.0;
}
void AutomataEngine::zoomIn(){
	zoom = (int)min(49,zoom*1.2)+1;
}
void AutomataEngine::zoomOut(){
	zoom = (int)max(2,zoom*0.8)-1;
}

AutomataEngine::~AutomataEngine(void){
}

void downHandler(SDL_Event *e, void *p){
	AutomataEngine *s = (AutomataEngine*)p;
	//find position of mouse
	e->button.x -= s->left; e->button.y -= s->top;

	int x = (float)e->button.x / s->zoom + s->panx;
	int y = (float)e->button.y / s->zoom + s->pany;
	s->affect(x,y);
}