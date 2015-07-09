#ifndef __INTERCODE_H__
#define __INTERCODE_H__

#include "tree.h"

struct Operand 
 { 
 	enum { VARIABLE_OP, CONSTANT_OP, ADDRESS_OP, ARRAY_OP} kind; 
 	union 
 	{
  		int value;
 		char* name;
 	} u;
 };

typedef struct Operand Operand; 

struct InterCode
{
	enum {ASSIGN_IC, ADD_IC, SUB_IC, MUL_IC, DIV_IC, MINUS_IC, RETURN_IC, 
		LABEL_IC, LABEL_GOTO_IC, READ_IC, WRITE_IC, CALLFUNC_IC, 
		FUNCTION_IC, ARG_IC, PARAM_IC, REFASSIGN_IC, DEC_IC, COND_IC} kind;

	union 
	{
		struct { char* name; } function;
		struct { char* name; } param;
		struct { char* name; } label;
		struct { char* name; } label_goto;
		struct { Operand* op; char* name; } call;
		struct { Operand* op; } ret;
		struct { Operand* op; } write;
		struct { Operand* op; } read;
		struct { Operand* op; } arg;
		struct { Operand* op; int size;} dec;
		struct { Operand* result; } sinop; 
		struct { Operand* result, *op; } binop;
		struct { Operand* result, *op1, *op2; } triop;
		struct { Operand* op1, *op2; char* op, *name; }cond;
	}u;

};

typedef struct InterCode InterCode;

struct LinkedInterCode
{
	struct InterCode* code;
	struct LinkedInterCode* prev;
	struct LinkedInterCode* next;
};



typedef struct LinkedInterCode LinkedInterCode;

LinkedInterCode* bindCode(LinkedInterCode*, LinkedInterCode*);
void writeCode(FILE* f, InterCode* ic);
void printInFile(char* filename);

void translateCond(Node* node, char*, char*);
//Function.
void translateProgram(struct Node*);
void translateExtDefList(struct Node*);
void translateExtDef(struct Node*);
void translateExtDecList(struct Node*);

int translateSpecifier(struct Node*);
void translateStructSpecifier(struct Node*);
void translateOptTag(struct Node*);
void translateTag(struct Node*);

char* translateVarDec(struct Node*, int);
void translateFunDec(struct Node*);
void translateVarList(struct Node*);
void translateParamDec(struct Node*);

void translateCompSt(struct Node*);
void translateStmtList(struct Node*);
void translateStmt(struct Node*);

void translateDefList(struct Node*);
void translateDef(struct Node*);
void translateDecList(struct Node*, int);
void translateDec(struct Node*, int);

void translateExp(struct Node*, Operand*);
void translateArgs(struct Node*, Operand**);

#endif