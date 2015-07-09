%{
#include "tree.h"//-_-|||顺序
#include "syntax.tab.h"

int yycolumn = 1;
#define YY_USER_ACTION \
yylloc.first_line = yylloc.last_line = yylineno; \
yylloc.first_column = yycolumn; \
yylloc.last_column = yycolumn+yyleng-1; \
yycolumn += yyleng;

int errorA = 0; 

%}

digit0 [0-9]
digit1 [1-9]
intNumber (0|({digit1}{digit0}*))
octIntNumber 0[0-7]+
hexIntNumber 0[xX][0-9a-fA-F]+
floatNumber ({digit0}*\.?{digit0}+|{digit0}+\.)
expFloatNumber {digit0}+(.{digit0}+)?([eE][+-]?{digit0}+)
letter [a-zA-Z]
letter_ [_a-zA-Z]
id {letter_}({letter_}|{digit0})*
delim [" "\t]
comment1 (\/\/[^\r\n]*)+
comment2 "/*"([*]*(([^*/])+([/])*)*)*"*/" 

wrongOct 0([0-7]*[8-9]+[0-7]*)+
wrongHex 0[xX]([0-9a-fA-F]*[g-zG-Z]+[0-9a-fA-F]*)+
wrongExpFloat1 {digit0}+(.{digit0}+)?([eE][+-]?)
wrongExpFloat2 [eE][+-]?{digit0}+
wrongId [1-9]{id}+

%option yylineno
%%
{wrongOct} { printf("Error type A at Line %d: Illegal octal number \'%s\'.\n", yylineno, yytext); yylval.type_node = createNode("INT", yytext, 0, yylineno, 0); ++errorA; return INT; }
{wrongHex} { printf("Error type A at Line %d: Illegal hexical number \'%s\'.\n", yylineno, yytext); yylval.type_node = createNode("INT", yytext, 0, yylineno, 0); ++errorA; return INT; }
{wrongExpFloat1}|{wrongExpFloat2} { printf("Error type A at Line %d: Illegal floating point number \'%s\'.\n", yylineno, yytext); yylval.type_node = createNode("FLOAT", yytext, 0, yylineno, 0); ++errorA; return FLOAT; }
{wrongId} { printf("Error type A at Line %d: Wrong id \'%s\'.\n", yylineno, yytext); yylval.type_node = createNode("ID", yytext, 0, yylineno, 0); ++errorA; return ID; }

struct { yylval.type_node = createNode("STRUCT", yytext, 0, yylineno, 0); return STRUCT; }
return { yylval.type_node = createNode("RETURN", yytext, 0, yylineno, 0); return RETURN; }
if { yylval.type_node = createNode("IF", yytext, 0, yylineno, 0); return IF; }
else { yylval.type_node = createNode("ELSE", yytext, 0, yylineno, 0); return ELSE; }
while { yylval.type_node = createNode("WHILE", yytext, 0, yylineno, 0); return WHILE; }
int|float { yylval.type_node = createNode("TYPE", yytext, 0, yylineno, 0); return TYPE;}
{delim} { /*Do nothing*/ }
{intNumber} { yylval.type_node = createNode("INT", yytext, 0, yylineno, 0); return INT; }
{octIntNumber} { yylval.type_node = createNode("INT", yytext, 0, yylineno, 0); return INT; }
{hexIntNumber} { yylval.type_node = createNode("INT", yytext, 0, yylineno, 0); return INT; }
{floatNumber} { yylval.type_node = createNode("FLOAT", yytext, 0, yylineno, 0); return FLOAT; }
{expFloatNumber} { yylval.type_node = createNode("FLOAT", yytext, 0, yylineno, 0); return FLOAT; }
{id} { yylval.type_node = createNode("ID", yytext, 0, yylineno, 0); return ID; }
";" { yylval.type_node = createNode("SEMI", yytext, 0, yylineno, 0); return SEMI; }
"," {yylval.type_node = createNode("COMMA", yytext, 0, yylineno, 0); return COMMA; }
"=" { yylval.type_node = createNode("ASSIGNOP", yytext, 0, yylineno, 0); return ASSIGNOP; }
>|<|>=|<=|==|!= { yylval.type_node = createNode("RELOP", yytext, 0, yylineno, 0); return RELOP; }
"+" { yylval.type_node = createNode("PLUS", yytext, 0, yylineno, 0); return PLUS; }
"-" { yylval.type_node = createNode("MINUS", yytext, 0, yylineno, 0); return MINUS; }
"*" { yylval.type_node = createNode("STAR", yytext, 0, yylineno, 0); return STAR; }
"/" { yylval.type_node = createNode("DIV", yytext, 0, yylineno, 0); return DIV; }
"&&" { yylval.type_node = createNode("AND", yytext, 0, yylineno, 0); return AND; }
"||" { yylval.type_node = createNode("OR", yytext, 0, yylineno, 0); return OR; }
"." { yylval.type_node = createNode("DOT", yytext, 0, yylineno, 0); return DOT; }
"!" { yylval.type_node = createNode("NOT", yytext, 0, yylineno, 0); return NOT; }
"(" { yylval.type_node = createNode("LP", yytext, 0, yylineno, 0); return LP; }
")" { yylval.type_node = createNode("RP", yytext, 0, yylineno, 0); return RP; }
"[" { yylval.type_node = createNode("LB", yytext, 0, yylineno, 0); return LB; }
"]" { yylval.type_node = createNode("RB", yytext, 0, yylineno, 0); return RB; }
"{" { yylval.type_node = createNode("LC", yytext, 0, yylineno, 0); return LC; }
"}" { yylval.type_node = createNode("RC", yytext, 0, yylineno, 0); return RC; }
{comment1}|{comment2} { /*Comment. Do nothing.*/ /*Comment2: refer to http://bbs.csdn.net/topics/40224740 */}
\n { yycolumn = 1; }
. { printf("Error type A at Line %d: Mysterious characters \'%s\'.\n", yylineno, yytext); ++errorA; }
%%




