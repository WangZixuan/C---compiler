This is a C-- compiler, copyright by Zixuan Wang.

Usage:
	make 
	./parser XXX1 XXX2
XXX1 is your C file.
XXX2 is your output assembly file.

Files in this folder:
	lexical.l:		Used in lexical analysis, based on Flex 2.5.35.
	syntax.y:		Used in syntax analysis, based on Bison 3.0.2.
	tree.h:			Header for tree.c.
	tree.c:			Source for creating and displaying grammar tree.
	grammar.h:		Header for grammar.c.
	grammar.c:		Source for checking if the file has grammar errors.
	intercode.h:	Header for intercode.c.
	intercode.c:	Source for generating 3-addr codes.
	mips.h:			Header for mips.c.
	mips.c:			Source for generating mips codes.
	source.c:		Access to function main().
	Makefile:		Compiling rules.
	parser 			A runnable application.
	ReadMe.txt:		Guidance for the program, and it is me myself.:-)
	