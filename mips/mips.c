#include "mips.h"
#define MALLOCSIZE 32
#define REGSIZE 32
#define USESIZE 32
#define DEBUG 0

MIPSCode* dataHead = 0;
MIPSCode* textHead = 0;
MIPSCode* textTail = 0;

Table searchTable[REGSIZE];
int unused[USESIZE];

int useIndex = 0;
int regIndex = 0;
int paramPos = 16;
int argPos = 0;
int isMain = -1;//0 means it is main function, and 1 means it is not.

int getPosition(char* uname)
{
	int i;
	for(i = 0; i < regIndex; i++){
		if(searchTable[i].uname != 0 && strcmp(searchTable[i].uname, uname) == 0)
			break;
	}
	
	if(i == regIndex)
	{
		searchTable[regIndex].uname = strdup(uname);
		searchTable[regIndex].pos = unused[useIndex];
		regIndex += 1;
		unused[useIndex] += 4;
	}
	return searchTable[i].pos;
}

MIPSCode* newMIPSCode(char* name, int kind)
{
	MIPSCode *temp = (MIPSCode *)malloc(sizeof(MIPSCode));
	temp->code = strdup(name);
	temp->next = 0;
	temp->kind = kind;
	assert(kind == 0 || kind == 1);
	return temp;
}

void makeDataLink(MIPSCode* m)
{
	//Insert in the head
	assert(m->kind == 0);
	if (0 == dataHead)
		dataHead = m;
	else
	{
		m->next = dataHead;
		dataHead = m;
	}
	return;
}

void makeTextLink(MIPSCode* m)
{
	assert(m->kind == 1);
	if (0 == textHead)
	{
		textHead = m;
		textTail = m;
	}
	else
	{
		textTail->next = m;
		textTail = m;
	}
	return;
}

void init()
{
	MIPSCode* dataInitial = newMIPSCode("_prompt: .asciiz \"Enter a integer:\"\n_ret: .asciiz \"\\n\"", 0);
	makeDataLink(dataInitial);

	MIPSCode* textInitial = newMIPSCode("read:\n  li $v0, 4\n  la $a0, _prompt\n  syscall\n  li $v0, 5\n  syscall\n  move $t0, $v0\n  jr $ra\n", 1);
	makeTextLink(textInitial);
	textInitial = newMIPSCode("write:\n  li $v0, 1\n  syscall\n  li $v0, 4\n  la $a0, _ret\n  syscall\n  move $v0, $0\n  jr $ra\n", 1);
	makeTextLink(textInitial);
	
	return;
}

void writeToFile(char* fileName)
{
	FILE* fp = fopen(fileName, "w");
	if (!fp)
	{
		printf("Error openning file!\n");
		exit(-1);
	}

	init();
	mipsLinkedInterCode(licHead);
	MIPSCode* temp = 0;

	fprintf(fp, ".data\n");

	for(temp = dataHead; temp != 0; temp = temp->next)
		fprintf(fp, "%s\n", temp->code);

	fprintf(fp, "%s\n", ".globl main");

	fprintf(fp, ".text\n");

	for(temp = textHead; temp != 0; temp = temp->next)
		fprintf(fp, "%s\n", temp->code);

	return;
}

void mipsLinkedInterCode(LinkedInterCode* lic)
{
	while(lic != 0)
	{
		mipsInterCode(lic->code);
		lic = lic->next;
	}
	return;
}

void mipsInterCode(InterCode* ic){
	if (0 == ic) return;
	switch(ic->kind) 
	{
		case FUNCTION_IC:			mipsFunction(ic); break;
		case PARAM_IC: 				mipsParam(ic);break;
		case LABEL_IC: 				mipsLabel(ic);break;
		case LABEL_GOTO_IC: 		mipsGoto(ic); break;
		case CALLFUNC_IC: 			mipsCall(ic); break;
		case RETURN_IC: 			mipsReturn(ic); break;
		case WRITE_IC: 				mipsWrite(ic); break;
		case READ_IC: 				mipsRead(ic); break;
		case ARG_IC: 				mipsArg(ic); break;
		case DEC_IC:				mipsDec(ic); break;
		case COND_IC: 				mipsCond(ic); break;
		case ADD_IC:	
		case SUB_IC:	
		case MUL_IC:	
		case DIV_IC:				mipsTriop(ic); break;
		case ASSIGN_IC: 			mipsBinop(ic); break;
		default: 
			printf("error : unknown ic->kind %d\n", ic->kind);
			exit(0); 
	}
	return;
}

void mipsBinop(InterCode* ic)
{
	if (DEBUG)
		printf("In binop\n");
	//Assign
	assert(ic->kind == ASSIGN_IC);
	load(ic->u.binop.op, 0);
	store(ic->u.binop.result, 0);
}

void mipsReturn(InterCode* ic)
{
	if (DEBUG)
		printf("In return\n");
	assert(ic->kind == RETURN_IC);
	load(ic->u.ret.op, 0);
	MIPSCode* temp = 0;

	if (1 == isMain)
	{
		//Not main function.
		temp = newMIPSCode("  lw $sp, 4($fp)", 1);
		makeTextLink(temp);

		temp = newMIPSCode("  lw $fp, 8($fp)", 1);
		makeTextLink(temp);
		paramPos = 16;//Reset.
	}

	temp = newMIPSCode("  jr $ra", 1);
	makeTextLink(temp);

	return;
}

void mipsTriop(InterCode* ic)
{
	if (DEBUG)
		printf("In calc\n");
	assert(ic->kind == ADD_IC || ic->kind == SUB_IC || ic->kind == MUL_IC || ic->kind == DIV_IC);

	load(ic->u.triop.op1, 0);
	load(ic->u.triop.op2, 1);

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);//20
	switch(ic->kind)
	{
		case ADD_IC: sprintf(name, "  %s $t1, $t0, $t1", "add"); break;
		case SUB_IC: sprintf(name, "  %s $t1, $t0, $t1", "sub"); break;
		case MUL_IC: sprintf(name, "  %s $t1, $t0, $t1", "mul"); break;
		case DIV_IC: sprintf(name, "  %s $t1, $t0, $t1", "div"); break;
	}
	MIPSCode* temp = newMIPSCode(name, 1);
	makeTextLink(temp);
	store(ic->u.triop.result, 1);

	return;
}

void mipsLabel(InterCode* ic)
{
	if (DEBUG)
		printf("In label\n");
	assert(ic->kind == LABEL_IC);

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);//8
	sprintf(name, "%s:" , ic->u.label.name);

	MIPSCode* temp = newMIPSCode(name, 1);
	makeTextLink(temp);
	return;
}

void mipsCond(InterCode* ic)
{
	if (DEBUG)
		printf("In cond\n");
	assert(ic->kind == COND_IC);


	load(ic->u.cond.op1, 0);
	load(ic->u.cond.op2, 1);

	char* relop = 0;

	if (0 == strcmp(ic->u.cond.op, ">"))
		relop = strdup("bgt");
	else if (0 == strcmp(ic->u.cond.op, "<"))
		relop = strdup("blt");
	else if (0 == strcmp(ic->u.cond.op, ">="))
		relop = strdup("bge");
	else if (0 == strcmp(ic->u.cond.op, "<="))
		relop = strdup("ble");
	else if (0 == strcmp(ic->u.cond.op, "=="))
		relop = strdup("beq");
	else if (0 == strcmp(ic->u.cond.op, "!="))
		relop = strdup("bne");
	else 
		printf("unknown relop type %s\n", ic->u.cond.op);
	

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);
	sprintf(name, "  %s $t0, $t1, %s", relop, ic->u.cond.name);

	MIPSCode* temp = newMIPSCode(name, 1);
	makeTextLink(temp);
	return;
} 

void mipsDec(InterCode* ic)
{
	if (DEBUG)
		printf("In dec\n");
	assert(ic->kind == DEC_IC);

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);	
	sprintf(name, "v%d : .space %d", ic->u.dec.op->u.value, ic->u.dec.size);

	MIPSCode* temp = newMIPSCode(name, 0);
	makeDataLink(temp);
	return;
}

void mipsGoto(InterCode* ic)
{
	if (DEBUG)
		printf("In goto\n");
	assert(ic->kind == LABEL_GOTO_IC);

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);
	sprintf(name, "  j %s", ic->u.label_goto.name);

	MIPSCode* temp = newMIPSCode(name, 1);
	makeTextLink(temp);
	return;
}

void mipsRead(InterCode* ic)
{
	if (DEBUG)
		printf("In read\n");
	assert(ic->kind == READ_IC);
	MIPSCode* temp = newMIPSCode("  sw $ra, 0($sp)", 1);
	makeTextLink(temp);

	temp = newMIPSCode("  jal read", 1);
	makeTextLink(temp);

	temp = newMIPSCode("  lw $ra, 0($sp)", 1);
	makeTextLink(temp);

	store(ic->u.read.op, 0);	
	
	return;
}

void mipsWrite(InterCode* ic)
{
	if (DEBUG)
		printf("In write\n");
	assert(ic->kind == WRITE_IC);

	load(ic->u.write.op, 0);

	MIPSCode* temp = newMIPSCode("  move $a0, $t0", 1);
	makeTextLink(temp);

	temp = newMIPSCode("  sw $ra, 0($sp)", 1);
	makeTextLink(temp);

	temp = newMIPSCode("  jal write", 1);
	makeTextLink(temp);

	temp = newMIPSCode("  lw $ra, 0($sp)", 1);
	makeTextLink(temp);
	return;
}

void mipsParam(InterCode* ic)
{
	if (DEBUG)
		printf("In param\n");
	assert(ic->kind == PARAM_IC);

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);
	sprintf(name, "  lw $t0, %d($fp)", paramPos);

	MIPSCode* temp = newMIPSCode(name, 1);
	makeTextLink(temp);

	Operand* paramOp = (Operand*)malloc(sizeof(Operand));
	paramOp->kind = VARIABLE_OP;
	paramOp->u.name = strdup(ic->u.param.name);
	store(paramOp, 0);
	//store(ic->u.param.op, 0);
	paramPos += 4;
	return;
}

void mipsCall(InterCode* ic)
{
	if (DEBUG)
		printf("In call\n");
	assert(ic->kind == CALLFUNC_IC);

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);
	sprintf(name, "  sw $ra, -%d($sp)", argPos);
	MIPSCode* temp = newMIPSCode(name, 1);
	makeTextLink(temp);

	memset(name, 0, sizeof(name));
	sprintf(name, "  sw $fp, -%d($sp)", argPos + 4);
	temp = newMIPSCode(name, 1);
	makeTextLink(temp);

	memset(name, 0, sizeof(name));
	sprintf(name, "  sw $sp, -%d($sp)", argPos + 8);
	temp = newMIPSCode(name, 1);
	makeTextLink(temp);

	memset(name, 0, sizeof(name));
	sprintf(name, "  addi $sp, $sp, -%d", argPos + 12);
	temp = newMIPSCode(name, 1);
	makeTextLink(temp);

	memset(name, 0, sizeof(name));
	sprintf(name, "  jal %s", ic->u.call.name);
	temp = newMIPSCode(name, 1);
	makeTextLink(temp);

	memset(name, 0, sizeof(name));
	sprintf(name, "  lw $ra, -%d($sp)", argPos);
	temp = newMIPSCode(name, 1);
	makeTextLink(temp);
	
	store(ic->u.call.op, 0);
	
	argPos = 0;
	return;
}


void mipsArg(InterCode* ic)
{
	if (DEBUG)
		printf("In arg\n");
	assert(ic->kind == ARG_IC);

	load(ic->u.arg.op, 0);

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);
	sprintf(name, "  sw $t0, -%d($sp)", argPos);
	
	MIPSCode* temp = newMIPSCode(name, 1);	
	makeTextLink(temp);
	argPos += 4;
	return;
}


void mipsFunction(InterCode* ic)
{
	if (DEBUG)
		printf("In function\n");
	assert(ic->kind == FUNCTION_IC);

	useIndex++;

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);
	sprintf(name, "%s:", ic->u.function.name);

	MIPSCode* temp = newMIPSCode(name, 1);
	makeTextLink(temp);

	//Whether it is main function.
	if(0 == strcmp(ic->u.function.name, "main"))
		isMain = 0;
	else
		isMain = 1;

	temp = newMIPSCode("  move $fp, $sp", 1);
	makeTextLink(temp);
	return;
}

void load(Operand* op, int reg)
{
	if (DEBUG)
		printf("In load\n");

	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);
	MIPSCode* temp = 0;

	switch(op->kind)
	{
		case VARIABLE_OP:
			sprintf(name, "  lw $t%d, -%d($fp)", reg, getPosition(op->u.name));
			temp = newMIPSCode(name, 1);
			makeTextLink(temp);
			break;
		case CONSTANT_OP:
			sprintf(name, "  li $t%d, %d", reg, op->u.value);
			temp = newMIPSCode(name, 1);
			makeTextLink(temp);
			break;
		default:
			printf("No array!\n");
			break;
	}
	return;
}

void store(Operand* op, int reg)
{
	if (DEBUG)
		printf("In store\n");
	if (op->kind == ARRAY_OP || op->kind == ADDRESS_OP)
	{
		printf("Array not support!\n");
		exit(-1);
	}

	assert(op->kind == VARIABLE_OP);
	int preUnused = unused[useIndex];
	
	char* name = (char*)malloc(sizeof(char) * MALLOCSIZE);
	sprintf(name, "  sw $t%d, -%d($fp)", reg, getPosition(op->u.name));
	MIPSCode* temp = newMIPSCode(name, 1);
	makeTextLink(temp);

	if(preUnused < unused[useIndex])
	{
		temp = newMIPSCode("  addi $sp, $sp, -4", 1);
		makeTextLink(temp);
	}

	return;
}

	
