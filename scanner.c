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
char currWord [64];				//Word being worked on. Delimited by found whitespaces 
								//and to be compared to a table of reserved words
Token setTok;

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
	//ok i'm assuming currline's size is Buff_size characters total
	//starting at index 0, and will be over limit at or above buffSize
	//this is sorta shit implementation, but i don't have a clear idea what will be 
	//calling this yet, so for now, it will collect all the characters as it gets 
	//called and when it reaches Buff_size, it'll restart, so better hope we have a new line 
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
	getChar();					//	get first character initally//
	charType();					//	S0, determing character type//
	
	switch (currTok) 
	{
		case digit: 			//	S1 starts looking for reals and ints//
			
			//	S2 keeps looking for a digit until it finds either
			// '.', whitespace or a hexdigit 
			// '.' can branch to REAL after a digit or
			//	will lead to three more states until it reaches REAL
			//	whitespace leads straight to an asignment to INT
			//	hexdigit will keep looking for hexdigit until H or X are found
			//	H will lead to INT and X should lead to STRING. 

			break;



		case '\"': 				//	S8 starts to look for a string//
			
			//	Keep fetching the next character until you find the
			//	next quote. Upon finding, it, set setTok to String
			while( currChar != '\"')
			{
				getChar();
			}
			setTok = string; 

			break;



		case letter: 			//	S9 starts looking for an ident//

			//  While currTok is either a letter or a digit, 
			//  it is valid as an identifier
			//  The while loop should break out and print an error
			//	message if something unexpected is seen. 
			while (currTok == letter | currTok == digit)
			{
				getChar();
				charType();
			}
			switch (currTok){
				case ' ':
					setTok = ident;
				break;

				default:
					printf("Unexpexted character. Is not ident");
					break;

			}
			break;



		//	Default happens when an unexpected character is read 
		//	and the current lexeme should be ignored...
		//	Currently, there's no code to ignore the rest of the 
		//	problem lexeme, but that can wait 
		default:
			printf("Unexpected character. Don't know what to do anymore");
			break;
	}



	/*if ( currTok == letter ){  	
		while (currTok == letter | currTok == digit){
			getChar();
			charType();
		}

	}
	else if ( currTok != letter | currTok != digit){
		setTok = ident; 
		return setTok;
	}*/

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