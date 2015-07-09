#ifndef __TREE_H__
#define __TREE_H__
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
extern int yylineno;
extern int errorA;
struct Node
{
	unsigned int lineno;
	char* name;
	char* value;
	struct Node** sons;
	unsigned int sonNum;
	unsigned int isLeaf;
};

typedef struct Node Node;

struct displayNode
{
	struct Node* disNode;
	unsigned int layer;
	struct displayNode* next;
};


struct Node* createNode(char*, char*, unsigned int, unsigned int, unsigned int, ...);

void displayTree(struct Node*);

#endif
