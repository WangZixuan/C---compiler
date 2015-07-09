#include "grammar.h"
#include <assert.h>

/*int kind in this file:
	0 = int
	1 = float
	2 = array
	3 = struct
	4 = function
	6 = var
	5 = allFit
	7 = error
	8 = valid
	9 = fieldlist
	10 = local
	11 = basic
*/

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct Var_* Var;
typedef struct Func_* Func;
typedef struct Args_* Args;
typedef struct Item_* Item;
typedef struct ExpReturnType_ ExpReturnType_;

struct Type_ typeInt;
struct Type_ typeFloat;

Item hashTable[HASHTABLELENGTH];

Type thisReturnType;

unsigned int hash_pjw(char* name)
{
	unsigned int val = 0, i;
	for (; *name; ++name)
	{
		val = (val << 2) + *name;
		if (i = val & ~HASHTABLELENGTH) val = (val ^ (i >> 12)) & HASHTABLELENGTH;
	}
	return val;
}

int initHashTable()
{
	int i = 0;
	for (i = 0; i < HASHTABLELENGTH; i++)
	{
		hashTable[i] = (Item)malloc(sizeof(struct Item_));
		hashTable[i]->next = 0;
	}
	return 0;
}

int insertInHashTable(Item item)
{
	//Insert in the hash table.
	int index = hash_pjw(item->name);

	Item p = hashTable[index];
	item->next = p->next;
	p->next = item;

	return 0;
}

int checkIfUsed(char* n)
{
	//printf("In checkIfUsed\n");
	//printf("I am checking %s\n", n);
	int index = hash_pjw(n);
	Item p = hashTable[index];
	if(p == 0)
		return -1;
	while(p != 0)
	{
		if (p->name != 0 && 0 == strcmp(p->name, n) != 0)
			return 1;
		p = p->next;
	}
	if(p == 0)
		return 0;
}

Item getFromHashTable(char* n)
{
	int index = hash_pjw(n);
	Item p = hashTable[index];
	while (p != 0)
	{
		if (p->name != 0 && 0 == strcmp(p->name, n) != 0)
			return p;
		p = p->next;
	}
	return 0;
}


//------------------High-level Definitions

void program(struct Node* node)
{
	//printf("In Program\n");

	//Initialization.
	typeInt.kind = BASIC;
	typeInt.u.basic = 0;
	typeFloat.kind = BASIC;
	typeFloat.u.basic = 1;
	initHashTable();
	//Program -> ExtDefList
	assert(strcmp(node->name, "Program") == 0);
	assert(node->sonNum == 1);
	assert(strcmp(node->sons[0]->name, "ExtDefList") == 0);
	extDefList(node->sons[0]);
	return;
}

void extDefList(struct Node* node)
{
	//printf("In ExtDefList\n");
	//ExtDefList -> EMPTY
	//			  | ExfDef ExtDefList

	if(0 == node->sonNum)//EMPTY
		return;
	else
	{
		assert(strcmp(node->name, "ExtDefList") == 0);
		assert(node->sonNum == 2);
		//assert(strcmp(node->sons[0]->name, "ExtDef") == 0);
		//assert(strcmp(node->sons[1]->name, "ExtDefList") == 0);
		extDef(node->sons[0]);
		extDefList(node->sons[1]);
		return;
	}
}

void extDef(struct Node* node)
{
	//printf("In ExtDef\n");
	//ExtDef -> Specifier ExtDecList SEMI
  	//		  | Specifier SEMI
  	//		  | Specifier FunDec CompSt
  	assert(strcmp(node->name, "ExtDef") == 0);
  	if (2 == node->sonNum)
  	{
  		//Specifier SEMI
  		assert(strcmp(node->sons[0]->name, "Specifier") == 0);
		assert(strcmp(node->sons[1]->name, "SEMI") == 0);
		specifier(node->sons[0]);
		//Do nothing.
		return;
  	}
  	else if (strcmp(node->sons[1], "ExtDecList") == 0)
  	{
  		//Specifier ExtDecList SEMI
  		assert(node->sonNum == 3);
  		assert(strcmp(node->sons[0]->name, "Specifier") == 0);
  		assert(strcmp(node->sons[2]->name, "SEMI") == 0);
  		Type thisType = specifier(node->sons[0]);
  		extDecList(node->sons[1], thisType);
  	}
  	else
  	{
  		//Specifier FunDec CompSt
  		assert(node->sonNum == 3);
  		assert(strcmp(node->sons[0]->name, "Specifier") == 0);
  		assert(strcmp(node->sons[1]->name, "FunDec") == 0);
  		assert(strcmp(node->sons[2]->name, "CompSt") == 0);
  		Type thisType = specifier(node->sons[0]);
  		Func f = funDec(node->sons[1]);

  		//Insert in the hash table.
  		Item item = (Item)malloc(sizeof(Item_));
  		item->name = f->name;//Just a pointer.
  		item->kind = 4;//Function.
  		f->returnType = thisType;
  		item->u.f = f;

  		insertInHashTable(item);

  		thisReturnType = thisType;

  		compSt(node->sons[2]);
  	}
  	return;
}

void extDecList(struct Node* node, Type t)
{
	//printf("In ExtDecList\n");
	//ExtDecList -> VarDec
  	//	 		  | VarDec COMMA ExtDecList
  	assert(strcmp(node->name, "ExtDecList") == 0);
  	if (node->sonNum == 1)
  	{
  		Var v = varDec(node->sons[0]);
  		if (checkIfUsed(v->name) == 1)
  			printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", node->sons[0]->lineno, node->sons[0]->name);
  		else
  		{
  			//Insert in the hash table.
  			Item item = (Item)malloc(sizeof(Item_));
	  		item->name = v->name;
	  		item->kind = 6;//Var.
	  		item->u.v->type = t;
	  		item->u.v = v;

	  		insertInHashTable(item);
  		}
  	}
  	else
  	{
  		//VarDec COMMA ExtDecList
  		assert(node->sonNum == 3);
  		Var v = varDec(node->sons[0]);
  		extDecList(node->sons[2], t);

  	}
  	return;
}

//------------------Specifiers
Type specifier(struct Node* node)
{
	//printf("In Specifier\n");
	//Specifier -> TYPE
  	//			 | StructSpecifier
  	assert(strcmp(node->name, "Specifier") == 0);
  	if (strcmp(node->sons[0]->name, "TYPE") == 0)
  	{
  		if (strcmp(node->sons[0]->value, "int") == 0)
  		{
  			typeInt.kind = BASIC,
			typeInt.u.basic = 0;
  			return &typeInt;
  		}
  		else
  		{
  			assert(strcmp(node->sons[0]->value, "float") == 0);
  			typeFloat.kind = BASIC,
			typeFloat.u.basic = 1;
  			return &typeFloat;
  		}
  	}
  	else
  	{
  		assert(strcmp(node->sons[0]->name, "StructSpecifier") == 0);
  		return structSpecifier(node->sons[0]);
  	}
}

Type structSpecifier(struct Node* node)
{
	//printf("In StructSpecifier\n");
	//StructSpecifier -> STRUCT OptTag LC DefList RC
	//				   | STRUCT Tag
	assert(strcmp(node->name, "StructSpecifier") == 0);
	if (node->sonNum == 5)
	{
		//STRUCT OptTag LC DefList RC
		Item item = optTag(node->sons[1]);

		item->u.type->u.structure->tail = defList(node->sons[3], 1);

		return item->u.type;
	}
	else
	{
		//STRUCT Tag
		assert(node->sonNum == 2);
		Item item = getFromHashTable(node->sons[1]->sons[0]->value);


		//PRINT HASH TABLE
		/*int i=0;
		for (i=0;i<HASHTABLELENGTH; i++)
		{
			Item it = hashTable[i];
			while(it != NULL && it->name != NULL)
				printf("%s          ", it->name);
			printf("\n");
		}*/

		if (item == 0)
		{
			printf("Error type 17 at Line %d: Undefined structure.\n", node->sons[1]->lineno);
				return 0;
		}
		else
		{
			assert(item != 0);
			if (item->kind == 5)
				return item->u.type;
			else
			{
				printf("Error type 17 at Line %d: Undefined structure.\n", node->sons[1]->lineno);
				return 0;
			}
		}
	}
}

Item optTag(struct Node* node)
{
	//printf("In OptTag\n");
	//OptTag -> EMPTY
  	//    	  | ID

  	if (node->sonNum == 1)
  	{
  		assert(strcmp(node->name, "OptTag") == 0);
  		//ID
  		if(checkIfUsed(node->sons[0]->value) == 1)
  			printf("Error type 16 at Line %d: Name is used by other structures \"%s\"\n", node->sons[0]->lineno, node->sons[0]->value);

  		//Insert in the hash table.
  		FieldList fl = (FieldList)malloc(sizeof(FieldList_));
  		fl->name = node->sons[0]->value;
  		fl->type = 0;
  		fl->tail = 0;
  		Type t = (Type)malloc(sizeof(Type_));
  		t->kind = STRUCTURE;
  		t->u.structure = fl;
  		Item item = (Item)malloc(sizeof(Item_));
  		item->name = fl->name;
  		item->kind = 5;
  		item->u.type = t;

  		insertInHashTable(item);

  		return item;
  	}
  	else
  	{
  		assert(node->sonNum == 0);

  		FieldList fl = (FieldList)malloc(sizeof(FieldList_));
  		fl->name = 0;
  		fl->type->kind = STRUCTURE;
  		fl->tail = 0;
  		Type t = (Type)malloc(sizeof(Type_));
  		t->kind = STRUCTURE;
  		t->u.structure = fl;
  		Item item = (Item)malloc(sizeof(Item_));
  		item->name = item->u.type->u.structure->name;
  		return item;

  	}
}

void tag(struct Node* node)
{
	//printf("In Tag\n");
	//Tag -> ID
	assert(strcmp(node->name, "Tag") == 0);
	if(checkIfUsed(node->sons[0]->value) == 1)
  			printf("Error type Unknown at Line %d:\n", node->sons[0]->lineno);
	return;
}

//------------------Declarators
Var varDec(struct Node* node)
{
	//printf("In VarDec\n");
	//VarDec -> ID
 	//		  | VarDec LB INT RB
 	assert(strcmp(node->name, "VarDec") == 0);
 	if (node->sonNum == 1)
 	{
 		//ID
 		Var v = (Var)malloc(sizeof(Var_));
 		v->name = node->sons[0]->value;
 		v->type = 0;
 		return v;
 	}
 	else
 	{
 		//VarDec LB INT RB
 		assert(node->sonNum == 4);
 		Var v = varDec(node->sons[0]);
 		Type t = (Type)malloc(sizeof(Type_));
 		t->kind == ARRAY;
 		t->u.array.elem = v->type;
 		t->u.array.size = strtol(node->sons[2]->value, 0, 10);
 		v->type = t;
 		return v;
 	}
}

Func funDec(struct Node* node)
{
	//printf("In FunDec\n");
	//FunDec -> ID LP VarList RP
 	//		  | ID LP RP
	assert(strcmp(node->name, "FunDec") == 0);
	if (checkIfUsed(node->sons[0]->value) == 1)//Check ID.
		printf("Error type 4 at Line %d: Redefined function \"%s\"\n", node->sons[0]->lineno,node->sons[0]->value);
	if (node->sonNum == 4)
	{
		//ID LP VarList RP
		Func f = (Func)malloc(sizeof(Func_));
		f->name = node->sons[0]->value;
		f->returnType = 0;
		f->args = varList(node->sons[2]);
		return f;
	}
	else
	{
		assert(node->sonNum == 3);
		Func f = (Func)malloc(sizeof(Func_));
		f->name = node->sons[0]->value;
		f->returnType = 0;
		f->args = 0;
		return f;
	}
}

Args varList(struct Node* node)
{
	//printf("In VarList\n");
	//VarList -> ParamDec COMMA VarList
 	//		   | ParamDec
	assert(strcmp(node->name, "VarList") == 0);
	Args a = paramDec(node->sons[0]);
	if (node->sonNum == 1)
		return a;
	else
	{
		assert(node->sonNum == 3);
		a->next = varList(node->sons[2]);
		return a;
	}
}

Args paramDec(struct Node* node)
{
	//printf("In ParamDec\n");
	//ParamDec -> Specifier VarDec
	assert(strcmp(node->name, "ParamDec") == 0);
	assert(node->sonNum == 2);

	Type t = specifier(node->sons[0]);
	Var v = varDec(node->sons[1]);
	if (v->type == 0)
		v->type = t;
	else//Array
	{
		Type tt = v->type;
		while (tt->u.array.elem != 0)
			tt = tt->u.array.elem;
		tt->u.array.elem = t;
	}

	if (checkIfUsed(v->name) == 1)
		printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", node->sons[1]->lineno, node->sons[1]->value);

	//Insert in the hash table.
	Item item = (Item)malloc(sizeof(Item_));
	item->name = v->name;
	item->kind = 6;//var.
	item->u.v = v;

	insertInHashTable(item);

	Args a = (Args)malloc(sizeof(Args_));
	a->v = v;
	a->next = 0;
	return a;
}

//------------------Statements
void compSt(struct Node* node)
{
	//printf("In CompSt\n");
	//CompSt -> LC DefList StmtList RC
	assert(strcmp(node->name, "CompSt") == 0);
	defList(node->sons[1], 0);
	stmtList(node->sons[2]);
	return;
}

void stmtList(struct Node* node)
{
	//printf("In StmtList\n");
	//StmtList -> EMPTY
 	// 		    | Stmt StmtList
 	if (node->sonNum == 0)
 		return;
 	else
 	{
 		//printf("WHY..%s........%s.\n", node->name,  node->sons[0]->name);
 		assert(strcmp(node->name, "StmtList") == 0);
 		assert(node->sonNum == 2);
 		stmt(node->sons[0]);
 		stmtList(node->sons[1]);
 		return;
 	}
}

void stmt(struct Node* node)
{
	//printf("In Stmt\n");
	//Stmt -> Exp SEMI
  	//		| CompSt
  	//		| RETURN Exp SEMI
  	//		| IF LP Exp RP Stmt
  	//		| IF LP Exp RP Stmt ELSE Stmt
  	//		| WHILE LP Exp RP Stmt
  	assert(strcmp(node->name, "Stmt") == 0);
  	if (node->sonNum == 2)
  	{
  		//Exp SEMI
  		exp(node->sons[0]);
  	}
  	else if (node->sonNum == 1)
  	{
  		//CompSt
  		compSt(node->sons[0]);
  	}
  	else if(node->sonNum == 3)
  	{
  		//RETURN Exp SEMI
  		ExpReturnType_ ert = exp(node->sons[1]);
  		if (ert.kind != 7 && thisReturnType != ert.type)
  			printf("Error type 8 at Line %d: Different types in the return.\n", node->sons[1]->lineno);

  	}
  	else if (node->sonNum == 7)
  	{
  		//IF LP Exp RP Stmt ELSE Stmt
  		ExpReturnType_ ert = exp(node->sons[2]);
  		if (ert.kind != 7 && ert.type != &typeInt)
  			printf("Error type 7 at Line %d: Mismatched operands\n",node->sons[0]->lineno);

  		stmt(node->sons[4]);
  		stmt(node->sons[6]);
  	}
  	else
  	{
  		assert(node->sonNum == 5);
  		assert(strcmp(node->sons[0]->name, "WHILE") == 0 || strcmp(node->sons[0]->name, "IF") == 0);

  		//WHILE LP Exp RP Stmt
  		//IF LP Exp RP Stmt
  		exp(node->sons[2]);

  		ExpReturnType_ ert = exp(node->sons[2]);
  		if (ert.kind != 7 && ert.type != &typeInt)
  			printf("Error type 7 at Line %d: Mismatched operands\n",node->sons[0]->lineno);
  		stmt(node->sons[4]);

  	}
}

//------------------Local Definitions
FieldList defList(struct Node* node, int varOrStruct)
{
    if (strcmp(node->name, "") ==0)
    return 0;
	//printf("In DefList\n");
	//DefList : EMPTY
 	// 		  | Def DefList

 	assert(strcmp(node->name, "DefList") == 0);
 	if (node->sonNum == 0)
 	{
 		//printf("EMPTY in DefList\n");
 		return 0;
 	}
 	else
 	{
 		assert(strcmp(node->name, "DefList") == 0);
 		assert(node->sonNum == 2);
 		FieldList fl = def(node->sons[0], varOrStruct);

 		//printf("IDONTKNOW %d\n", varOrStruct);

 		if (varOrStruct == 0)
 			defList(node->sons[1], varOrStruct);
 		else
 		{

 			assert(varOrStruct == 1);
 			if (fl != 0)
 			{
 				FieldList flfl = fl;
 				while (flfl->tail != 0)
 					flfl =flfl->tail;
 				flfl->tail = defList(node->sons[1], varOrStruct);
 			}
 		}
 		return fl;
 	}
}

FieldList def(struct Node* node, int varOrStruct)
{
	//printf("In Def\n");
	//Def -> Specifier DecList SEMI
	assert(strcmp(node->name, "Def") == 0);
	Type t = specifier(node->sons[0]);
	FieldList fl = decList(node->sons[1], t, varOrStruct);

	return fl;
}

FieldList decList(struct Node* node, Type t, int varOrStruct)
{
	//printf("In DecList\n");
	//DecList -> Dec
  	//		   | Dec COMMA DecList
  	assert(strcmp(node->name, "DecList") == 0);
  	FieldList fl =dec(node->sons[0], 0, varOrStruct);
  	if (node->sonNum == 1)

 		return fl;
 	else
 	{
 		assert(node->sonNum == 3);
 		if (varOrStruct == 1)
 			fl->tail = decList(node->sons[2], t, varOrStruct);
 		else
 		{
 			assert(varOrStruct == 0);
 			decList(node->sons[2], t, varOrStruct);
 		}
 		return fl;
 	}
}

FieldList dec(struct Node* node, Type t, int varOrStruct)
{
	//printf("In Dec\n");
	//Dec -> VarDec
 	// 	   | VarDec ASSIGNOP Exp
 	assert(strcmp(node->name, "Dec") == 0);

 	Var v = varDec(node->sons[0]);
 	if (checkIfUsed(v->name) == 1)
 	{
 		if (varOrStruct == 1)
 			printf("Error type 15 at Line %d: Redefined field in the struct.\n", node->sons[0]->lineno);
 		else
 		{
 			assert(varOrStruct == 0);
 			printf("Error type 3 at Line %d: Redefined variable.\n", node->sons[0]->lineno);
 		}
 	}

 	v->type = t;

 	FieldList fl = 0;

 	if (varOrStruct == 1)
 	{
 		//Insert in the hash table.
 		fl = (FieldList)malloc(sizeof(FieldList_));
 		fl->name = v->name;
 		fl->type = v->type;
 		fl->tail = 0;

 		Item item = (Item)malloc(sizeof(Item_));
 		item->name = fl->name;
 		item->kind = 9;
 		item->u.fl = fl;

 		insertInHashTable(item);
 	}
 	else
 	{
 		//Insert in the hash table.
 		assert(varOrStruct == 0);
 		Item item = (Item)malloc(sizeof(Item_));
 		item->name = v->name;
 		item->kind = 6;
 		item->u.v = v;

 		insertInHashTable(item);
 	}

 	if (node->sonNum == 3)
 	{
 		// VarDec ASSIGNOP Exp
 		if (varOrStruct == 1)
 			printf("Error type 15 at Line %d: Redefined names in the structure.\n", node->sons[0]->lineno);
 		else
 		{
 			assert(varOrStruct == 0);
 			ExpReturnType_ ert = exp(node->sons[2]);
  		}
 	}
 	else
 		assert(node->sonNum == 1);

 	return fl;
}

ExpReturnType_ exp(struct Node* node)
{
	//printf("In Exp\n");
	//Exp -> Exp ASSIGNOP Exp
  	//	  	| Exp AND Exp
  	//	  	| Exp OR Exp
  	//	  	| Exp RELOP Exp
  	//	  	| Exp PLUS Exp
  	//	  	| Exp MINUS Exp
  	//	  	| Exp STAR Exp
  	//	  	| Exp DIV Exp
  	//	  	| LP Exp RP
  	//	  	| MINUS Exp
  	//	  	| NOT Exp
  	//	  	| ID LP Args RP
  	//	  	| ID LP RP
  	//	  	| Exp LB Exp RB
  	//	  	| Exp DOT ID
  	//	  	| ID
  	//	  	| INT
  	//	  	| FLOAT
  	assert(strcmp(node->name, "Exp") == 0);

  	ExpReturnType_ ert;

	if (strcmp(node->sons[0]->name, "Exp") == 0)
	{
		//printf("In the first is Exp\n");
		ExpReturnType_ ert1 = exp(node->sons[0]);
		if (strcmp(node->sons[1]->name, "ASSIGNOP") == 0)
		{
			//printf("In assignop\n");
			//Exp ASSIGNOP Exp
			if (ert1.kind != 7 && ert1.kind != 6) {
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", node->sons[0]->lineno);
				ert.kind = 7;
				return ert;
			}

			struct ExpReturnType_ ert2 = exp(node->sons[2]);

			if (ert1.kind == 7 || ert2.kind == 7)
				ert.kind = 7;
			else if (ert1.type != ert2.type)
			{
				printf("Error type 5 at Line %d: Type mismatched for assignment.\n", node->sons[1]->lineno);
				ert.kind = 7;
			} else
			{
				ert.kind = 10;
				ert.type = ert1.type;
				ert.flag = ert1.flag;
			}
			return ert;
		}
		else if (strcmp(node->sons[1]->name, "AND") == 0 || strcmp(node->sons[1]->name, "OR") == 0)
		{
			//Exp AND Exp
  			//Exp OR Exp

  			ExpReturnType_ ert2 = exp(node->sons[2]);

			if (ert1.kind == 7 || ert2.kind == 7)
				ert.kind = 7;
			else if (ert1.flag == 0 && ert2.flag == 0)
			{
				ert.kind = 10;
				ert.type = ert1.type;
				ert.flag = ert1.flag;
			}
			else
			{
				ert.kind = 7;
				if (ert1.type->kind != BASIC || ert1.type->u.basic != 0)
					printf("Error type 7 at Line %d: Mismatched operands.\n", node->sons[1]->lineno);

				if (ert2.type->kind != BASIC || ert2.type->u.basic != 1)
					printf("Error type 7 at Line %d: Mismatched operands.\n", node->sons[1]->lineno);
			}
			return ert;
		}
		else if ((strcmp(node->sons[1]->name, "RELOP") == 0) ||
				 (strcmp(node->sons[1]->name, "PLUS") == 0)  ||
				 (strcmp(node->sons[1]->name, "MINUS") == 0) ||
				 (strcmp(node->sons[1]->name, "STAR") == 0)  ||
				 (strcmp(node->sons[1]->name, "DIV") == 0))
		{
			struct ExpReturnType_ ert2 = exp(node->sons[2]);

			if (ert1.kind == 7 || ert2.kind == 7)
				ert.kind = 7;

			else if ((ert1.flag <= 1 && ert2.flag <= 1) && (ert1.kind == ert2.kind))
			{
				ert.kind = 10;
				ert.type = &typeInt;
				ert.flag = ert1.flag;
			}
			else
			{
				//printf("ert1.kind  %d    ert2.kind  %d \n", ert1.kind, ert2.kind);

				ert.kind = 7;
				printf("Error type 7 at Line %d: Mismatched operands.\n", node->sons[1]->lineno);
			}
			return ert;
		}
		else if (strcmp(node->sons[1]->name, "LB") == 0)
		{
			//Exp LB Exp RB
			//printf("In LB\n");
			ExpReturnType_ ert2 = exp(node->sons[2]);

			if (ert1.kind == 7 || ert2.kind == 7)
				ert.kind = 7;
			else if (ert1.flag == 2 && ert2.type == &typeInt)
			{
				//Array
				ert.kind = ert1.kind;
				ert.type = ert1.type->u.array.elem;

				if (ert.type == 0)
					ert.flag = -1;
				else if (ert.type  == &typeInt)
					ert.flag = 0;
				else if (ert.type  == &typeFloat)
					ert.flag = 1;
				else if (ert.type->kind == ARRAY)
					ert.flag = 2;
				else
				{
					assert(ert.type->kind == STRUCTURE);
					ert.flag = 3;
				}

			}
			else
			{
				ert.kind = 7;
				if (ert1.flag != 2)
					printf("Error type 10 at Line %d: Not an array.\n", node->sons[1]->lineno);

				if (ert2.type != &typeInt)
					printf("Error type 12 at Line %d: Not an int.\n", node->sons[1]->lineno);
			}
			return ert;
		}
		else
		{
			//Exp DOT ID

			assert(strcmp(node->sons[1]->name, "DOT") == 0);
			//printf("ffffff%d\n", ert1.flag);
			if (ert1.kind == 7)
				return ert1;
			else if (ert1.flag == 3)
			{
				//printf("why i'm here\n");
				FieldList fl = ert1.type->u.structure->tail;
				while (fl != 0)
				{
					if (strcmp(fl->name, node->sons[2]->name) == 0)
					{
						ert1.type = fl->type;
						if (ert1.type == 0)
							ert1.flag = -1;
						else if (ert1.type  == &typeInt)
							ert1.flag = 0;
						else if (ert1.type  == &typeFloat)
							ert1.flag = 1;
						else if (ert1.type->kind == ARRAY)
							ert1.flag = 2;
						else
						{
							assert(ert1.type->kind == STRUCTURE);
							ert1.flag = 3;
						}
						break;
					}
					fl = fl->tail;
				}

				if (fl == 0)
				{
					ert1.kind = 7;
					printf("Error type 14 at Line %d: Non-existent field\n", node->sons[1]);
				}
			}

			else
			{
				ert1.kind = 7;
				printf("Error type 13 at Line %d: Illegal use of \'.\'\n", node->sons[1]->lineno);
			}
			return ert1;
		}

	}
	else if (strcmp(node->sons[0]->name, "LP") == 0)
	{
		//LP Exp RP
		ert = exp(node->sons[1]);
		return ert;
	}
	else if (strcmp(node->sons[0]->name, "MINUS") == 0 || strcmp(node->sons[0]->name, "NOT") == 0)
	{
		//MINUS Exp
		ert = exp(node->sons[1]);

		if (ert.flag <= 1 && ert.kind != 7)
			ert.kind = 10;
		else
			ert.kind = 7;

		return ert;
	}
	else if (strcmp(node->sons[0]->name, "ID") == 0)
	{
		//printf("In first is ID\n");
		Item item = getFromHashTable(node->sons[0]->value);
		if (item == 0)
			ert.kind = 7;
		else
		{
			ert.kind = item->kind;
			switch (item->kind)
			{
				case 6: ert.type = item->u.v->type; break;//var
				case 4: ert.type = item->u.f->returnType; break;//function
				case 9: ert.type = 0; break;
			}

			//printf("ok\n");
			if (ert.type == 0)
				ert.flag = -1;
			else if (ert.type  == &typeInt)
				ert.flag = 0;
			else if (ert.type  == &typeFloat)
				ert.flag = 1;
			else if (ert.type->kind == ARRAY)
				ert.flag = 2;
			else
			{
				assert(ert.type->kind == STRUCTURE);
				ert.flag = 3;
			}



		}


		if (node->sonNum == 1)
		{
			//ID
			if (item == 0)
				printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", node->sons[0]->lineno, node->sons[0]->value);

			return ert;
		}
		else
		{
			//ID LP RP
			//ID LP Args RP
			if (item == 0)
			{
				printf("Error type 2 at Line %d: Undefined function \"%s\".\n", node->sons[0]->lineno, node->sons[0]->value);
				return ert;
			}
			if (item->kind != 4) {
				printf("Error type 11 at Line %d: \"%s\" is not a function\n", node->sons[0]->lineno, item->name);
				ert.kind = 7;
				return ert;
			}
			ert.kind = 10;
			ert.type = item->u.f->returnType;



			Args a = item->u.f->args;
			if (node->sonNum == 4)
			{	//ID LP Args RP
				if (args(node->sons[2], a) == 0)
					ert.kind = 7;
			}
			return ert;

		}
	}
	else if (strcmp(node->sons[0]->name, "INT") == 0)
	{
		ert.type = &typeInt;
		ert.flag = 0;
		return ert;
	}
	else
	{
		assert(strcmp(node->sons[0]->name, "FLOAT") == 0);
		ert.type = &typeFloat;
		ert.flag = 1;
		return ert;
	}

}

int args(struct Node* node, Args a)
{
	//printf("In Args\n");
	//Args -> Exp COMMA Args
  	//    	| Exp
  	ExpReturnType_ ert = exp(node->sons[0]);

  	if(node->sonNum == 1)
  	{
  		if (ert.kind == 7)
  			return 0;
  		else if (a == 0 || ert.type == a->v->type)
  		{
  			printf("Error type 9 at Line %d: Different arguments to function.\n", node->sons[0]->lineno);
  			return 0;
  		}
  		else
  		{
  			printf("Error type 9 at Line %d: Different arguments to function.\n", node->sons[0]->lineno);
  			return 0;
  		}
  	}
  	else
  	{

  		assert(node->sonNum == 3);
  		return args(node->sons[2], a->next);
  	}
}
