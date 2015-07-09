#ifndef __MIPS_H__
#define __MIPS_H__

#include "intercode.h"

struct MIPSCode
{
	char* code;
	struct MIPSCode* next;
	int kind;//0 is data, 1 is text
};
typedef struct MIPSCode MIPSCode;

struct Table
{
	char* uname;
	int pos;
};
typedef struct Table Table;

int getPosition(char*);
MIPSCode* newMIPSCode(char*, int);
void makeDataLink(MIPSCode*);
void makeTextLink(MIPSCode*);
void init();
void writeTOFile(char*);

void mipsLinkedInterCode(LinkedInterCode*);
void mipsInterCode(InterCode*);

void mipsBinop(InterCode*);
void mipsReturn(InterCode*);
void mipsTriop(InterCode*);
void mipsLabel(InterCode*);
void mipsCond(InterCode*);
void mipsDec(InterCode*);
void mipsGoto(InterCode*);
void mipsRead(InterCode*);
void mipsWrite(InterCode*);
void mipsParam(InterCode*);
void mipsCall(InterCode*);
void mipsArg(InterCode*);
void mipsFunction(InterCode*);

void load(Operand*, int);
void store(Operand*, int);

#endif