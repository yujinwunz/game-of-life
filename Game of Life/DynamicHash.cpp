#include "DynamicHash.h"

void DynamicHash::clean(){
	//stores all keys and values
	int index = 0; char* tempValues = new char[items];
	unsigned int *tempKeys = new unsigned int[items];

	for(int i = 0; i < dimension + 1000; i++){
		if(keys[i]!=delimiter){
			tempKeys[items] = keys[i];
			tempValues[items++] = values[i];
		}
	}

	wipe();
	//restore original hash
	for(int i = 0; i < dimension + 1000; i++){
		insert(keys[i],values[i]);
	}

	delete[] tempKeys; delete[] tempValues;
}

void DynamicHash::wipe(){
	for(int i = 0; i < dimension + 1000; i++) keys[i] = delimiter;
	items = collisions = 0;
}

void DynamicHash::resize(int newi){
	if(newi == sizei) return;
	if(newi >= 10 || newi < 0) return;

	if(newi > sizei && resizes>(sizes[newi]>>4)){
		//larger.
		//store all existing keys and values.
		int index = 0; char* tempValues = new char[items];
		unsigned int *tempKeys = new unsigned int[items];

		for(int i = 0; i < dimension + 1000; i++){
			if(keys[i]!=delimiter){
				tempKeys[items] = keys[i];
				tempValues[items++] = values[i];
			}
		}

		//reallocate memory
		delete[] keys; delete[] values;
		keys = new unsigned int [1000 + (dimension = sizes[newi])];
		values = new char[dimension + 1000];
		wipe();

		//restore original hash
		for(int i = 0; i < items; i++){
			insert(keys[i],values[i]);
		}

		resizes = 0;
		delete[] tempKeys; delete[] tempValues;
	}else{
		//smaller
		resizes++;
	}
}


void DynamicHash::clear(){
	if(sizei > 1){
		delete[] values; delete[] keys;
		dimension = sizes[sizei = 0];
		values = new char[dimension + 1000];
		keys = new unsigned int[dimension + 1000];
	}
	wipe();
}
void DynamicHash::insert(unsigned int key, char value){
	int index = hash(key);
	while(keys[index]!=delimiter&&keys[index]!=key) index ++;
	if(keys[index]!=key){
		if(items>(dimension<<16)) resize(sizei+1);
		items++;
	}

	keys[index] = key;
	values[index] = value;
}
char DynamicHash::get(unsigned int key){
	int index = hash(key);
	while(keys[index]!=delimiter&&keys[index]!=key) index ++;
	if(keys[index]!=key) return 0;	//error not found
	return values[index];
}
void DynamicHash::remove(unsigned int key){
	int index = hash(key);
	while(keys[index]!=delimiter&&keys[index]!=key) index ++;
	if(keys[index]!=key) return;
	items--;
	keys[index] = occupied;
	if(collisions++>(dimension>>5)) clean();
}

DynamicHash::DynamicHash(){
	dimension = sizes[sizei= 0 ];
	collisions = items = 0;
	resizes = 0;
	//allocate memory
	values = new char[dimension + 1000];
	keys = new unsigned int[dimension + 1000];
}

DynamicHash::~DynamicHash(){
	delete[] values;
	delete[] keys;
}

int DynamicHash::hash(unsigned int val){
	int a =( val>>16), b = val&0x0000ffff;
	unsigned int r = a*30000+b*2+b*10231+a*3;
	return  r%dimension;
}