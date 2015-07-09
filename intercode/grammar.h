#ifndef __GRAMMAR_H__
#define __GRAMMAR_H__

#include "tree.h"
#define HASHTABLELENGTH 15

typedef struct Type_ Type_;
typedef struct FieldList_ FieldList_;
typedef struct Var_ Var_;
typedef struct Func_ Func_;
typedef struct Args_ Args_;
typedef struct Item_ Item_;
typedef struct ExpReturnType_ ExpReturnType_;

struct Type_
{
	enum { BASIC, ARRAY, STRUCTURE } kind;
	union
	{
		int basic;//int or float
		struct {Type_* elem; int size; } array;
		FieldList_* structure;
	}u;
};

struct FieldList_
{
	char* name;
	Type_* type;
	FieldList_* tail;
};

struct Var_
{
	char* name;
	Type_* type;
};

struct Args_
{
	Var_* v;
	Args_* next;
};

struct Func_
{
	char* name;
	Type_* returnType;
	//int argNum;
	Args_* args;
};

struct Item_
{
	char* name;
	int kind;
	union
	{
		Var_* v;
		Func_* f;
		FieldList_* fl;
		Type_* type;
	}u;
	Item_*  next;
};

struct ExpReturnType_
{
	int kind;
	Type_* type;
	int flag;
};



typedef Type_* Type;
typedef FieldList_* FieldList;
typedef Var_* Var;
typedef Func_* Func;
typedef Args_* Args;
typedef Item_* Item;
typedef ExpReturnType_* ExpReturnType;

unsigned int hash_pjw(char*);

int initHashTable();

int insertInHashTable(Item);

int checkIfUsed(char*);

Item getFromHashTable(char*);


//Function.
void program(struct Node*);
void extDefList(struct Node*);
void extDef(struct Node*);
void extDecList(struct Node*, Type);

Type specifier(struct Node*);
Type structSpecifier(struct Node*);
Item optTag(struct Node*);
void tag(struct Node*);

Var varDec(struct Node*);
Func funDec(struct Node*);
Args varList(struct Node*);
Args paramDec(struct Node*);

void compSt(struct Node*);
void stmtList(struct Node*);
void stmt(struct Node*);

FieldList defList(struct Node*, int);
FieldList def(struct Node*, int);
FieldList decList(struct Node*, Type, int);
FieldList dec(struct Node*, Type, int);

ExpReturnType_ exp(struct Node*);
int args(struct Node*, Args);


void printTable();

#endif