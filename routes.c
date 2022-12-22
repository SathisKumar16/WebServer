#include<stdio.h>
#include<stdlib.h>
#include<string.h>
struct Route {
	char*  key;
	char* value;

	struct Route *left,*right;
};

struct Route * init(char* key,char* value);
struct Route * add(struct Route * root,char* key,char* value);
struct Route *search(struct Route * root,char* key);
void inorder(struct Route * root);
struct Route * init(char* key,char* value){
	struct Route *temp=(struct Route*)malloc(sizeof(struct Route));
	temp->key=key;
	temp->value=value;
	temp->left=NULL;
	temp->right=NULL;
}

struct Route * add(struct Route * root,char* key,char* value){
	if(root==NULL)
		return init(key,value);
	if(strcmp(key,root->key)==0)
		printf("\n %s Already Exists\n",key);
	else if(strcmp(key,root->key)>0)
		root->right=add(root->right,key,value);
	else
		root->left=add(root->left,key,value);
}

struct Route * search(struct Route * root,char* key){
	if(root==NULL)
		return NULL;
	else if(strcmp(key,root->key) == 0)
		return root;
	else if(strcmp(key,root->key)>0)
		return search(root->right,key);
	return search(root->left,key);
}
void inorder(struct Route * root)
{
	if(root!=NULL){
		inorder(root->left);
		printf("%s-%s",root->key,root->value);
		inorder(root->right);
	}

}
