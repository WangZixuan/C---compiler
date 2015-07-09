#include "stdio.h"
#include "tree.h"
#include "syntax.tab.h"
//#include "symbol.h"
#include "grammar.h"
#include "intercode.h"
extern FILE* yyin;
extern int errorNum;
extern struct YYLTYPE;
extern YYLTYPE yylloc;
extern struct Node;
extern struct Node* root;

int main(int argc, char** argv)
{
	if (argc <= 2)
	{
		printf("Usage: ./parser xx.c xx.ir\n");
		return -1;
	}
	yyin = fopen(argv[1], "r");
	if (!yyin)
	{
		perror(argv[1]);
		return 1;
	}
	//yyrestart(f);
	yyparse();

	//if (0 == errorNum)
	//	displayTree(root);

	//program(root);
	//printf("Over over.\n");

	translateProgram(root);
	//printf("translation over\n");

	//printTable();

	printInFile(argv[2]);


	return 0;
}



yyerror(char* msg)
{
	 printf("Error type B at Line %d: %s\n", yylloc.first_line, msg);
}
