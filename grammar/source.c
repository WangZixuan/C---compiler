#include "stdio.h"
#include "tree.h"
#include "syntax.tab.h"
//#include "symbol.h"
#include "grammar.h"
extern FILE* yyin;
extern int errorNum;
extern struct YYLTYPE;
extern YYLTYPE yylloc;
extern struct Node;
extern struct Node* root;

int main(int argc, char** argv)
{
	if (argc <= 1) return 1;
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

	program(root);


	return 0;
}



yyerror(char* msg)
{
	 printf("Error type B at Line %d: %s\n", yylloc.first_line, msg);
}
