#include "tree.h"

struct Node* createNode(char* text, char* val, unsigned int leaf, unsigned int line, unsigned int sonNum, ...)
{
	unsigned int i = 0;
	va_list argPtr;
	struct Node* oneSon;

	struct Node* thisNode = (struct Node*)malloc(sizeof(struct Node));
	memset(thisNode, 0, sizeof(struct Node));

	thisNode->lineno = line;// yylineno;
	thisNode->isLeaf = leaf;
	thisNode->name = (char*)malloc(strlen(text) + 1);
	strcpy(thisNode->name, text);
	thisNode->value = (char*)malloc(strlen(val) + 1);
	strcpy(thisNode->value, val);
	thisNode->sonNum = sonNum;
	//Deal with multi-args.
	if (sonNum > 0)
	{
		thisNode->sons = (struct Node**)malloc(sizeof(struct Node*) * sonNum);
		va_start(argPtr, sonNum);
		while (i < sonNum)
		{
			oneSon = va_arg(argPtr, struct Node*);
			thisNode->sons[i] = oneSon;
			i++;
		}
		va_end(argPtr);
	}
	else
		thisNode->sons = NULL;

	return thisNode;
}

void displayTree(struct Node* root)
{
	//Initialization.
	
	struct displayNode* head = (struct displayNode*)malloc(sizeof(struct displayNode));
	head->disNode = root;
	head->layer = 0;
	head->next = NULL;
	struct displayNode* tail = head;
	struct displayNode* current = head;
	struct displayNode* p = head;
	unsigned int lineCount = 1;

	while (1)
	{
		int i = 0;

		p = head;
		for (i = current->disNode->sonNum - 1; i > -1; --i)
		{
			//Add a node in front of the link.
			p = (struct displayNode*)malloc(sizeof(struct displayNode));
			p->disNode = current->disNode->sons[i];
			p->layer = current->layer + 1;
			p->next = head;
			head = p;

		}

		if (strlen(current->disNode->name) > 0 && errorA == 0)
		//Display and remove current.
		{	
			printf("%3d", lineCount++);	
			for (i = 0; i < current->layer; ++i)
				printf("  ");
			printf("%s", current->disNode->name);
			if (current->disNode->isLeaf == 1)//Not leaf.
				printf(" (%d)", current->disNode->lineno);

			if (strlen(current->disNode->value) > 0)
				if (0 == strcmp(current->disNode->name, "INT"))//Print int.
					if (strlen(current->disNode->value) == 1)
						printf(": %d", strtol(current->disNode->value, NULL, 10));
					else if (current->disNode->value[0] == '0')
						if (current->disNode->value[1] == 'x' || current->disNode->value[1] == 'X')//Hex
							printf(": %d", strtol(current->disNode->value, NULL, 16));
						else //Oct
							printf(": %d", strtol(current->disNode->value, NULL, 8));
					else
						printf(": %d", strtol(current->disNode->value, NULL, 10));
				else if (0 == strcmp(current->disNode->name, "FLOAT"))//Print float.
					printf(": %f", atof(current->disNode->value));
				else if (0 == strcmp(current->disNode->name, "TYPE") || 0 == strcmp(current->disNode->name, "ID"))
					printf(": %s", current->disNode->value);

			printf("\n");

		}

		p = head;
		if (head == tail)
		{
			free(head);
			break;
		}
		else
		{
			if (p == current)//Current is head.
			{
				current = head->next;
				head = current;
				free(p);
			}
			else
			{
				while (p->next != current && p->next != NULL)
					p = p->next;

				p->next = current->next;
				if (p->next == NULL)//Tail.
					tail = p;
				free(current);
				current = head;
			}
		}

		
	} 


}

