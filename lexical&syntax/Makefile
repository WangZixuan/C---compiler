lex:
	flex lexical.l
syn:
	bison -d syntax.y
compile:
	gcc tree.c source.c syntax.tab.c -lfl -w -o parser
play:
	./parser test.c
