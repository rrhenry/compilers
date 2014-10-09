#include <stdlib.h>		// needed for malloc (at least)
#include <stdio.h>		// need for file io

typedef enum { 	
				module, 
				ident, letter, digit,
			 	importList, import,
			 	declarationSequence, constantDeclaration,
			 	identdef,
			 	constExpression, expression, simpleExpression,
			 	term, factor, number, integer, hexDigit, real, 
			 	scaleFactor, charConstant, string,
			 	set, element, 
			 	designator, expList, actualParameters,
			 	mulOperator, addOperator, relation, 
			 	typeDeclaration, type, qualident, arrayType, length, recordType, baseType,
			 	variableDeclaration, procedureDeclaration, procedureHeading, formalParameters,
			 	fpsSection, formalType, prodecureBody, forwardDeclaration,
			 	statementSequence, statement, assignment, procedureCall, 
			 	ifStatement, caseStatement, _case, caseLabelList, caseLabels,
			 	whileStatement, repeatStatement, loopStatement, withStatement 
			} Token;

Token currTok;
const int BUFF_SIZE = 80;		// if you change this pls change currLine's size
char currChar;
char currLine [80];		// 80 character limit is arbitrary but sensible.
								// should be pointer...?
int count = 0;
FILE *toScan;

//	*
//	Get a character and put it in currChar.
//	-- might be useless
//	-- might want to grab from currLine and put into currChar?
//	++ probably not though. We need to fetch a character based on a counter, right?
//  ++ we don't necessarily want that function in getLine? 
void getChar()
{
	if(count >= 80) //ok i'm assuming currline's size is 80 characters total
		count = 0; 	//starting at index 0, and will be over limit at or abover 80

	currChar = currline[count]; 
}

//	*
//	Get a line and put it in currLine.
//
void getLine()
{
	char *funLine = (char *) malloc(BUFF_SIZE);
	if ( fgets(funLine, BUFF_SIZE, toScan) != NULL )
		fputs(funLine, stdout);

}

int main( void )
{	
	// so cleeeeaaaaaan

	toScan = fopen("testFile.txt", "r");		// ?

	if (toScan != NULL)
	{
		getLine();
		getLine();
		printf("Lol\n");
		getLine();
	}

	fclose(toScan);
	return 42;
}