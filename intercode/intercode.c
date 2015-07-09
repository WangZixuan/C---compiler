#include "intercode.h"
#include "string.h"
#include "assert.h"
#include "grammar.h"
#include "stdlib.h"
#define MAXSIZE 64

LinkedInterCode* head = 0;
LinkedInterCode* tail = 0;

typedef struct FieldList_* FieldList;

int labelNo = 1;
int varNo = 1;

int sizeType(Type type)
{
	FieldList fl;
	switch(type->kind)
	{
		case BASIC: return 4;
		case ARRAY: return sizeType(type->u.array.elem) * type->u.array.size;
		case STRUCTURE:

			fl = type->u.structure;
			int size = 0;
			for(; fl != 0; fl = fl->tail)
				size += sizeType(fl->type);
			return size;
		default:
			printf("Unknown type...\n");
			exit(-1);
	}
}

char* newLabel()
{
	char* temp = (char*)malloc(MAXSIZE);
	sprintf(temp, "label%d", labelNo);
	labelNo++;
	return temp;
}

Operand* newOperand()
{
	//You need to free it after using it!
	Operand* op = (Operand*)malloc(sizeof(Operand));
	return op;
}

Operand* newVar()
{
	Operand* temp = (Operand*)malloc(sizeof(Operand));
	temp->kind = VARIABLE_OP;
	temp->u.name = (char*)malloc(MAXSIZE);
	sprintf(temp->u.name, "t%d", varNo);
	varNo++;
	return temp;
}

InterCode* newInterCode()
{
	//You need to free it after using it!
	InterCode* ic = (InterCode*)malloc(sizeof(InterCode));
	return ic;
}

LinkedInterCode* bindCode(LinkedInterCode* lic1, LinkedInterCode* lic2)
{
	lic1->next = lic2;
	lic2->next = 0;
	lic1->prev = 0;
	lic2->prev = lic1;
	return lic1;
}

LinkedInterCode* insertLink(InterCode* ic)
{
	LinkedInterCode* lic = (LinkedInterCode*)malloc(sizeof(struct LinkedInterCode));
	lic->code = ic;
	lic->prev = 0;
	lic->next = 0;
	if(head == 0)
	{
		head = lic;
		tail = lic;
	}
	else
	{
		tail->next = lic;
		lic->prev = tail;
		tail = lic;
		tail->next = 0;//head?
		head->prev = 0;//tail? Loop linkedlist?
	}
	return lic;
}

void translateArray(Node* node, Type type, Type tp, Operand* last, Operand* temp)
{
	Node* q = node->sons[0];
	
	Type t;

	int tpSize = 1;
	for(t=tp->u.array.elem;t->u.array.elem!=0;t=t->u.array.elem)
		tpSize *= t->u.array.size;

	Operand* t1 = newVar();
	translateExp(node->sons[2], t1);

	InterCode* ic = newInterCode();
	ic->kind = MUL_IC;
	ic->u.triop.result = t1;
	ic->u.triop.op1 = newOperand();
	ic->u.triop.op1->kind = CONSTANT_OP;
	ic->u.triop.op1->u.value = 4;
	ic->u.triop.op2 = t1;
	insertLink(ic);

	if(tpSize > 1)
	{	
		printf("Array here...\n");
		ic = newInterCode();
		ic->kind = MUL_IC;
		ic->u.triop.result = t1;
		ic->u.triop.op1 = newOperand();
		ic->u.triop.op1->kind = CONSTANT_OP;
		ic->u.triop.op1->u.value = tpSize;
		ic->u.triop.op2 = t1;
		insertLink(ic);			
	}	

	if(last != 0)
	{
		ic = newInterCode();
		ic->kind = ADD_IC;
		ic->u.triop.result = t1;
		ic->u.triop.op1 = t1;
		ic->u.triop.op2 = last;
		insertLink(ic);			
	}

	if(strcmp(q->sons[0]->name,"ID")==0)
	{
		ic = newInterCode();
		ic->kind = ADD_IC;
		ic->u.triop.result = temp;
		ic->u.triop.op1 = newOperand();
		ic->u.triop.op1->kind = ARRAY_OP;
		ic->u.triop.op1->u.name = strdup(q->sons[0]->value);
		ic->u.triop.op2 = t1;
		insertLink(ic);
	}
	else
	{
		//Find previous
		for(t=type;t->u.array.elem!=tp;t=t->u.array.elem);
		translateArray(q,type,t,t1,temp);
	}
}

void translateCond(Node* node, char* labelTrue, char* labelFalse)
{
	//node is Exp.
	//printf("In COND\n");
	assert(strcmp(node->name, "Exp") == 0);

	if (strcmp(node->sons[1]->name, "RELOP") == 0)
	{
		//printf("Here am I\n");
		Operand* t1 = newVar();
		Operand* t2 = newVar();
		translateExp(node->sons[0], t1);
		translateExp(node->sons[2], t2);

		InterCode* ic = newInterCode();
		ic->kind = COND_IC;
		ic->u.cond.op1 = t1;
		ic->u.cond.op2 = t2;
		ic->u.cond.op = strdup(node->sons[1]->value);
		ic->u.cond.name = strdup(labelTrue);
		insertLink(ic);

		ic = newInterCode();
		ic->kind = LABEL_GOTO_IC;
		ic->u.label_goto.name = strdup(labelFalse);
		insertLink(ic);
	}
	else if (strcmp(node->sons[0]->name, "NOT") == 0)
	{
		translateCond(node->sons[1], labelFalse, labelTrue);
	}
	else if (strcmp(node->sons[1]->name, "AND") == 0)
	{
		char* label1 = newLabel();

  		translateCond(node->sons[0], label1, labelFalse);

  		InterCode* ic = newInterCode();
  		ic->kind = LABEL_IC;
  		ic->u.label.name = strdup(label1);
  		insertLink(ic);

  		translateCond(node->sons[2], labelTrue, labelFalse);
  		free(label1);
	}
	else if (strcmp(node->sons[1]->name, "OR") == 0)
	{
		char* label1 = newLabel();

  		translateCond(node->sons[0], labelTrue, label1);

  		InterCode* ic = newInterCode();
  		ic->kind = LABEL_IC;
  		ic->u.label.name = strdup(label1);
  		insertLink(ic);

  		translateCond(node->sons[2], labelTrue, labelFalse);
  		free(label1);
	}
	else
	{
		Operand* t1 = newVar();
		translateExp(node, t1);

		InterCode* ic = newInterCode();
		ic->kind = COND_IC;
		ic->u.cond.op1 = t1;
		ic->u.cond.op2 = newOperand();
		ic->u.cond.op2->kind = CONSTANT_OP;
		ic->u.cond.op2->u.value = 0;
		ic->u.cond.op = strdup("!=");
		ic->u.cond.name = strdup(labelTrue);
		insertLink(ic);

		ic = newInterCode();
		ic->kind = LABEL_GOTO_IC;
		ic->u.label_goto.name = strdup(labelFalse);
		insertLink(ic);
	}
	return;
}

void translateProgram(Node* node)
{
	//printf("In Program\n");
	assert(strcmp(node->name, "Program") == 0);
	translateExtDefList(node->sons[0]);
	return;
}

void translateExtDefList(Node* node)
{
	//printf("In ExtDefList\n");
	assert(strcmp(node->name, "ExtDefList") == 0);
	if (0 == node->sonNum)
		return;
	else
	{
		translateExtDef(node->sons[0]);
		if (strcmp(node->sons[1]->name, "") != 0)
            translateExtDefList(node->sons[1]);
		return;
	}
}

void translateExtDef(Node* node)
{
	//printf("In ExtDef\n");
	assert(strcmp(node->name, "ExtDef") == 0);
	if (3 == node->sonNum && strcmp(node->sons[1]->name, "FunDec") == 0)
	{
		translateFunDec(node->sons[1]);
		translateCompSt(node->sons[2]);
	}
	return;
}

int translateSpecifier(Node* node)
{
	//printf("In Specifier\n");
	assert(strcmp(node->name, "Specifier") == 0);
	if (strcmp(node->sons[0]->name, "TYPE") == 0)
	{
		//Never float.
		return 4;
	}
	else
	{
		//TODO
		assert(strcmp(node->sons[0]->name, "StructSpecifier") == 0);
		printf("I can not do this, Mr. Struct.\n");

		Node* structNode = node->sons[0];
		//Deal with tag;
		if (strcmp(structNode->sons[1]->name, "Tag") == 0)
		{
			//printf("struct name is %s\n", structNode->sons[1]->sons[0]->value);
			Item i = getFromHashTable(structNode->sons[1]->sons[0]->value);
			int size = 0;
			FieldList fl = i->u.fl;
			for(; fl != 0; fl = fl->tail)
				size += sizeType(fl->type);
		}
	}
}

char* translateVarDec(Node* node, int size)
{
	//printf("In VarDec\n");
	assert(strcmp(node->name, "VarDec") == 0);
	if (node->sonNum == 1)
	{
		//ID.
		//printf("sonNum=1\n");
		char* id = node->sons[0]->value;
		if (size > 4)
		{
     
			InterCode* ic = newInterCode();
			ic->kind = DEC_IC;
			ic->u.dec.op = newOperand();
			ic->u.dec.op->kind = ARRAY_OP;
			ic->u.dec.op->u.name = strdup(node->sons[0]->value);
			ic->u.dec.size = size;
			insertLink(ic);

		}
		return id;
	}
	else
	{
		//VarDec LB INT RB
		assert(node->sonNum == 4);
		int n = strtol(node->sons[2]->value, 0, 10);
		//printf("%c\n", node->sons[2]->value);
		size *= n;
		//printf("!!!%d %d\n", size, n);
		return translateVarDec(node->sons[0], size);
	}
}

void translateFunDec(Node* node)
{
	//printf("In FunDec\n");
	//ID LP VarList RP
  	//ID LP RP
  	assert(strcmp(node->name, "FunDec") == 0);

  	InterCode* ic = newInterCode();
  	ic->kind = FUNCTION_IC;
  	ic->u.function.name = strdup(node->sons[0]->value);
  	insertLink(ic);

  	if (node->sonNum == 4)
  		translateVarList(node->sons[2]);
  	return;

}

void translateVarList(Node* node)
{
	//printf("In VarList\n");
	assert(strcmp(node->name, "VarList") == 0);
	//ParamDec COMMA VarList
  	//ParamDec
  	translateParamDec(node->sons[0]);
  	if (node->sonNum == 3)
  		translateVarList(node->sons[2]);
}

void translateParamDec(Node* node)
{
	//printf("In ParamDec\n");
	assert(strcmp(node->name, "ParamDec") == 0);
	//Specifier VarDec
	char* id  = translateVarDec(node->sons[1], 0);

	InterCode* ic = newInterCode();
	ic->kind = PARAM_IC;
	ic->u.param.name = strdup(id);
	insertLink(ic);
	return;
}

void translateCompSt(Node* node)
{
	////printf("In CompSt, %d--%s--\n", node->sonNum, node->sons[1]->name);
	//LC DefList StmtList RC
	assert(strcmp(node->name, "CompSt") == 0);
	translateDefList(node->sons[1]);
	//printf("Next is here\n");
	translateStmtList(node->sons[2]);
	return;
}

void translateStmtList(Node* node)
{
	//printf("In StmtList\n");
	
	//empty
  	//Stmt StmtList
	assert(strcmp(node->name, "StmtList") == 0);
	if (node->sonNum == 0)
		return;
	else
	{
		//printf("that's why %s  %s  %s\n", node->sons[0]->name, node->sons[0]->value,node->sons[1]->name);
		assert(node->sonNum == 2);
		//if (strcmp(node->sons[0]->name, "Stmt") == 0)
		translateStmt(node->sons[0]);
		if (strcmp(node->sons[1]->name, "") != 0)
            translateStmtList(node->sons[1]);
		return;
	}
}

void translateStmt(Node* node)
{
	//printf("In Stmt\n");
	assert(strcmp(node->name, "Stmt") == 0);
	//Exp SEMI
  	//CompSt
  	//RETURN Exp SEMI
  	//IF LP Exp RP Stmt
  	//IF LP Exp RP Stmt ELSE Stmt
  	//WHILE LP Exp RP Stmt
  	if (strcmp(node->sons[0]->name, "Exp") == 0)
  		translateExp(node->sons[0], 0);
  	else if (strcmp(node->sons[0]->name, "CompSt") == 0)
  		translateCompSt(node->sons[0]);
  	else if (strcmp(node->sons[0]->name, "RETURN") == 0)
  	{
  		//TODO
  		Operand* t1 = newVar();

  		translateExp(node->sons[1], t1);

  		InterCode* ic = newInterCode();
  		ic->kind = RETURN_IC;
  		ic->u.ret.op = t1;
  		insertLink(ic);
  	}
  	else if (strcmp(node->sons[0]->name, "WHILE") == 0)
  	{
  		char* label1 = newLabel();
  		char* label2 = newLabel();
  		char* label3 = newLabel();

  		InterCode* ic = newInterCode();
  		ic->kind = LABEL_IC;
  		ic->u.label.name = strdup(label1);
  		insertLink(ic);

  		translateCond(node->sons[2], label2, label3);

  		//Label 2
  		ic = newInterCode();
  		ic->kind = LABEL_IC;
  		ic->u.label.name = strdup(label2);
  		insertLink(ic);

  		translateStmt(node->sons[4]);

  		//Go to label 1
  		ic = newInterCode();
  		ic->kind = LABEL_GOTO_IC;
  		ic->u.label_goto.name = strdup(label1);
  		insertLink(ic);

  		//Label 3
  		ic = newInterCode();
  		ic->kind = LABEL_IC;
  		ic->u.label.name = strdup(label3);
  		insertLink(ic);

  		free(label1);
  		free(label2);
  		free(label3);

  	}
  	else
  	{
  		//IF LP Exp RP Stmt
  		//IF LP Exp RP Stmt ELSE Stmt
  		assert(strcmp(node->sons[0]->name, "IF") == 0);
  		if (node->sonNum == 5)
  		{
  			char* label1 = newLabel();
	  		char* label2 = newLabel();

	  		translateCond(node->sons[2], label1, label2);

	  		//Label1
	  		InterCode* ic = newInterCode();
	  		ic->kind = LABEL_IC;
	  		ic->u.label.name = strdup(label1);
	  		insertLink(ic);

	  		translateStmt(node->sons[4]);

	  		//Label 2
	  		ic = newInterCode();
	  		ic->kind = LABEL_IC;
	  		ic->u.label.name = strdup(label2);
	  		insertLink(ic);

	  		free(label1);
	  		free(label2);
  		}
  		else
  		{
  			assert(node->sonNum == 7);

  			char* label1 = newLabel();
	  		char* label2 = newLabel();
	  		char* label3 = newLabel();

	  		translateCond(node->sons[2], label1, label2);

	  		//Label1
	  		InterCode* ic = newInterCode();
	  		ic->kind = LABEL_IC;
	  		ic->u.label.name = strdup(label1);
	  		insertLink(ic);

	  		translateStmt(node->sons[4]);

	  		//Goto label3
	  		ic = newInterCode();
	  		ic->kind = LABEL_GOTO_IC;
	  		ic->u.label_goto.name = strdup(label3);
	  		insertLink(ic);

	  		//Label 2
	  		ic = newInterCode();
	  		ic->kind = LABEL_IC;
	  		ic->u.label.name = strdup(label2);
	  		insertLink(ic);

	  		translateStmt(node->sons[6]);
	  		ic = newInterCode();
	  		ic->kind = LABEL_IC;
	  		ic->u.label.name = strdup(label3);
	  		insertLink(ic);

	  		free(label1);
	  		free(label2);
	  		free(label3);

  		}
  	}
  	return;
}

void translateDefList(Node* node)
{
	if (strcmp(node->name, "") == 0)
		return;
	//printf("In DefList\n");
	//empty
	//Def DefList
	assert(strcmp(node->name, "DefList") == 0);
	if (node->sonNum == 0)
		return;
	else
	{
		assert(node->sonNum == 2);
		//printf("here is it, %s, %s\n", node->sons[0]->name, node->sons[1]->name);
		translateDef(node->sons[0]);
		//printf("Maybe here?\n");
		if (strcmp(node->sons[1]->name, "") != 0)
            translateDefList(node->sons[1]);
		return;
	}
}

void translateDef(Node* node)
{
	//printf("In Def\n");
	//Specifier DecList SEMI
	assert(strcmp(node->name, "Def") == 0);
	int size = translateSpecifier(node->sons[0]);
	//printf("^^^^^^^%d\n", size);
	translateDecList(node->sons[1], size);
	return;
}

void translateDecList(Node* node, int size)
{
	//printf("In DecList\n");
	assert(strcmp(node->name, "DecList") == 0);
	//Dec
 	//Dec COMMA DecList
 	if (node->sonNum == 1)
 		translateDec(node->sons[0], size);
 	else
 	{
 		assert(node->sonNum == 3);
 		translateDec(node->sons[0], size);
 		translateDecList(node->sons[2], size);
 	}
 	return;
}

void translateDec(Node* node, int size)
{
	//printf("In Dec");
	//VarDec
  	//VarDec ASSIGNOP Exp
  	assert(strcmp(node->name, "Dec") == 0);
  	char* v = translateVarDec(node->sons[0], size);
  	//printf("woo is %s\n", v);

  	if (node->sonNum == 3)
  	{
  		Operand* t1 = newVar();

  		translateExp(node->sons[2], t1);

  		//Assign.

  		InterCode* ic = newInterCode();
  		ic->kind = ASSIGN_IC;
  		ic->u.binop.result = newOperand();
  		ic->u.binop.result->kind = VARIABLE_OP;
  		//printf("here???\n");
  		ic->u.binop.result->u.name = strdup(v);
  		//printf("get here?\n");
  		ic->u.binop.op = t1;
  		printf("ASS in Dec, %d, %d\n", ic->u.binop.result->kind, ic->u.binop.op->kind);
  		insertLink(ic);
  	}
  	//printf("Dec over\n");
  	return;

}

void translateExp(Node* node, Operand* op)
{
	//printf("In Exp\n");
	//Exp ASSIGNOP Exp
  	//Exp AND Exp
  	//Exp OR Exp
  	//Exp RELOP Exp
  	//Exp PLUS Exp
  	//Exp MINUS Exp
  	//Exp STAR Exp
  	//Exp DIV Exp
  	//LP Exp RP
  	//MINUS Exp
  	//NOT Exp
  	//ID LP Args RP
  	//ID LP RP
  	//Exp LB Exp RB
  	//Exp DOT ID
  	//ID
  	//INT
  	//FLOAT
	assert(strcmp(node->name, "Exp") == 0);

	if (strcmp(node->sons[0]->name, "Exp") == 0)
	{
		if (strcmp(node->sons[1]->name, "ASSIGNOP") == 0)
		{
			Item it = getFromHashTable(node->sons[1]->value);
			if (it == 0)
			{
				Operand* t1 = newVar();
			
				translateExp(node->sons[2], t1);
				
				//Assign.
				InterCode* ic = newInterCode();
				ic->kind = ASSIGN_IC;
				ic->u.binop.result = newOperand();
				ic->u.binop.result->kind = VARIABLE_OP;
				ic->u.binop.result->u.name = strdup(node->sons[0]->sons[0]->value);
				ic->u.binop.op = t1;
				//printf("ASS in Exp assop 1, %d, %d\n", ic->u.binop.result->kind, ic->u.binop.op->kind);
				insertLink(ic);

				if (op != 0)
				{
					ic = newInterCode();
					ic->kind = ASSIGN_IC;
					ic->u.binop.result = op;
					ic->u.binop.op = newOperand();
					ic->u.binop.op->kind = VARIABLE_OP;
					//Here node->sons[2]?
					ic->u.binop.op->u.name = strdup(node->sons[2]->sons[0]->value);
					//printf("ASS in Exp assop 2, %d, %d\n", ic->u.binop.result->kind, ic->u.binop.op->kind);
				
					insertLink(ic);
				}
			}
			else if( it!=0 && it->kind==ARRAY)
			{
				printf("I'm here...\n");
				Operand* t1 = newVar();
				Operand* t2 = newVar();

				translateExp(node->sons[0], t1);
				translateExp(node->sons[2], t2);
				InterCode* ic = newInterCode();
				ic->kind = ASSIGN_IC;
				ic->u.binop.result = t1;
				ic->u.binop.op = t2;
				insertLink(ic);

				if (op != 0)
				{
					ic = newInterCode();
					ic->kind = ASSIGN_IC;
					ic->u.binop.result = op;
					ic->u.binop.op = newOperand();
					ic->u.binop.op->kind = VARIABLE_OP;
					ic->u.binop.op->u.name = strdup(node->sons[2]->sons[0]->value);
				}
			}
		}
		else if (strcmp(node->sons[1]->name, "PLUS") == 0 ||
				 strcmp(node->sons[1]->name, "MINUS") == 0 ||
				 strcmp(node->sons[1]->name, "STAR") == 0 ||
				 strcmp(node->sons[1]->name, "DIV") == 0)
		{
			Operand* t1 = newVar();
			Operand* t2 = newVar();

			translateExp(node->sons[0], t1);
			translateExp(node->sons[2], t2);

			InterCode* ic;

			if (strcmp(node->sons[1]->name, "PLUS") == 0)
			{
				ic = newInterCode();
				ic->kind = ADD_IC;
				ic->u.triop.result = op;
				ic->u.triop.op1 = t1;
				ic->u.triop.op2 = t2;
				insertLink(ic);
			}
			else if (strcmp(node->sons[1]->name, "MINUS") == 0)
			{
				ic = newInterCode();
				ic->kind = SUB_IC;
				ic->u.triop.result = op;
				ic->u.triop.op1 = t1;
				ic->u.triop.op2 = t2;
				insertLink(ic);
			}
			else if (strcmp(node->sons[1]->name, "STAR") == 0)
			{
				ic = newInterCode();
				ic->kind = MUL_IC;
				ic->u.triop.result = op;
				ic->u.triop.op1 = t1;
				ic->u.triop.op2 = t2;
				insertLink(ic);
			}
			else
			{
				assert(strcmp(node->sons[1]->name, "DIV") == 0);
				ic = newInterCode();
				ic->kind = DIV_IC;
				ic->u.triop.result = op;
				ic->u.triop.op1 = t1;
				ic->u.triop.op2 = t2;
				insertLink(ic);
			}
		}
		else if ((strcmp(node->sons[1]->name, "RELOP") == 0) ||
				   (strcmp(node->sons[1]->name, "AND") == 0) ||
				   (strcmp(node->sons[1]->name, "OR") == 0) ||
				   (strcmp(node->sons[0]->name, "NOT") == 0))
		{

			printf("PPPPPPPPRELOPPPPPPPP\n");
			char* label1 = newLabel();
	  		char* label2 = newLabel();

	  		InterCode* ic = newInterCode();
	  		ic->kind = ASSIGN_IC;
	  		ic->u.binop.result = op;
	  		ic->u.binop.op = newOperand();
	  		ic->u.binop.op->kind = CONSTANT_OP;
	  		ic->u.binop.op->u.value = 0;
	  		//printf("ASS in Exp andnotor1, %d, %d\n", ic->u.binop.result->kind, ic->u.binop.op->kind);
			
	  		insertLink(ic);

	  		translateCond(node, label1, label2);

	  		ic = newInterCode();
	  		ic->kind = LABEL_IC;
	  		ic->u.label.name = strdup(label1);
	  		insertLink(ic);

	  		ic = newInterCode();
	  		ic->kind = ASSIGN_IC;
	  		ic->u.binop.result = op;
	  		ic->u.binop.op = newOperand();
	  		ic->u.binop.op->kind = CONSTANT_OP;
	  		ic->u.binop.op->u.value = 1;
	  		//printf("ASS in Exp andnotor2, %d, %d\n", ic->u.binop.result->kind, ic->u.binop.op->kind);
			
	  		insertLink(ic);

	  		ic = newInterCode();
	  		ic->kind = LABEL_IC;
	  		ic->u.label.name = strdup(label2);
	  		insertLink(ic);

	  		free(label1);
	  		free(label2);

		}
		else if (strcmp(node->sons[1]->name, "LB") == 0)
		{
						
			//EXP LB EXP RB
			Node* n = node->sons[0]->sons[0];
			
			for(; strcmp(n->name,"ID") != 0; n = n->sons[0]);

			//printf("Before not found%s\n", n->value);
			//printTable();

			Item it = getFromHashTable(n->value);

			if (it == 0)
			{
				printf("Not Found.\n");
				return;
			}

			Type type = it->u.v->type;
			Type tp = type;
			for(; tp->u.array.elem->kind != BASIC ; tp = tp->u.array.elem);
			op->kind = ADDRESS_OP;
			translateArray(node, type, tp, 0, op);

		}
		else
		{
			printf("Why i am here\n");
			//Do nothing.
			//EXP DOT..
		}
	}
	else if (strcmp(node->sons[0]->name, "LP") == 0)
	{
		translateExp(node->sons[1], op);
	}
	else if (strcmp(node->sons[0]->name, "MINUS") == 0)
	{
		Operand* t1 = newVar();
		translateExp(node->sons[1], t1);

		//MINUS
		InterCode* ic = newInterCode();
		ic->kind = MINUS_IC;
		ic->u.triop.result = op;
		ic->u.triop.op1 = newOperand();
		ic->u.triop.op1->kind = CONSTANT_OP;
		ic->u.triop.op1->u.value = 0;
		ic->u.triop.op2 = t1;
		insertLink(ic);
	}
	else if (strcmp(node->sons[0]->name, "ID") == 0)
	{
		////printf("In ID\n");
		if (node->sonNum == 1)
		{
			InterCode* ic = newInterCode();
			ic->kind = ASSIGN_IC;
			ic->u.binop.result = op;
			ic->u.binop.op = newOperand();
			ic->u.binop.op->kind = VARIABLE_OP;
			ic->u.binop.op->u.name = strdup(node->sons[0]->value);
			//printf("ASS in Exp ID, %d, %d\n", ic->u.binop.result->kind, ic->u.binop.op->kind);
			
			insertLink(ic);
		}
		else if (node->sonNum == 4)
		{
			//ID LP Args RP
			Operand* args[10];
			int i = 0;
			for (i = 0; i < 10; i++)
				args[i] = 0;

			translateArgs(node->sons[2], args);

			//Deal with write.
			if (strcmp(node->sons[0]->value, "write") == 0)
			{
				//printf("I'm writing...\n");
				InterCode* ic = newInterCode();
				ic->kind = WRITE_IC;
				ic->u.write.op = args[0];
				insertLink(ic);
			}
			else
			{
				int i = 0;
				int j = 0;
				for (j = 0; args[j] != 0; j++);
				for (i = j - 1; i >= 0; i--)
				{
					InterCode* ic = newInterCode();
					ic->kind = ARG_IC;
					ic->u.arg.op = args[i];
					insertLink(ic);
				}

				//int i = 0;
				//for (i = 0; args[i] != 0; i++)
				//{
				//	InterCode* ic = newInterCode();
				//	ic->kind = ARG_IC;
				//	ic->u.arg.op = args[i];
				//	insertLink(ic);
				//}

				InterCode* ic = newInterCode();
				ic->kind = CALLFUNC_IC;
				ic->u.call.op = op;
				ic->u.call.name = strdup(node->sons[0]->value);
				insertLink(ic);
			}

		}
		else
		{
			//ID LP RP
			assert(node->sonNum == 3);
			//Deal with read
			if (strcmp(node->sons[0]->value, "read") == 0)
			{
				//printf("I'm reading...\n");
				InterCode* ic = newInterCode();
				ic->kind = READ_IC;
				ic->u.read.op = op;
				insertLink(ic);
			}
			else
			{
				InterCode* ic = newInterCode();
				ic->kind = CALLFUNC_IC;
				ic->u.call.op = op;
				ic->u.call.name = strdup(node->sons[0]->value);
				insertLink(ic);
			}
		}

	}
	else if(strcmp(node->sons[0]->name, "INT") == 0)
	{
		//printf("Should be here\n");
		op->kind = CONSTANT_OP;
		op->u.value = strtol(node->sons[0]->value, 0, 10);
		//printf("----%d\n", op->u.value);
		//printf("woo\n");
	}
	else
	{
		printf("%s\n", node->sons[0]->name);
		//Do nothing.
	}
	//printf("exp over\n");
	return;

}

void translateArgs(Node* node, Operand** args)
{
	//printf("In Args\n");
	assert(strcmp(node->name, "Args") == 0);
	Operand* t1 = newVar();
	translateExp(node->sons[0], t1);
	int i = 0;
	for(i = 0; args[i] != 0; i++);
	//printf("Here i is %d\n", i);
	args[i] = t1;

	if (node->sonNum == 3)
		translateArgs(node->sons[2], args);

	return;
}

void printInFile(char* filename)
{
	FILE* f = fopen(filename, "w");
	if (!f)
	{
		printf("Error openning file!\n");
		exit(-1);
	}
	//printf("Creating file done\n");

	LinkedInterCode* lic = head;
	for (lic = head; lic != 0; lic = lic->next)
		writeCode(f, lic->code);

	fclose(f);
	return;
}

void writeCode(FILE* f, InterCode* ic)
{
	//printf("Writing a code  %d\n", ic->kind);
	switch(ic->kind)
	{
		case FUNCTION_IC:
			fprintf(f, "FUNCTION %s :\n", ic->u.function.name);
			break;
		case PARAM_IC:
			fprintf(f, "PARAM %s\n", ic->u.param.name);
			break;
		case LABEL_IC:
			fprintf(f, "LABEL %s :\n", ic->u.label.name);
			break;
		case LABEL_GOTO_IC:
			fprintf(f, "GOTO %s\n", ic->u.label_goto.name);
			break;
		case CALLFUNC_IC:
			fprintf(f, "%s := CALL %s\n", ic->u.call.op->u.name, ic->u.call.name);
			break;
		case RETURN_IC:
			if(ic->u.ret.op->kind == CONSTANT_OP)
				fprintf(f, "RETURN #%d\n", ic->u.ret.op->u.value);
			else
				fprintf(f, "RETURN %s\n", ic->u.ret.op->u.name);
			break;
		case WRITE_IC:
			//printf("I'm in writing ic, %d\n", ic->u.write.op->kind);
			if(ic->u.write.op->kind == CONSTANT_OP)
				fprintf(f, "WRITE #%d\n",ic->u.write.op->u.value);
			else
				fprintf(f, "WRITE %s\n",ic->u.write.op->u.name);
			break;
		case READ_IC:
			fprintf(f, "READ %s\n",ic->u.read.op->u.name);
			break;
		case ARG_IC:
			if(ic->u.arg.op->kind==CONSTANT_OP)
				fprintf(f,"ARG #%d\n",ic->u.arg.op->u.value);
			else
				fprintf(f,"ARG %s\n",ic->u.arg.op->u.name);
			break;
		case DEC_IC:
			fprintf(f, "DEC %s %d\n", ic->u.dec.op->u.name, ic->u.dec.size);
			break;
		case COND_IC:
			if(ic->u.cond.op1->kind==CONSTANT_OP&&ic->u.cond.op2->kind==CONSTANT_OP)
				fprintf(f,"IF #%d %s #%d GOTO %s\n",ic->u.cond.op1->u.value,\
				ic->u.cond.op,ic->u.cond.op2->u.value,ic->u.cond.name);
			else if(ic->u.cond.op1->kind == CONSTANT_OP)
				fprintf(f,"IF #%d %s %s GOTO %s\n",ic->u.cond.op1->u.value,\
				ic->u.cond.op,ic->u.cond.op2->u.value,ic->u.cond.name);
			else if(ic->u.cond.op2->kind == CONSTANT_OP)
				fprintf(f,"IF %s %s #%d GOTO %s\n",ic->u.cond.op1->u.value,\
				ic->u.cond.op,ic->u.cond.op2->u.value,ic->u.cond.name);
			else
				fprintf(f,"IF %s %s %s GOTO %s\n",ic->u.cond.op1->u.value,\
				ic->u.cond.op,ic->u.cond.op2->u.value,ic->u.cond.name);
			break;
		case MINUS_IC:
			//printf("I'm in minus ic... %d   %d  \n", ic->u.triop.op1->kind, ic->u.triop.op2->kind);
			if(ic->u.triop.op1->kind==CONSTANT_OP&&ic->u.triop.op2->kind==CONSTANT_OP){
				//printf("WAHAHA\n");
				fprintf(f,"%s := #%d - #%d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.value);
			}
			else if(ic->u.triop.op1->kind==CONSTANT_OP){
				fprintf(f,"%s := #%d - %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.name);
			}
			else if(ic->u.triop.op2->kind==CONSTANT_OP){
				fprintf(f,"%s := %s - %d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.value);
			}
			else{
				fprintf(f,"%s := %s - %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.name);
			}
			break;
		case ADD_IC:
			if(ic->u.triop.op1->kind==CONSTANT_OP&&ic->u.triop.op2->kind==CONSTANT_OP){
				fprintf(f,"%s := #%d + #%d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.value);
			}
			else if(ic->u.triop.op1->kind==CONSTANT_OP){
				fprintf(f,"%s := #%d + %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.name);
			}
			else if(ic->u.triop.op2->kind==CONSTANT_OP){
				fprintf(f,"%s := %s + #%d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.value);
			}
			else{
				fprintf(f,"%s := %s + %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.name);
			}
			break;
		case SUB_IC:
			if(ic->u.triop.op1->kind==CONSTANT_OP&&ic->u.triop.op2->kind==CONSTANT_OP){
				fprintf(f,"%s := #%d - #%d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.value);
			}
			else if(ic->u.triop.op1->kind==CONSTANT_OP){
				fprintf(f,"%s := #%d - %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.name);
			}
			else if(ic->u.triop.op2->kind==CONSTANT_OP){
				fprintf(f,"%s := %s - #%d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.value);
			}
			else{
				fprintf(f,"%s := %s - %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.name);
			}
			break;
		case MUL_IC:
			if(ic->u.triop.op1->kind==CONSTANT_OP&&ic->u.triop.op2->kind==CONSTANT_OP){
				fprintf(f,"%s := #%d * #%d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.value);
			}
			else if(ic->u.triop.op1->kind==CONSTANT_OP){
				fprintf(f,"%s := #%d * %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.name);
			}
			else if(ic->u.triop.op2->kind==CONSTANT_OP){
				fprintf(f,"%s := %s * #%d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.value);
			}
			else{
				fprintf(f,"%s := %s * %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.name);
			}
			break;
		case DIV_IC:
			if(ic->u.triop.op1->kind==CONSTANT_OP&&ic->u.triop.op2->kind==CONSTANT_OP){
				fprintf(f,"%s := #%d / #%d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.value);
			}
			else if(ic->u.triop.op1->kind==CONSTANT_OP){
				fprintf(f,"%s := #%d / %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.value,ic->u.triop.op2->u.name);
			}
			else if(ic->u.triop.op2->kind==CONSTANT_OP){
				fprintf(f,"%s := %s / #%d\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.value);
			}
			else{
				fprintf(f,"%s := %s / %s\n",ic->u.triop.result->u.name,\
				ic->u.triop.op1->u.name,ic->u.triop.op2->u.name);
			}
			break;
		case ASSIGN_IC:
			//printf("HERE\n");
			if(ic->u.binop.op->kind==CONSTANT_OP)
			{	
				fprintf(f,"%s := #%d\n",ic->u.binop.result->u.name,\
				ic->u.binop.op->u.value);
			}
			else
			{
				
				//printf("%dvvv%s\n", ic->u.binop.op->kind, ic->u.binop.result->u.name);
				//printf("HERE\n");
				//printf("sss%s\n", ic->u.binop.op->u.name);
				fprintf(f,"%s := %s\n",ic->u.binop.result->u.name,\
				ic->u.binop.op->u.name);
			}
			break;			
		default:
			printf("Wrong kind: %d\n", ic->kind);
			break;

	}
}

