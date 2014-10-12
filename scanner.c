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

const char *resWords [41][64];
const int RESWORD_SIZE = 41;

Token currTok;
const int BUFF_SIZE = 256;		// if you change this pls change currLine's size
const int WORD_SIZE = 64;
char currChar;
char currLine [256];			// 256 character limit is arbitrary but sensible.
								// should be pointer...?
int count = 0;					// Global counter for current line position
char currWord [64];				// Word being worked on. Delimited by found whitespaces 
								// and to be compared to a table of reserved words
Token setTok;
int gotNewLine;

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
	if (aChar == ' ' || aChar == '\t' || aChar == '\n')			// if its a space
		return 1;

	if (gotNewLine)
	{
		gotNewLine = 0;
		return 1;
	}

	return 0;
}

int isSepG()
{
	if (currChar == ' ' || currChar == '\t' || currChar == '\n')			// if its a space
		return 1;

	if (gotNewLine)
	{
		gotNewLine = 0;
		return 1;
	}

	return 0;
}

int isResWord()
{
	int i;
	for ( i = 0; i < RESWORD_SIZE; i++ )
	{
		if (strcmp(currWord, resWords[i]))
			return 1;
	}

	return 0;
}

void clrWord()
{
	int i;
	for ( i = 0; i < WORD_SIZE; i++ )
	{
		currWord[i] = '\0';
	}
}

void clrLine()
{
	int i;
	for ( i = 0; i < BUFF_SIZE; i++ )
	{
		currLine[i] = '\0';
	}
}

//	*
//	Get a line and put it in currLine.
//
void getLine()
{
	clrLine();
	fgets(currLine, BUFF_SIZE, toScan);
	//gotNewLine = 1;
	//fputs(currLine, stdout); 					// Debug MSG

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

	//int gotNewLine = 0;

	//ok i'm assuming currline's size is Buff_size characters total
	//starting at index 0, and will be over limit at or above buffSize
	//this is sorta shit implementation, but i don't have a clear idea what will be 
	//calling this yet, so for now, it will collect all the characters as it gets 
	//called and when it reaches Buff_size, it'll restart, so better hope we have a new line 
	//by then.
	if(count >= BUFF_SIZE || currLine[count] == '\n')
	{
		count = 0;
		getLine(); 	
		gotNewLine = 1;
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
			//	A whitespace is the only ending case that should 
			//	lead to a proper ident.  
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

//	*
//	Attempting to make a getWord method...
//
void getWord()
{
	int cursor = count;
	int sCount = count;
	clrWord();

	while(isSepG())				// while the current character is a separator ...
	{
		getChar();				// ... get the next character
	}

	while(!isSepG() && (cursor - sCount) < WORD_SIZE)				// while the current character is not a separator ...
	{
		currWord[cursor - sCount] = currChar;
		cursor++;
		getChar();
	}

	// debug
	currWord[WORD_SIZE - 1] = '\0';
	fputs(currWord, stdout);
	fputs("\n", stdout);

}

void initScanner()
{
	// Initial Reading
	getLine();
	getChar();

	// Initialize Reserved Words
	resWords [0][0] = "BOOLEAN";
	resWords [1][0] = "CHAR";
	resWords [2][0] = "FALSE";
	resWords [3][0] = "INTEGER";
	resWords [4][0] = "NEW";
	resWords [5][0] = "REAL";
	resWords [6][0] = "TRUE";
	resWords [7][0] = "ARRAY";
	resWords [8][0] = "BEGIN";
	resWords [9][0] = "BY";
	resWords [10][0] = "CASE";
	resWords [11][0] = "CONST";
	resWords [12][0] = "DIV";
	resWords [13][0] = "DO";
	resWords [14][0] = "ELSE";
	resWords [15][0] = "ELSIF";
	resWords [16][0] = "END";
	resWords [17][0] = "EXIT";
	resWords [18][0] = "FOR";
	resWords [19][0] = "IF";
	resWords [20][0] = "IMPORT";
	resWords [21][0] = "IN";
	resWords [22][0] = "IS";
	resWords [23][0] = "LOOP";
	resWords [24][0] = "MOD";
	resWords [25][0] = "MODULE";
	resWords [26][0] = "NIL";
	resWords [27][0] = "OF";
	resWords [28][0] = "OR";
	resWords [29][0] = "POINTER";
	resWords [30][0] = "PRODECURE";
	resWords [31][0] = "RECORD";
	resWords [32][0] = "REPEAT";
	resWords [33][0] = "RETURN";
	resWords [34][0] = "THEN";
	resWords [35][0] = "TO";
	resWords [36][0] = "TYPE";
	resWords [37][0] = "UNTIL";
	resWords [38][0] = "VAR";
	resWords [39][0] = "WHILE";
	resWords [40][0] = "WITH";

}

//
//	Scans the file and outputs the tokens to the screen.
//
void scan()
{
	initScanner();

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