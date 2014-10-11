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
const int BUFF_SIZE = 256;		// if you change this pls change currLine's size
char currChar;
char currLine [256];			// 256 character limit is arbitrary but sensible.
								// should be pointer...?
int count = 0;
FILE *toScan;

int isDigit(char aChar)
{
	if ( aChar >= '0' && aChar <= '9')
	{
		return 1;
	}

	return 0;
}

int isAlpha(char aChar)
{
	if ((aChar >= 'a' && aChar <= 'z') || (aChar >= 'A' && aChar <= 'Z'))
		return 1;

	return 0;
}

int isHexDigit(char aChar)
{
	if (isDigit(aChar) || aChar == 'A' || aChar == 'B' || aChar == 'C' || aChar == 'D' || aChar == 'E' || aChar == 'F')
		return 1;

	return 0;
}


//	*
//	Get a character and put it in currChar.
//	-- might be useless
//	-- might want to grab from currLine and put into currChar?
//	++ probably not though. We need to fetch a character based on a counter, right?
//  ++ we don't necessarily want that function in getLine? 
//
void getChar()
{
	if(count >= BUFF_SIZE) //ok i'm assuming currline's size is BUFF_SIZE characters total
		count = 0; 	//starting at index 0, and will be over limit at or above BUFF_SIZE
	//this is sorta shit implementation, but i don't have a clear idea what will be 
	//calling this yet, so for now, it will collect all the characters as it gets 
	//called and when it reaches 80, it'll restart, so better hope we have a new line 
	//by then.

	currChar = currLine[count]; 
	count ++;
}

//	*
//	Get a line and put it in currLine.
//
void getLine()
{
	char *funLine = (char *) malloc(BUFF_SIZE);
	if ( fgets(funLine, BUFF_SIZE, toScan) != NULL ){
		fputs(funLine, stdout);
		*currLine = *funLine;
	}

}

// *
// Determines if currChar is a digit (0...9) or a letter (a...z | A...Z)
// sets currTok to relevant state
// -- might merge this with getChar because who wants to call this everytime
//
void charType()
{
	if(isDigit(currChar))
		currTok = digit;
	else if (isAlpha(currChar))
		currTok = letter;
}

int main( int argc, char *argv[] )
{	
	if ( argc == 2)		// we need 1 file to open, specified on command line, so 2 command line args
	{
		toScan = fopen(argv[1], "r");		// assume second value is toParse file name

		if (toScan != NULL)
		{
			getLine();
			getLine();
			//printf("Lol\n");
			getLine();
		}

		if (isDigit('0'))
			printf("Lool\n");

		if (isAlpha('a'))
			printf("No\n");

		fclose(toScan);
	}

	return 42;
}