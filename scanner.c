#include <stdio.h>		// need for file io

// this token stuff might have to be ... simpler? like: letter, colon, semicolon, etc. -- yes
typedef enum { 								// OBERON 2, not OBERON S 
				lparen, rparen, plus, minus, mul, slash, rbrac, lbrac, equal, colon, lt, lte, gt, gte, semic, null, assign, hat, notEqual,
				ident, letter, digit, resWord, number,
			 	eofSym, invalidSym, opSym,
			 	ARRAY_SYM,
			    BEGIN_SYM,
			    BY_SYM,
			    CASE_SYM,
			    CONST_SYM,
			    DIV_SYM,
			    DO_SYM,
			    ELSE_SYM,
			    ELSIF_SYM,
			    END_SYM,
			    EXIT_SYM,
			    FOR_SYM,
			    IF_SYM,
			    IMPORT_SYM,
			    IN_SYM,
			    IS_SYM,
			    LOOP_SYM,
			    MOD_SYM,
			    MODULE_SYM,
			    NIL_SYM,
			    OF_SYM,
			    OR_SYM,
			    POINTER_SYM,
			    PROCEDURE_SYM,
			    RECORD_SYM,
			    REPEAT_SYM,
			    RETURN_SYM,
			    THEN_SYM,
			    TO_SYM,
			    TYPE_SYM,
			    UNTIL_SYM,
			    VAR_SYM,
			    WHILE_SYM,
			    WITH_SYM,
			    BOOLEAN_SYM,
			    CHAR_SYM,
			    FALSE_SYM,
			    INTEGER_SYM,
			    NEW_SYM,
			    REAL_SYM,
			    TRUE_SYM
			} Token;	

const char *resWords [41][64];
const char *symNames [127][64];
Token resWordTokens [127];
Token specialSymbols [127];
const int RESWORD_SIZE = 41;

Token currTok;
const int BUFF_SIZE = 256;		// if you change this pls change currLine's size
const int WORD_SIZE = 64;
char currChar;
char currLine [256];			// 256 character limit is arbitrary but sensible.
								// should be pointer...?
int count = 0;					// Global counter for current line position
int lineNo = 0;
char currWord [64];				// Word being worked on. Delimited by found whitespaces 
								// and to be compared to a table of reserved words
Token setTok;
int gotNewLine;
int eofParsed;

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
	if (aChar == ' ' || aChar == '\t' || aChar == '\n' || aChar == '\r')// || currChar == ')' || currChar == '(')			// if its a space
		return 1;

	/*if (gotNewLine)
	{
		gotNewLine = 0;
		return 1;
	}*/

	return 0;
}

int isOp(char aChar)
{ 
	if (aChar == '+' || aChar == '-' || aChar == '*' || aChar == '/' || aChar == '=' || aChar == '>' || aChar == '<' || aChar == '#' || aChar == '&')
		return 1;

	if (aChar == 'D')
	{
		if (count+1 < BUFF_SIZE && currLine[count + 1] == 'I')
		{
			if (count+2 < BUFF_SIZE && currLine[count + 2] == 'V')
				return 1;
		}
	}

	else if (aChar == 'M')
	{
		if (count+1 < BUFF_SIZE && currLine[count + 1] == 'O')
		{
			if (count+2 < BUFF_SIZE && currLine[count + 2] == 'D')
				return 1;
		}
	}

	else if (aChar == 'O')
	{
		if (count+1 < BUFF_SIZE && currLine[count + 1] == 'R')
		{
			return 1;
		}
	}

	else if (aChar == ':')
	{
		if (count+1 < BUFF_SIZE && currLine[count + 1] == '=')
		{
			return 1;
		}
	}

	return 0;

}

int isSepG()
{
	if (currChar == ' ' || currChar == '\t' || currChar == '\n' || currChar == '\r')//|| currChar == ')' || currChar == '(')			// if its a space
		return 1;

/*	if (gotNewLine) 
	{
		gotNewLine = 0;
		return 1;
	}*/

	return 0;
}

int isIdent()
{

	int i;
	for ( i = 0; i < WORD_SIZE; i++ )
	{
		if (i == 0 && currWord[i] == '\0')
			break;	// not valid

		if (!isDigit(currWord[i]) || !isAlpha(currWord[i]))
		{
			break;	// not valid
		}
	}

	return 1;
}

int isResWord()
{
	int i;
	for ( i = 0; i < RESWORD_SIZE; i++ )
	{
		if (strcmp(currWord, *resWords[i]) == 0)
			return i;
	}

	return -1;
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
	if (feof(toScan)) 
	{
		eofParsed = 1;
		fputs("END OF FILE REACHED.", stdout);
	}
	lineNo ++;
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
		printf("NEWLINE ENCOUNTERED!\n");
		count = 0;
		getLine(); 	
		gotNewLine = 1;
	}

	currChar = currLine[count]; 
	count ++;

}

void moveUp()
{
	while (isSep(currChar))
	{
		getChar();
	}
}

//	*
//	Attempting to make a getWord method...
//
void getWord()	// assumes we're on the first letter of a word
{
	int cursor = count;
	int sCount = count;
	clrWord();

	//currWord[0] = currChar;
	//cursor ++;

	/*while(isSepG())				// while the current character is a separator ...
	{
		getChar();				// ... get the next character
	}*/

	while(isAlpha(currChar) && (cursor - sCount) < WORD_SIZE)				// while the current character is not a separator ...
	{
		currWord[cursor - sCount] = currChar;
		cursor++;
		getChar();
	}

	// debug
	currWord[WORD_SIZE - 1] = '\0';
	/*fputs(currWord, stdout);
	fputs("\n", stdout);*/

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
	else if (isOp(currChar))
		currTok = opSym;
	else
		currTok = invalidSym;
}

void dealWithComment()
{
	getChar();

	while (currChar != '*')// || currChar != '(')
	{
		getChar();

		if (currChar == '(')
		{
			// it could be a new comment
			getChar();
			if (currChar == '*')
			{
				// it is a new comment.
				dealWithComment();
			}
		}

	}

	// which situation is it; end or nested comment?
	if (currChar == '*')
	{
		getChar();
		if (currChar == ')')
		{
			// end comment
			return;
		}
	}


}

void scanNum()
{
	while ( isDigit(currChar) )
	{
		getChar();
	}

	currTok = number;
}

void scanIdent()
{
	getWord();

	int resIndex = isResWord();

	if (resIndex != -1)
	{
		currTok = resWordTokens[resIndex];
	}
	else
	{
		currTok = ident;
	}
}

void writeSym()
{

	fputs(symNames[currTok][0], stdout);
	fputs(" ", stdout);

	switch (currTok)
	{
		case ident:
			fputs(currWord, stdout);
			break;

		case number:
			fputs("number ", stdout);

			break;

		default:

			break;

	}

	fputs("\n", stdout);
}

//
//	Return the next symbol.
//
Token nextSym()
{
	moveUp();					    // in case of white space
	// getChar();					// get first character initally//
	// charType();					// S0, determing character type// ?? -- ok

	printf("Debug: %c -- ", currChar);

	if ( isAlpha(currChar) )
	{
		scanIdent();
	}
	else if (isDigit(currChar))
	{
		scanNum();
	}
	else
	{
		switch (currChar)
		{
			case '(':					// lparen

				getChar();
				if (currChar != '*')
				{
					currTok = lparen;
				}
				else
				{
					// comment
					dealWithComment();
				}
				break;

			case '<':				// lt
				currTok = lt;
				getChar();
				if ( currChar == '=' )
					currTok = lte;
				break;

			case '>':
				currTok = gt;
				getChar();
				if ( currChar == '=' )
					currTok = gte;
				break;

			case ':':
				currTok = colon;
				getChar();
				if ( currChar == '=')
					currTok = assign;
				break;

			default:
				currTok = specialSymbols[currChar];
				getChar();
				break;
		}
	}

	writeSym();

	/*switch (currChar) 
	{
		case digit: 			//	S1 starts looking for reals and ints//
			
			//	S2 keeps looking for a digit until it finds either
			// '.', whitespace or a hexdigit 
			// '.' can branch to REAL after a digit or 
			//	will lead to three more states until it reaches REAL 
			//	whitespace leads straight to an asignment to INT 
			//	hexdigit will keep looking for hexdigit until H or X are found 
			//	H will lead to INT and X should lead to STRING.  

			while (isDigit(currChar))	// eat all digits 
			{
				getChar();	
			}

			if (!isSepG())
			{
				printf("Invalid number format.");
			}
			else
			{
				currTok = number;
			}

			break;			



		case '\"': 				//	S8 starts to look for a string//      --- this isn't going to work, it's not a sym
			
			//	Keep fetching the next character until you find the
			//	next quote. Upon finding, it, set setTok to String
			while( currChar != '\"')
			{
				getChar();
			}
			setTok = string; 

			break;



		case letter: 			//	S9 starts looking for an ident
		    //  z or a digit, 
			//  it is valid as an identifier
			//  The while loop should break out and print an error
			//	message if something unexpected is seen. 
			//	A whitespace is the only ending case that should 
			//	lead to a proper ident.  


			getWord();

			if (isResWord())
			{
				currTok = resWord;
				//fputs("resWord  ", stdout);
			}
			else if (isIdent())
			{
				currTok = ident;
				//fputs("ident  ", stdout);
			}

			break;

		case opSym:


			break;

		case '(':

			getChar();

			if (currChar == '*')
			{
				//comment situation
				dealWithComment();
			}

			break;

		case eofSym:
			printf("Scanning complete.");
			break;

		//	Default happens when an unexpected character is read 
		//	and the current lexeme should be ignored...
		//	Currently, there's no code to ignore the rest of the 
		//	problem lexeme, but that can wait 
		default:
			printf("Unexpected character. Don't know what to do anymore");
			break;
	}// end switch*/

	// debug switch
/*	switch (currTok)
	{
		case number:
			fputs("number   ", stdout);
			//fputs(currWord, stdout);
			break;
		case ident:
			fputs("ident    ", stdout);
			fputs(currWord, stdout);
			break;
		case resWord:
			fputs("resWord  ", stdout);
			fputs(currWord, stdout);
			break;
		default:
		break;

	}*/
/*
	fputs("\n", stdout);*/

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

void initScanner()
{
	// Initial Reading
	getLine();
	getChar();
	eofParsed = 0;

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

	resWordTokens [0] = BOOLEAN_SYM;
	resWordTokens [1] = CHAR_SYM;
	resWordTokens [2] = FALSE_SYM;
	resWordTokens [3] = INTEGER_SYM;
	resWordTokens [4] = NEW_SYM;
	resWordTokens [5] = REAL_SYM;
	resWordTokens [6] = TRUE_SYM;
	resWordTokens [7] = ARRAY_SYM;
	resWordTokens [8] = BEGIN_SYM;
	resWordTokens [9] = BY_SYM;
	resWordTokens [10] = CASE_SYM;
	resWordTokens [11] = CONST_SYM;
	resWordTokens [12] = DIV_SYM;
	resWordTokens [13] = DO_SYM;
	resWordTokens [14] = ELSE_SYM;
	resWordTokens [15] = ELSIF_SYM;
	resWordTokens [16] = END_SYM;
	resWordTokens [17] = EXIT_SYM;
	resWordTokens [18] = FOR_SYM;
	resWordTokens [19] = IF_SYM;
	resWordTokens [20] = IMPORT_SYM;
	resWordTokens [21] = IN_SYM;
	resWordTokens [22] = IS_SYM;
	resWordTokens [23] = LOOP_SYM;
	resWordTokens [24] = MOD_SYM;
	resWordTokens [25] = MODULE_SYM;
	resWordTokens [26] = NIL_SYM;
	resWordTokens [27] = OF_SYM;
	resWordTokens [28] = OR_SYM;
	resWordTokens [29] = POINTER_SYM;
	resWordTokens [30] = PROCEDURE_SYM;
	resWordTokens [31] = RECORD_SYM;
	resWordTokens [32] = REPEAT_SYM;
	resWordTokens [33] = RETURN_SYM;
	resWordTokens [34] = THEN_SYM;
	resWordTokens [35] = TO_SYM;
	resWordTokens [36] = TYPE_SYM;
	resWordTokens [37] = UNTIL_SYM;
	resWordTokens [38] = VAR_SYM;
	resWordTokens [39] = WHILE_SYM;
	resWordTokens [40] = WITH_SYM;

}

void initSpecialSyms()
{
	int i;
	for ( i = 0; i < 127; i++ )
	{
		specialSymbols[i] = null;
	}

	// lparen, rparen, plus, minus, mul, slash, rbrac, lbrac, equal, colon, lt, gt, semic

	specialSymbols['('] = lparen;
	specialSymbols[')'] = rparen;
	specialSymbols['+'] = plus;
	specialSymbols['-'] = minus;
	specialSymbols['/'] = slash;
	specialSymbols['*'] = mul;
	specialSymbols[']'] = rbrac;
	specialSymbols['['] = lbrac;
	specialSymbols['='] = equal;
	specialSymbols[':'] = colon;
	specialSymbols['<'] = lt;
	specialSymbols['>'] = gt;
	specialSymbols[';'] = semic;
	specialSymbols['^'] = hat;
	specialSymbols['#'] = notEqual;

}

void initSymNames()
{

	int i;
	for ( i = 0; i < 127; i ++ )
	{
		symNames[i][0] = "\0";
	}

	 symNames[    ARRAY_SYM][0] = "ARRAY_SYM";
	 symNames[    BEGIN_SYM][0] = "BEGIN_SYM";
	 symNames[    BY_SYM][0] = "BY_SYM";
	 symNames[    CASE_SYM][0] = "CASE_SYM";
	 symNames[    CONST_SYM][0] = "CONST_SYM";
	 symNames[    DIV_SYM][0] = "DIV_SYM";
	 symNames[    DO_SYM][0] = "DO_SYM";
	 symNames[    ELSE_SYM][0] = "ELSE_SYM";
	 symNames[    ELSIF_SYM][0] = "ELSIF_SYM";
	 symNames[    END_SYM][0] = "END_SYM";
	 symNames[    EXIT_SYM][0] = "EXIT_SYM";
	 symNames[    FOR_SYM][0] = "FOR_SYM";
	 symNames[    IF_SYM][0] = "IF_SYM";
	 symNames[    IMPORT_SYM][0] = "IMPORT_SYM";
	 symNames[    IN_SYM][0] = "IN_SYM";
	 symNames[    IS_SYM][0] = "IS_SYM";
	 symNames[    LOOP_SYM][0] = "LOOP_SYM";
	 symNames[    MOD_SYM][0] = "MOD_SYM";
	 symNames[    MODULE_SYM][0] = "MODULE_SYM";
	 symNames[    NIL_SYM][0] = "NIL_SYM";
	 symNames[    OF_SYM][0] = "OF_SYM";
	 symNames[    OR_SYM][0] = "OR_SYM";
	 symNames[    POINTER_SYM][0] = "POINTER_SYM";
	 symNames[    PROCEDURE_SYM][0] = "PROCEDURE_SYM";
	 symNames[    RECORD_SYM][0] = "RECORD_SYM";
	 symNames[    REPEAT_SYM][0] = "REPEAT_SYM";
	 symNames[    RETURN_SYM][0] = "RETURN_SYM";
	 symNames[    THEN_SYM][0] = "THEN_SYM";
	 symNames[    TO_SYM][0] = "TO_SYM";
	 symNames[    TYPE_SYM][0] = "TYPE_SYM";
	 symNames[    UNTIL_SYM][0] = "UNTIL_SYM";
	 symNames[    VAR_SYM][0] = "VAR_SYM";
	 symNames[    WHILE_SYM][0] = "WHILE_SYM";
	 symNames[    WITH_SYM][0] = "WITH_SYM";
	 symNames[    BOOLEAN_SYM][0] = "BOOLEAN_SYM";
	 symNames[    CHAR_SYM][0] = "CHAR_SYM";
	 symNames[    FALSE_SYM][0] = "FALSE_SYM";
	 symNames[    INTEGER_SYM][0] = "INTEGER_SYM";
	 symNames[    NEW_SYM][0] = "NEW_SYM";
	 symNames[    REAL_SYM][0] = "REAL_SYM";
	 symNames[    TRUE_SYM][0] = "TRUE_SYM";
	 symNames[    ident][0] = "IDENT";
	 symNames[    number][0] = "NUMBER";
	 symNames[    lparen][0] = "LPAREN";
	 symNames[    rparen][0] = "RPAREN";
	 symNames[    plus][0] = "PLUS";
	 symNames[    minus][0] = "MINUS";
	 symNames[    mul][0] = "MUL";
	 symNames[    slash][0] = "SLASH";
	 symNames[    rbrac][0] = "RBRAC";
	 symNames[    lbrac][0] = "LBRAC";
	 symNames[    equal][0] = "EQUAL";
	 symNames[    colon][0] = "COLON";
	 symNames[    lt][0] = "LT";
	 symNames[    gt][0] = "GT";
	 symNames[    gte][0] = "GTE";
	 symNames[    semic][0] = "SEMIC";
	 symNames[    null][0] = "NULL";
	 symNames[    assign][0] = "ASSIGN";
	 symNames[    hat][0] = "HAT";
	 symNames[    notEqual][0] = "NOT_EQUAL";
}

//
//	Scans the file and outputs the tokens to the screen.
//
void scan()
{
	initScanner();
	initSymNames();
	initSpecialSyms();

	// while (currTok != eofSym)
	// 	nextSym();

	while (eofParsed == 0)
	{
		//printf("hahahahaha, no\n");
		nextSym();
	}

/*	getWord();
	if(isResWord())
	{
		fputs(currWord, stdout);
		printf("\nok..");
	}*/

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


//
//	Surgically removed code...
//

			/*while (currTok == letter | currTok == digit)				// from Letter in nextSym
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

			}*/