#include "SDLGUI.h"
#include "AutomataEngine.h"
#include "sandbox.h"
#undef main
using namespace std;



extern TTF_Font *DefaultFont;
SDL_Surface *screen;



bool init(){	//This is the initializer called by main which sets up the environment for SDL and truetype fonts.
	if(SDL_Init(SDL_INIT_EVERYTHING)==-1 || TTF_Init() == -1){
		cout<<"Error initializing.\n";
		return true;
	};
	SDL_EnableKeyRepeat(500,25);
	//make default font a free font
	DefaultFont = TTF_OpenFont("LiberationSans-Bold.ttf",16);	//Decides on a default font.
	return 0;
}

static bool running = true;
void Quit(SDL_Event *e,void* p){
	running = false;
}

struct menuTag{
	int *location;
	int value;
};

void menuClick(SDL_Event *e, void *p){
	Button *s = (Button*)p;
	menuTag *me = (menuTag*)s->tag;
	*me->location = me->value;
}

int menuResult(SDL_Surface *s, pair<string,int> *option, int options, string title = ""){
	form menu(0,0,1000,600);
	SDL_Colour background = {0,20,70,255};
	//checkBox c(&menu,100,100,100,100); c.setText( "LOL");
	menu.backColour = menu.foreColour = background;

	menuTag e[10]; Button *b[10];
	int ret = -1;

	for(int i = 0; i < options; i++){
		b[i] = new Button(&menu,400,((title=="")?300:400)-options*30-25+i*60,200,50);
		b[i]->text = option[i].first;
		e[i].location = &ret; e[i].value = option[i].second;
		b[i]->tag = (void*)(&e[i]);
		b[i]->addHandler(event_Lclick,menuClick);

		//style!
		SDL_Color c1 = {10,50,150,255}, c2 = {50,100,255,255};
		b[i]->backColour = c1; b[i]->foreColour = c2;
		if(option[i].second == -1) b[i]->enabled = false;
	}

	//title
	label *titleLabel = NULL;
	TTF_Font *lf = TTF_OpenFont("LiberationSans-Bold.ttf",50);
	if(title!=""){
		titleLabel = new label(&menu,0,50,1000,80);
		titleLabel->text = title;
		titleLabel->setTextColour(255,255,255);
		titleLabel->setFont(lf);
	}
	
	menu.startFramerate(60,s);
	while(ret == -1){
		menu.wait();
		SDL_Flip(s);
	}
	//clean up
	for(int i = 0; i < options; i++) delete b[i];
	if(titleLabel != NULL) delete titleLabel;
	TTF_CloseFont(lf);
	return ret;
}

void main(){
	cout<<0xffffffff<<"\n";
	if(init()) return;
	screen = SDL_SetVideoMode(1000,600,32,SDL_DOUBLEBUF|SDL_HWSURFACE);
	
	
	//main menu loop
	while(running){
		pair<string,int> p[4] = {make_pair("Play",-1),make_pair("Sandbox",2),make_pair("Options",-1),make_pair("Quit",4)};
		int result = menuResult(screen,p,4,"Main menu");
		if(result == 4) return ;
		else if(result == 2){
			//sandbox
			int width, height;
			pair <string,int> s[6] = {make_pair("10x10",1),make_pair("50x50",3),
				make_pair("100x50",4),make_pair("500x250",6),
				make_pair("1500x1500",7),make_pair("Infinite",8)};
			int w[9] = {-2,10,20,50,100,200,500,1500,-1};
			int y[9] = {-2,10,10,50,50,200,250,1500,-1};
			int condition = menuResult(screen,s,6,"Select a size");
			sandbox(screen,w[condition],y[condition]);
		}else if(result == 1){
			//play
			pair <string,int> s[6] = {make_pair("Singleplayer",1),make_pair("Local",2),make_pair("Online",3)};
		}else if(result==3){
			//options

		}
	}
}