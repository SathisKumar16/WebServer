#include<stdio.h>
#include<stdlib.h>
struct Route{

	char* key;
	char *value;
	struct Route *left, *right;

};

struct Route * initroute(char* key,char* value);

struct Route * add(struct Route * root,char* key,char* value);

struct Route * search(struct Route * root,char* key);

void inorder(struct Route * root);
