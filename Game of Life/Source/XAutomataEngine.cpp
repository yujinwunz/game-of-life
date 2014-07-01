#include "XAutomataEngine.h"
#include "form.h"
#define _StateOffset 50000
#define _SO _StateOffset
#include <conio.h>
#include <dos.h>
#include <fstream>
//Utility
bool checkRule(int cellTeam, int team){
	if(team == 0) return true;
	if(team != cellTeam) return false;
	return true;
	
}

XAutomataEngine::XAutomataEngine(form* parent, int x, int y, int w, int h, int columns, int rows, bool keyBound):control(parent,x,y,w,h){
	this->addHandler(event_mousebuttondown,XdownHandler);
	this->addHandler(event_mousemotion,XmoveHandler);
	this->columns = columns; this->rows = rows;

	ticksPerFrame = 1.0;
	paused = true;
	currentTick = 1.0;

	//interface
	panx = pany = 0.0;
	zoom = 10.0;
	tool = xtool_toggle;
	team = 0;
	ruleRestricted = false;
	//FILL COLOUR TABLE.
	colourtable[0] = (1<<24)-1;
	colourtable[1] = 0;
	colourtable[2] = 0xffdddd;
	colourtable[3] = 0xff0000;
	colourtable[4] = 0xddffdd;
	colourtable[5] = 0x00ff00;
	colourtable[6] = 0xddddff;
	colourtable[7] = 0x0000ff;
	colourtable[8] = 0xffff99;
	colourtable[9] = 0x999900;
	//misc
	proc = new unsigned int[5001012];
	
	collisions = 0;
	toAdds = toChanges = 0;
	toAddSize = toChangeSize = 1;
	NumInHash = 0;
	toAdd = new pair<unsigned int, char>[1];
	toChange = new unsigned int[1];
	
	generation = 0;
	//hash
	hashsizes[0] = 1009; hashsizes[1] = 10007; hashsizes[2] = 100003;
	hashsizes[3] = 1000003; hashsizes[4] = 5000011;

	hashsizeindex = 0;
	hashsize = hashsizes[hashsizeindex];
	resetHash();

	//arrow key controls
	if(keyBound){
		addHandler(event_keydown,XKeyHandler);
	}
	/*
	//experiment
	Part of loops experiment.
	colourtable[0] = (1<<24)-1;
	colourtable[1] = 0x0000bb;
	colourtable[2] = 0xff0000;
	colourtable[3] = 0x00ff00;
	colourtable[4] = 0xdddd00;
	colourtable[5] = 0xdd00dd;
	colourtable[6] = 0xffffff;
	colourtable[7] = 0x7777ff;
	colourtable[8] = 0xffff99;
	colourtable[9] = 0x999900;
	ifstream in("Loops.txt");
	for(int i = 0; i < 8; i++) for(int j  = 0; j < 8; j++) for(int k = 0; k < 8; k++) for(int l = 0; l < 8; l++) for(int m = 0; m < 8; m++) table[i][j][k][l][m] = 0;
	for(int i = 0; i < 1000; i++){
		cout<<"read";
		int a = -1, b = 0, c = 0, d = 0, n = 0, v = 0;
		in>>n>>a>>b>>c>>d>>v;
		if(a==-1) break;
		cout<<v;
		table[a][b][c][d][n] = table[b][c][d][a][n] = table[c][d][a][b][n] = table[d][a][b][c][n] = v;
	}
	in.close();
	*/
}

void XAutomataEngine::draw(SDL_Surface *s){
	SDL_FillRect(s,NULL,(200<<16)|(200<<8)|200);
	//find out what rectangle is shown.

	//draw cells.
	int topToBottom = height/zoom+1, leftToRight = width/zoom+1;
	if(rows==-1||columns==-1){
		//unlimited space
		for(int i = pany; i < pany+ topToBottom; i++){
			for(int j = panx; j < panx + leftToRight; j++){
				SDL_Rect r = {(j-panx)*zoom+1,(i-pany)*zoom+1,zoom-1,zoom-1};
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
				SDL_Rect r = {(j-panx)*zoom+1,(i-pany)*zoom+1,zoom-1,zoom-1};
				if(zoom<5.0){
					r.w +=1; r.h +=1;
				}
				SDL_FillRect(s,&r,colourtable[get(j,i)]);
			}
		}
	}
	//draw some co-ordinates if infinite
	if(columns == -1 || rows == -1){
		//top left
		drawText(intToString(panx)+","+intToString(pany),0,0,font,s,0,0,0,TopLeft);
		//bottom right
		drawText(intToString(panx+width/zoom)+","+intToString(pany+height/zoom),width-50,height-	 15,font,s,0,0,0,Centered);
	}
	//draw generation
	drawText("Generation: "+intToString(generation),10,height-30,font,s,100,100,100,TopLeft);
}

void XAutomataEngine::frameTick(SDL_Event *e){
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
void XAutomataEngine::simulate(int steps){
	if(steps==0) return;
	//simulation test
	//will just create a new data table
	for(int i = 0; i < steps; i++) generate();
}


void XAutomataEngine::generate(){
	//cout<<fieldChange.size()<<"\n";
	//cout<<collisions<<"\n";
	generation ++;
	collisions = 0;
	int dx[8] = {-1,0,1,1,1,0,-1,-1}, dy[8] = {-1,-1,-1,0,1,1,1,0};
	//tricky.
	//iterate through all framechange keys.
	toAdds = toChanges = 0;
	if((NumInHash<<3)+NumInHash>=toChangeSize){
		delete[] toChange; delete[] toAdd;
		while(toChangeSize <= (NumInHash<<3)+NumInHash) toChangeSize = (toChangeSize<<1);
		toAddSize = toChangeSize;
		toChange = new unsigned int [toChangeSize];
		toAdd = new pair<unsigned int,char> [toAddSize];
	}

	bool exitDone = false;
	for(int j = 0; j < _XhashSize+1000; j++){
		if(proc[j]==0xffffffff){
			if(exitDone) continue;	
			exitDone = true;
		}
		unsigned int *i = &proc[j];
		short first = (*i)>>16, second = (*i)&0x0000ffff;
		char current = get(second,first), n[8];
		//there must be a different way to do this

		for(int j = 0; j < 8; j++){
			n[j] = get(second+dx[j],first+dy[j]);
		}
		//cout<<"\n";
		char result = determine(current,n);
		/*
		int neighbours = state[first+_SO-1].count(second-1) + 
			 state[first+_SO-1].count(second) +
			  state[first+_SO-1].count(second+1) +
			   state[first+_SO].count(second+1) +
			    state[first+_SO+1].count(second+1) +
				 state[first+_SO+1].count(second) +
				  state[first+_SO+1].count(second-1) +
				   state[first+_SO].count(second-1) ;
		int result = current ;
		if( current){
			if(neighbours<2||neighbours>3) result = 0;
		}else if(neighbours==3) result = 1;*/
		short a = (*i)>>16, b=  (*i)&0x0000ffff;
		if(result!=current){
			//needs to be added and changed
			toAdd[toAdds++] = make_pair(*i,result);
			toChange[toChanges++] = (*i);
		}
	}
	resetHash();
	//impliment changes
	//cout<<toAdd.size()<<"\n";
	for(int i = 0; i < toAdds; i++){
		short first = toAdd[i].first>>16, second = toAdd[i].first&0x0000ffff;
		if(toAdd[i].second == 0){
			state[first+_SO].erase(second);	//KILL HIM!
		}else{
			state[first+_SO][second] = toAdd[i].second;
		}
	}
	for(int i = 0; i < toChanges; i++){
		store(toChange[i]);
		short first = toChange[i]>>16, second = toChange[i]&0x0000ffff;
		for(int j = 0; j < 8; j++){
			store((((unsigned int)((unsigned short)(first+dy[j])))<<16)|((unsigned int)((unsigned short)(second+dx[j]))));
		}
	}
}

/*
Uncomment this, uncomment the "experimental" section in the constructor, and comment out the "determine" function below to display Loops.
#include <random>
int XAutomataEngine::determine(char current, char n[8]){

	int r=  table[n[1]][n[3]][n[5]][n[7]][current];
	if((rand()&0xffffff)==1){
		if(generation>400&&((rand()&0x0007f)==1)){
		cout<<"Mutation\n";
		return abs(rand())%8;
		}
	}
	return r;
}
*/

int XAutomataEngine::determine(char current, char n[8]){
	//impliment conway
	int neighbours = 0, occur[5] = {0,0,0,0,0}, deadoccur[5] = {0,0,0,0,0};
	int bestTeam = 0, bestScore = 0, deadTeam = 0, deadScore = 0;
	bool tie = true, deadTie = true;
	for(int i = 0; i < 8; i++){
		neighbours+= n[i]&1;
		if(n[i]&1){
			occur[n[i]>>1] ++;
			if(occur[n[i]>>1] > bestScore){
				tie = false;
				bestScore = occur[n[i]>>1];
				bestTeam = (n[i]>>1);
			}else if(occur[n[i]>>1] == bestScore){
				tie = true;
			}
		}
		if(((n[i]>>1)!=0)&&!(n[i]&1)){
			deadoccur[n[i]>>1] ++;
			if(deadoccur[n[i]>>1] > deadScore){
				deadTie = false;
				deadScore = deadoccur[n[i]>>1];
				deadTeam = n[i]>>1;
			}else if(deadoccur[n[i]>>1] == deadScore){
				deadTie = true;
			}
		}
	}

	//if(tie) bestTeam = 0;
	
	if(current&1){
		//alive.
		if(neighbours<2||neighbours>3){
			if (tie){
				if(deadTie){
					return current-1;
				}
				return deadTeam<<1;
			}
			return (bestTeam<<1);
		}
		return (bestTeam<<1)+1;
	}else{
		//dead
		if(neighbours==3){
			if(tie) return 1;
			return (bestTeam<<1)+1;
		}
		if(tie){
			return current;
		}
		return (bestTeam<<1);
	}
}
//editing tools

void XAutomataEngine::fit(){
	if(columns==-1||rows==-1){
		//home
		panx = -width/(zoom<<1);
		pany = -height/(zoom<<1);
	}else{
		//fit.
		zoom = max(min(width/columns,height/rows),1);
		panx = -(width-columns*zoom)/(zoom<<1);
		pany = -(height-rows*zoom)/(zoom<<1);
	}
}

void XAutomataEngine::affect(int x, int y){
	
	if((x<0||x>=columns||y<0||y>=rows)&&(columns!=-1&&rows!=-1)) return;
	
	if(NumInHash>(hashsize>>4)) resizeTable(hashsizeindex+1);
	int value = get(x,y);
	int cellTeam = (value>>1);
	//check team rules
	if(ruleRestricted){
		if(!checkRule(cellTeam,team)) return;
	}

	if(value&1) value = team<<1;
	else value = (team<<1)+1;
	if(value==0) state[y+_SO].erase(x);
	else state[y+_SO][x] = value;

	
	int dx[8] = {-1,0,1,1,1,0,-1,-1}, dy[8] = {-1,-1,-1,0,1,1,1,0};

	unsigned short first = y, second = x;
	unsigned int key = (((unsigned int)first)<<16)|second;
	//cout<<first<<","<<second<<","<<key<<" ";
	store(key);
	for(int i = 0; i < 8; i++){
	//	cout<<"("<<dy[i]<<","<<dx[i]<<","<<(dy[i]<<16)+dx[i]<<","<<(key+(dy[i]<<16)+dx[i])<<") ";
		unsigned short first = key>>16, second = key&0x0000ffff;
		store((((unsigned int)((unsigned short)(first+dy[i])))<<16)|(unsigned int)((unsigned short)(second+dx[i])));
	}
	//cout<<"\n";
}

int XAutomataEngine::get(int x, int y){

	if((x<0||x>=columns||y<0||y>=rows)&&(columns!=-1&&rows!=-1)) return 0;
	//locate the block
	if(state[y+_SO].count(x) == 0) return 0;
	return state[y+_SO][x];
}

void XAutomataEngine::panLeft(){
	//move a quarter left.
	panx = (int)((float)panx-(float)width/(float)zoom/4.0)-1;
}

void XAutomataEngine::panRight(){
	panx = (int)((float)panx+(float)width/(float)zoom/4.0)+1;
}
void XAutomataEngine::panUp(){
	pany = (int)((float)pany-(float)height/(float)zoom/4.0)-1;
}

void XAutomataEngine::panDown(){
	pany = (int)((float)pany+(float)height/(float)zoom/4.0)+1;
}
void XAutomataEngine::zoomIn(){
	panx += width/(zoom<<1);
	pany += height/(zoom<<1);
	zoom = (int)min(50,(float)zoom*2.0);
	panx -= width/(zoom<<1);
	pany -= height/(zoom<<1);
}
void XAutomataEngine::zoomOut(){
	panx += width/(zoom<<1);
	pany += height/(zoom<<1);
	zoom = (int)max(1,(float)zoom/2.0);
	panx -= width/(zoom<<1);
	pany -= height/(zoom<<1);
}

XAutomataEngine::~XAutomataEngine(void){
	delete[] toAdd;
	delete[] toChange;
	delete[] proc;
}

void XmoveHandler(SDL_Event *e, void *p){
	XAutomataEngine *s = (XAutomataEngine*)p;
	if(!s->leftDown) return;
	//uses line rasterization to affect every pixel between start and current except start.
	e->button.x -= s->left; e->button.y -= s->top;
	Uint8 *keys = SDL_GetKeyState(NULL);
	if(keys[SDLK_LSHIFT]||keys[SDLK_RSHIFT]) return;
	

	int x0 = s->startingX, y0 = s->startingY, x1, y1;
	s->getGridCoords((signed short)e->motion.x,(signed short)e->motion.y,&x1,&y1);
	
	//Bresenham's line algorithm off Wikipedia
	int dx = abs(x1-x0), dy = abs(y1-y0), sx, sy;
	if(x0 < x1) sx = 1;
	else sx = -1;
	if(y0 < y1 ) sy = 1;
	else sy = -1;
	int err = dx-dy;
 
	while(true){
		if(x0!=s->startingX||y0!=s->startingY)s->affect(x0,y0);
		if(x0 ==x1 && y0 == y1) break;
		int e2 = (err<<1);
		if(e2 > -dy){ 
			err = err - dy;
			x0 = x0 + sx;
		}
		if (e2 <  dx){ 
			err = err + dx;
			y0 = y0 + sy;
		}
	}

	s->startingX = x1; s->startingY = y1;	//assumed that x1 and y1 isn't changed by the algorithm
}

void XdownHandler(SDL_Event *e, void *p){
	//Remaking. Now handling dragging drawing.
	XAutomataEngine *s = (XAutomataEngine*)p;
	//find position of mouse
	e->button.x -= s->left; e->button.y -= s->top;

	int x, y; s->getGridCoords(e->button.x,e->button.y,&x,&y);
	s->startingX = x; s->startingY = y;
	s->affect(x,y);
}

void XAutomataEngine::getGridCoords(int mouseX, int mouseY, int *destX, int *destY){
	*destX = mouseX / zoom + panx;
	*destY = mouseY / zoom + pany;
}

void XKeyHandler(SDL_Event *e, void*p){
	XAutomataEngine* s = (XAutomataEngine*)p;
	if(e->key.keysym.sym == SDLK_LEFT) s->panLeft();
	else if(e->key.keysym.sym == SDLK_RIGHT) s->panRight();
	else if(e->key.keysym.sym == SDLK_UP) s->panUp();
	else if(e->key.keysym.sym == SDLK_DOWN) s->panDown();
	else if(e->key.keysym.sym == SDLK_PAGEUP) s->zoomIn();
	else if(e->key.keysym.sym == SDLK_PAGEDOWN) s->zoomOut();
}