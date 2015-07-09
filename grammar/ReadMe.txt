This is a C-- lexical and syntax analyzer, copyright by Zixuan Wang.

Usage:
	make lex
	make syn
	make compile
	./parser XXX
XXX is your C file.

Files in this folder:
	lexical.l:	Used in lexical analysis, based on Flex 2.5.35.
	syntax.y:	Used in syntax analysis, based on Bison 3.0.2.
	tree.h:		Header for tree.c.
	tree.c:		Source for creating and displaying grammar tree.
	source.c:	Access to function main().
	test.c:		A test C file, and is XXX mentioned above.
	Makefile:	Compiling rules.
	parser 		A runnable application.
	ReadMe.txt:	Guidance for the program, and it is me myself.:-)
	
