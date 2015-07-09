compile:
	flex lexical.l
	bison -d syntax.y
	gcc tree.c grammar.c intercode.c source.c syntax.tab.c -lfl -w -o parser
clean:
	rm lex.yy.c syntax.tab.h syntax.tab.c 
play:
	./parser test.c output.ir
