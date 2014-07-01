class DynamicHash{
	void clean();
	void resize(int size);
	int dimension; int sizes[10]; int sizei; int resizes;
	int delimiter, occupied;
	void wipe();
public:
	char *values;
	unsigned int *keys;
	void clear();
	void insert(unsigned int key, char value);
	char get(unsigned int key);
	void remove(unsigned int key);

	int items, collisions;
	int hash(unsigned int key);

	DynamicHash();
	~DynamicHash();
};