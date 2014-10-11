#include <stdio.h>		// need for file io

typedef enum { 								// OBERON 2, not OBERON S
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
int count = -1;					// Global counter for current line position
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

int isSep(char aChar)
{
	if (aChar == ' ' || aChar == '\t')			// if its a space
		return 1;
	return 0;
}

//	*
//	Get a line and put it in currLine.
//
void getLine()
{
	
	fgets(currLine, BUFF_SIZE, toScan);
	fputs(currLine, stdout); 					// Debug MSG

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
	//ok i'm assuming currline's size is 80 characters total
	//starting at index 0, and will be over limit at or abover 80
	//this is sorta shit implementation, but i don't have a clear idea what will be 
	//calling this yet, so for now, it will collect all the characters as it gets 
	//called and when it reaches 80, it'll restart, so better hope we have a new line 
	//by then.
	if(count >= BUFF_SIZE || count == -1) 
	{
		count = 0;
		getLine(); 	
	}

	currChar = currLine[count]; 
	count ++;
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


//
//	Return the next symbol.
//
Token nextSym()
{
	getChar();					// get first character initally



}

//
//	Scans the file and outputs the tokens to the screen.
//
void scan()
{
	// getChar();				// gets the first character



}

int main( int argc, char *argv[] )
{	
	if ( argc == 2)		// we need 1 file to open, specified on command line, so 2 command line args
	{
		toScan = fopen(argv[1], "r");		// assume second value is toParse file name

		if (toScan != NULL)
		{
			scan();
		}

		fclose(toScan);
	}

	return 0;
}