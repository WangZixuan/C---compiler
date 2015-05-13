%{
#include <stdio.h>
#include <stdarg.h>
#include "tree.h"
#include "lex.yy.c"

extern struct Node;
Node* root;
int errorNum = 0;

%}

/*Declared types*/
%union
{
	//int type_int;
	//float type_float;
	Node* type_node;
}

/*Declared tokens*/
%token <type_node> INT
%token <type_node> FLOAT
%token <type_node> SEMI COMMA
%token <type_node> TYPE STRUCT ID
%token <type_node> LP RP LB RB LC RC
%token <type_node> RETURN IF WHILE ELSE 
%token <type_node> ASSIGNOP PLUS MINUS STAR DIV 
%token <type_node> AND OR NOT DOT RELOP

%type <type_node> Program ExtDefList ExtDef ExtDecList 
%type <type_node> Specifier StructSpecifier OptTag Tag
%type <type_node> VarDec FunDec VarList ParamDec
%type <type_node> CompSt StmtList Stmt
%type <type_node> DefList Def DecList Dec
%type <type_node> Exp Args

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGNOP
%left OR
%left AND 
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT
%error-verbose

%%

/*High-level Definitions*/
Program : ExtDefList { $$ = createNode("Program", "", 1, @$.first_line, 1, $1); root = $$; }
;
ExtDefList : /*empty */{ $$ = createNode("", "", 1, @$.first_line, 0); }
  | ExtDef ExtDefList { $$ = createNode("ExtDefList", "", 1, @$.first_line, 2, $1, $2); }
;
ExtDef : Specifier ExtDecList SEMI { $$ = createNode("", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Specifier SEMI { $$ = createNode("ExtDef", "", 1, @$.first_line, 2, $1, $2); }
  | Specifier FunDec CompSt { $$ = createNode("ExtDef", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Specifier error { errorNum++; }
  | error SEMI { errorNum++; }
;
ExtDecList : VarDec { $$ = createNode("ExtDecList", "", 1, @$.first_line, 1, $1); }
  | VarDec COMMA ExtDecList { $$ = createNode("ExtDecList", "", 1, @$.first_line, 3, $1, $2, $3); }
;

/*Specifiers*/
Specifier : TYPE { $$ = createNode("Specifier", "", 1, @$.first_line, 1, $1); }
  | StructSpecifier { $$ = createNode("Specifier", "", 1, @$.first_line, 1, $1); }
;
StructSpecifier : STRUCT OptTag LC DefList RC { $$ = createNode("StructSpecifier", "", 1, @$.first_line, 5, $1, $2, $3, $4, $5); }
  | STRUCT Tag { $$ = createNode("StructSpecifier", "", 1, @$.first_line, 2, $1, $2); }
;
OptTag :  /* empty */{ $$ = createNode("", "", 1, @$.first_line, 0); }
  | ID { $$ = createNode("OptTag", "", 1, @$.first_line, 1, $1); }
;
Tag : ID { $$ = createNode("Tag", "", 1, @$.first_line, 1, $1); }
;

/*Declarators*/
VarDec : ID { $$ = createNode("VarDec", "", 1, @$.first_line, 1, $1); }
  | VarDec LB INT RB { $$ = createNode("VarDec", "", 1, @$.first_line, 4, $1, $2, $3, $4); }
  | VarDec LB error RB { errorNum++; }
;
FunDec : ID LP VarList RP { $$ = createNode("FunDec", "", 1, @$.first_line, 4, $1, $2, $3, $4); }
  | ID LP RP { $$ = createNode("FunDec", "", 1, @$.first_line, 3, $1, $2, $3); }
  | ID error RP { errorNum++; }
;
VarList : ParamDec COMMA VarList { $$ = createNode("VarList", "", 1, @$.first_line, 3, $1, $2, $3); }
  | ParamDec { $$ = createNode("VarList", "", 1, @$.first_line, 1, $1); }
;
ParamDec : Specifier VarDec { $$ = createNode("ParamDec", "", 1, @$.first_line, 2, $1, $2); }
;

/*Statements*/
CompSt : LC DefList StmtList RC { $$ = createNode("CompSt", "", 1, @$.first_line, 4, $1, $2, $3, $4); }
;
StmtList : /* empty */{ $$ = createNode("", "", 1, @$.first_line, 0); }
  | Stmt StmtList { $$ = createNode("StmtList", "", 1, @$.first_line, 2, $1, $2); }
;
Stmt : Exp SEMI { $$ = createNode("Stmt", "", 1, @$.first_line, 2, $1, $2); }
  | CompSt { $$ = createNode("Stmt", "", 1, @$.first_line, 1, $1); }
  | RETURN Exp SEMI { $$ = createNode("Stmt", "", 1, @$.first_line, 3, $1, $2, $3); }
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$ = createNode("Stmt", "", 1, @$.first_line, 5, $1, $2, $3, $4, $5); }
  | IF LP Exp RP Stmt ELSE Stmt { $$ = createNode("Stmt", "", 1, @$.first_line, 7, $1, $2, $3, $4, $5, $6, $7); }
  | WHILE LP Exp RP Stmt { $$ = createNode("Stmt", "", 1, @$.first_line, 5, $1, $2, $3, $4, $5); }
  | Exp error { errorNum++; }
  | RETURN Exp error { errorNum++; }
  | error SEMI { errorNum++; }
;

/*Local Definitions*/
DefList : /* empty */{ $$ = createNode("", "", 1, @$.first_line, 0); }
  | Def DefList { $$ = createNode("DefList", "", 1, @$.first_line, 2, $1, $2); }
;
Def : Specifier DecList SEMI { $$ = createNode("Def", "", 1, @$.first_line, 3, $1, $2, $3); }
;
DecList : Dec { $$ = createNode("DecList", "", 1, @$.first_line, 1, $1); }
  | Dec COMMA DecList { $$ = createNode("DecList", "", 1, @$.first_line, 3, $1, $2, $3); }
 ;
Dec : VarDec { $$ = createNode("Dec", "", 1, @$.first_line, 1, $1); }
  | VarDec ASSIGNOP Exp { $$ = createNode("Dec", "", 1, @$.first_line, 3, $1, $2, $3); }
;

/* Expressions*/
Exp : Exp ASSIGNOP Exp { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Exp AND Exp { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Exp OR Exp { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Exp RELOP Exp { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Exp PLUS Exp { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Exp MINUS Exp { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Exp STAR Exp { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Exp DIV Exp { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | LP Exp RP { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | MINUS Exp { $$ = createNode("Exp", "", 1, @$.first_line, 2, $1, $2); }
  | NOT Exp { $$ = createNode("Exp", "", 1, @$.first_line, 2, $1, $2); }
  | ID LP Args RP { $$ = createNode("Exp", "", 1, @$.first_line, 4, $1, $2, $3, $4); }
  | ID LP RP { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Exp LB Exp RB { $$ = createNode("Exp", "", 1, @$.first_line, 4, $1, $2, $3, $4); }
  | Exp DOT ID { $$ = createNode("Exp", "", 1, @$.first_line, 3, $1, $2, $3); }
  | ID { $$ = createNode("Exp", "", 1, @$.first_line, 1, $1); }
  | INT { $$ = createNode("Exp", "", 1, @$.first_line, 1, $1); }
  | FLOAT { $$ = createNode("Exp", "", 1, @$.first_line, 1, $1); }
  | Exp LB error RB { errorNum++; }
;
Args : Exp COMMA Args { $$ = createNode("Args", "", 1, @$.first_line, 3, $1, $2, $3); }
  | Exp { $$ = createNode("Args", "", 1, @$.first_line, 1, $1); }
;

%%


