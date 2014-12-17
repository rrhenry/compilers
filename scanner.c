//  Oberon Sparser
//  Alexi Turcotte
//  Roxanne Henry

/*			TODO LIST
		-> phase out need for <string.h>
		-> make fatal errors fatal (i.e. ctrl-find F_ERROR and add exit statement)
*/

#include <stdio.h>		// need for file io
#include <string.h>
#include "scanner.h"

typedef enum { 								// OBERON 2, not OBERON S 
				lparen, rparen, plus, minus, mul, slash, rbrac, lbrac, equal, colon, lt, lte, gt, gte, SEMIC, null, assign, hat, notEqual, comma, period,
				ident, resWord, number, string, 
			 	eofSym, invalidSym, opSym, SET_SYM, tilde, lcurly, rcurly,
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
				AND_SYM,
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
			    //INTEGER_SYM,
			    NEW_SYM,
			    REAL_SYM,
			    TRUE_SYM
			} Token;	

const char *resWords [41][64];
const char *symNames [127][64];
Token resWordTokens [127];
Token specialSymbols [127];
const int RESWORD_SIZE = 40;

Token currTok;
const int BUFF_SIZE = 256;		// if you change this pls change currLine's size
const int WORD_SIZE = 64;
char currChar;
char currLine [256];			// 256 character limit is arbitrary but sensible.
								// should be pointer...?
int lineLen = 0;
int inptr = 0;
int count = 0;					// Global counter for current line position
int lineNo = 0;
char currWord [64];				// Word being worked on. Delimited by found whitespaces 
								// and to be compared to a table of reserved words
char currNum [64];      		// Simply saving the num so as to pass it on over later,
								// if needed.
int gotNewLine = 0;
Token setTok;
//int eofParsed = 0;

FILE *toScan;

/* BEGIN: String processing methods */

char *strcopy(char *dst, const char *src)
{
   char *saved = dst;
   while (*src)
   {
       *dst++ = *src++;
   }
   *dst = 0;
   return saved;
}

/* END:   String processing methods */

/* Code Generator Data Fields */

int currlev = 0;
int lc = 0;
int stptr = 0;				// ptr to last entry in symbol table
int ttptr = 0;				// ptr to last entry in type table
const int maxlev = 10;
const int stsize = 256;		// length of symbol table
const int ttsize = 128;  	// length of type table
const int codemax = 1023;   // length of code array
const int idbuffsize = 16;	// length of identifiers

const char *mnemonic [16][5];

/* Predefined types */

int inttyp, realtyp, booltyp, chartyp, texttyp;

/*                  */

typedef enum 				// classes of identifiers
{
	funccls
	,paramcls
	,typcls
	,varcls
	,constcls
	,proccls
	,stdpcls
} IdClass;

typedef enum				// types of types
{
	arrayfrm
	,recrodfrm
	,ptrfrm
	,procfrm
	,scalarfrm
} TypeForm;

typedef enum				// all possible opcodes
{
	opr
	,push
	,pshc
	,psha
	,pshi
	,pop
	,popi
	,isp
	,jmp
	,jmpc
	,jsr
	,jmpx
	,for0
	,for1
	,selc
	,nop
} Opcode;

struct vminstr				// struct for VM instructions
{
	Opcode op;				// opcode
	int ld;					// static level difference
	int ad;					// offset from beginning of stack frame
};

struct typerec				// type struct
{
	int size;				// memory footprint
	TypeForm form;			// type type
};

struct identrec 			// ident struct
{
	char name [16];			// name of the ident
	int previd;				// link to previous identrec
	int idlev;				// static level of the ident decl
	int idtyp;				// type of the ident
	IdClass class;			// class of the ident (i.e. ref to class table)
	// TODO: need to store class information
};

struct identrec symtab [256];  		// symbol table
struct typerec typetab [128];		// type table
struct vminstr code    [1023];		// generated code
int scopetab [10];					// scope table

/* End Code Gen Data Fields */

/* BEGIN: Code gen methods */

void enterstdtypes();
void printsymtab();
void printtyptab();

void enterScope()
{
	if ( currlev < maxlev )
	{	
		currlev ++;
		scopetab[ currlev] = 0;
	}
	else
	{
		fputs("F_ERROR: Maximum nesting level exceeded.", stdout);
	}
}

void exitScope()
{
	currlev --;
}

void genCode( Opcode o, int l, int a)
{
	if ( lc > codemax)
	{
		fputs("F_ERROR: Code too large.", stdout);
	}
	else
	{
		code[ lc].op = o;
		code[ lc].ld = l;
		code[ lc].ad = a;
		lc ++;
	}
}

void enterstdident ( char id [], IdClass cls, int ttp)
{
	stptr ++;
	strcopy(symtab[ stptr].name, id);
	symtab[ stptr].previd = scopetab[ currlev];
	symtab[ stptr].idlev = currlev;
	symtab[ stptr].idtyp = ttp;
	symtab[ stptr].class = cls;
	scopetab[ currlev] = stptr;
}

int searchid( char id [16])
{
	int stp = stptr;				// local symbol table ptr
	int lev = 0;					// local var for level
	strcopy(symtab[ 0].name, id);	// sentinel for search

	lev = currlev;					// start searching at the current scope level
	do
	{
		stp = scopetab[ lev];
		printf("%d: stp\n", stp);
		while ( strcmp(symtab[ stp].name, id) != 0)
		{
			stp = symtab[ stp].previd;
		}
		lev --;
	} while (stp == 0 && lev >= 0);

	if (stp == 0)
		printf("ERROR: 42 Undeclared identifier: %s\n", id);

	return stp;
}

void insertid( char id [16], IdClass cls)
{
	int stp = scopetab[ currlev];
	strcopy(symtab[ 0].name, id);	// sentinel for search

	while ( strcmp(symtab[ stp].name, id) != 0)
	{
		stp = symtab[ stp].previd;
	}
	if ( stp != 0)
	{
		fputs("ERROR: 44 Multiple declarations.\n", stdout);
	}
	if ( stptr < stsize)
	{
		stptr ++;
	}
	else
	{
		fputs("F_ERROR: Symbol table overflow.\n", stdout);
	}

	strcopy(symtab[ stptr].name, id);
	symtab[ stptr].class = cls;
	symtab[ stptr].idlev = currlev;
	symtab[ stptr].previd = scopetab[ currlev];

	scopetab[ currlev] = stptr;

}

void checktypes(int ttp1, int ttp2)
{
	if (ttp1 != ttp2)
		printf("Type mismatch! Oh no!\n");
}

//* BEGIN: Subset of Code Gen methods -- compiler initialization *//

void initstdmnemonics()
{
    mnemonic[ opr ][ 0] = "opr ";
    mnemonic[ push][ 0] = "push";
    mnemonic[ pshc][ 0] = "pshc";
    mnemonic[ psha][ 0] = "psha";
    mnemonic[ pshi][ 0] = "pshi";
    mnemonic[ pop ][ 0] = "pop ";
    mnemonic[ popi][ 0] = "popi";
    mnemonic[ jsr ][ 0] = "jsr ";
    mnemonic[ isp ][ 0] = "isp ";
    mnemonic[ jmp ][ 0] = "jmp ";
    mnemonic[ jmpc][ 0] = "jmpc";
    mnemonic[ jmpx][ 0] = "jmpx";
    mnemonic[ for0][ 0] = "for0";
    mnemonic[ for1][ 0] = "for1";
    mnemonic[ selc][ 0] = "selc";
    mnemonic[ nop ][ 0] = "nop ";
}

void initstdidents()
{
	int nstdidents = 24;
	char *stdidents [24][16];
	stdidents[  0][ 0] = "ABS            ";
	stdidents[  1][ 0] = "CHAR           ";
	stdidents[  2][ 0] = "FLT            ";
	stdidents[  3][ 0] = "LSL            ";
	stdidents[  4][ 0] = "REAL           ";
	stdidents[  5][ 0] = "ASR            ";
	stdidents[  6][ 0] = "CHR            ";
	stdidents[  7][ 0] = "INC            ";
	stdidents[  8][ 0] = "NEW            ";
	stdidents[  9][ 0] = "ROR            ";
	stdidents[ 10][ 0] = "ASSERT         ";
	stdidents[ 11][ 0] = "DEC            ";
	stdidents[ 12][ 0] = "INCL           ";
	stdidents[ 13][ 0] = "ODD            ";
	stdidents[ 14][ 0] = "SET            ";
	stdidents[ 15][ 0] = "BOOLEAN        ";
	stdidents[ 16][ 0] = "EXCL           ";
	stdidents[ 17][ 0] = "INTEGER        ";
	stdidents[ 18][ 0] = "ORD            ";
	stdidents[ 19][ 0] = "UNPK           ";
	stdidents[ 20][ 0] = "BYTE           ";
	stdidents[ 21][ 0] = "FLOOR          ";
	stdidents[ 22][ 0] = "LEN            ";
	stdidents[ 23][ 0] = "PACK           ";

	// enter std types into symbol table
	enterstdident( stdidents[ 17][0], typcls, inttyp);		// integer
	enterstdident( stdidents[  4][0], typcls, realtyp);		// real
	enterstdident( stdidents[  1][0], typcls, chartyp);		// character
	enterstdident( stdidents[ 15][0], typcls, booltyp);		// boolean

	// enter std function calls
	enterstdident( stdidents[  0][0], stdpcls, 0);			// ABS
	enterstdident( stdidents[ 13][0], stdpcls, 0);			// ODD

}


// initialize standard types
void enterstdtypes()
{
	// integer type
	ttptr ++;
	inttyp = ttptr;
	typetab[ ttptr].size = 1;
	typetab[ ttptr].form = scalarfrm;

	// real type
	ttptr ++;
	realtyp = ttptr;
	typetab[ ttptr].size = 1;
	typetab[ ttptr].form = scalarfrm;

	// boolean type
	ttptr ++;
	booltyp = ttptr;
	typetab[ ttptr].size = 1;
	typetab[ ttptr].form = scalarfrm;

	// character type
	ttptr ++;
	chartyp = ttptr;
	typetab[ ttptr].size = 1;
	typetab[ ttptr].form = scalarfrm;
}

void printsymtab()
{
	printf("\n");
	printf("      name             level   type  previd\n");		// TOADD addr  

	int i;
	for ( i = 1; i <= stptr; i++)
	{
		printf("%4d:%16s %6d %6d %7d\n", i, symtab[ i].name, symtab[ i].idlev, symtab[ i].idtyp, symtab[ i].previd);
	}
}

void printtyptab()
{
	printf("\n");
	printf("       size  lastfld \n");

	int i;
	for ( i = 1; i < ttptr; i++)
	{
		printf("%4d:%6d %8d\n", i, typetab[ i].size, typetab[ i].form);
	}
}

void initcompile()
{
	enterstdtypes();
	initstdidents();
	initstdmnemonics();
	printsymtab();
	printtyptab();
}

//* END:   Subset of Code Gen methods -- compiler initialization *//

/* END:   Code gen methods */

void writeSym();

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
	if (aChar == ' ' || aChar == '\t' || aChar == '\n' || aChar == '\r')// || aChar == '\0')// || currChar == ')' || currChar == '(')			// if its a space
		return 1;

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
		if (currLine[i] == '\0') break;
		currLine[i] = '\0';
	}
}

//	*
//	Get a line and put it in currLine.
//
void getLineLegacy()
{
	clrLine();
	fgets(currLine, BUFF_SIZE, toScan);
	if (feof(toScan)) 
	{
		//eofParsed = 1;
		fputs("END OF FILE REACHED.", stdout);
	}
	lineNo ++;

}

void getLine()
{
	clrLine();
	lineNo ++;

	inptr = 0;
	char theChar = getc(toScan);

	currLine[inptr] = theChar;

	while ( theChar != '\n' && inptr <= BUFF_SIZE && theChar != EOF && theChar != '\0' )
	{
		// putc(theChar, stdout);
		theChar = getc(toScan);
		inptr ++;
		currLine[inptr] = theChar;
		// putc(currWord[inptr], stdout);
	}

	lineLen = inptr;
	inptr = 0;

	currChar = theChar;

	gotNewLine = 1;

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
	if(inptr >= lineLen)
	{
		getLine();
	}
	else
	{
		currChar = currLine[inptr]; 
		inptr ++;
	}
	 
	//putc(currChar, stdout);

}

void moveUp()
{
	while (isSep(currChar) && currChar != EOF)
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

	while(isAlpha(currChar) && (cursor - sCount) < WORD_SIZE)				// while the current character is not a separator ...
	{
		currWord[cursor - sCount] = currChar;
		cursor++;
		getChar();
	}

}

void dealWithComment()
{
	int nestLvl = 1;

	do
	{
		getChar();

		if (currChar == EOF)
		{
			printf("Oops, end of file encoutered in comment.");
			nestLvl = 0;
			break;
		}
		else if (currChar == '(')
		{
			if ( inptr < BUFF_SIZE && currLine[inptr] == '*')
			{
				nestLvl++;
				getChar();
			}
		}
		else if (currChar == '*')
		{
			if ( inptr < BUFF_SIZE && currLine[inptr] == ')')
			{
				nestLvl--;
				getChar();
			}
		}

	} while (nestLvl > 0);
	getChar();
}

void scanNum()
{
	while( isDigit(currChar) )
	{
		getChar();
	}

	if (currChar == '.' && currLine[inptr] != '.') // need addtional lookahead for range op
	{
		
		getChar();
		while( isDigit(currChar) )
		{
			getChar();
		}


		if( isSep(currChar) || currChar == ';' || currChar == EOF) // Real
		{
			//fputs("Setting for Decimal\n", stdout);
			currTok = REAL_SYM;
		}
		else if( currChar == 'E' || currChar == 'D' ) // Scalefac
		{	
			getChar();
			if( currChar == '+' || currChar == '-' )
			{
				getChar();

				while ( isDigit(currChar) )	
					getChar();		

				if( isSep(currChar) || currChar == ';' || currChar == EOF)
				{
					//fputs("Setting for ScaleFac\n", stdout);
					currTok = number;
				}
			}
		}
	}
	else if ( isSep(currChar) || currChar == ';')
	{
		//fputs("Setting for integer\n", stdout);
		currTok = number;
	}
	else if ( isHexDigit(currChar) )
	{
		while( isHexDigit(currChar) )
		{
			getChar();
		} 
		if( currChar == 'H' )
		{	
			getChar();
			if( isSep(currChar) || currChar == ';' )
			{
				//fputs("Setting for hexDigit\n", stdout);
				currTok = number;
			}
				
		}

		//--Hex String
		else if ( currChar == 'X' )
		{
			getChar();
			if( isSep(currChar) || currChar == ';' )
			{
				//fputs("Setting for hexString\n", stdout);
				currTok = string;
			}
				
		}
	}
	else if ( currChar == 'H' )
	{
		getChar();
		if( isSep(currChar) || currChar == ';' )
		{
			currTok = number;
		}
	}	
	else 
	{
		currTok = number;
	}
}


void scanIdent()
{
	getWord();

	int resIndex = isResWord();

	if (resIndex != -1)
	{
		//fputs("Scanning ident\n", stdout);
		currTok = resWordTokens[resIndex];
	}
	else
	{
		currTok = ident;
	}
}

void scanString()
{
	getChar();
	int endString = 0;
	while ( endString <=0 )
	{
		if ( currChar == '\\' )
		{
			getChar();
			if ( currChar == '\"')
			{
				getChar();	// if it's a backslash quote walk over it

			}
		}

		if ( currChar == '\"')
		{
			currTok = string;
			break;
		}
		getChar();
	}
	getChar();	// to move past the /" we parsed upon exit
}

void writeSym()
{
	printf("%d", lineNo);
	fputs(": ", stdout);
	fputs("[", stdout);
	fputs(symNames[currTok][0], stdout);
	

	switch (currTok)
	{
		case ident:
			fputs(": ", stdout);
			fputs(currWord, stdout);
			break;

		default:

			break;

	}

	fputs("] \n", stdout);

	if (gotNewLine == 1 && lineNo != 1)
	{
		printf("\n");
		gotNewLine = 0;
	}
}

//
//	Return the next symbol.
//
void nextSym()
{
	moveUp();					    // in case of white space
	int wasComment = 0;

	if (currChar == EOF)
	{
		currTok = eofSym;
	}
	else if ( isAlpha(currChar) )
	{
		scanIdent();
		//fputs("Scanned Ident\n",stdout);
	}
	else if (isDigit(currChar))
	{
		fputs("Scanned numba\n",stdout);
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
					wasComment = 1;
					dealWithComment();
				}
				break;

			case '<':				// lt
				currTok = lt;
				getChar();
				if ( currChar == '=' )
				{
					currTok = lte;
					getChar(); // we need to move up to the next character
				}
				break;

			case '>':
				currTok = gt;
				getChar();
				if ( currChar == '=' )
				{
					currTok = gte;
					getChar();
				}
				break;

			case ':':
				currTok = colon;
				getChar();
				if ( currChar == '=')
				{
					currTok = assign;
					getChar();
				}
				break;

			case '"':
				scanString();
				break;

			default:
				currTok = specialSymbols[currChar];
				getChar();
				break;
		}
	}
	if (wasComment == 0)
	{
		writeSym();
	}
}

void initScanner()
{
	// Initialize Reserved Words
	resWords [  0][ 0] = "BOOLEAN";
	resWords [  1][ 0] = "CHAR";
	resWords [  2][ 0] = "FALSE";
	resWords [  3][ 0] = "NEW";
	resWords [  4][ 0] = "REAL";
	resWords [  5][ 0] = "TRUE";
	resWords [  6][ 0] = "ARRAY";
	resWords [  7][ 0] = "BEGIN";
	resWords [  8][ 0] = "BY";
	resWords [  9][ 0] = "CASE";
	resWords [ 10][ 0] = "CONST";
	resWords [ 11][ 0] = "DIV";
	resWords [ 12][ 0] = "DO";
	resWords [ 13][ 0] = "ELSE";
	resWords [ 14][ 0] = "ELSIF";
	resWords [ 15][ 0] = "END";
	resWords [ 16][ 0] = "EXIT";
	resWords [ 17][ 0] = "FOR";
	resWords [ 18][ 0] = "IF";
	resWords [ 19][ 0] = "IMPORT";
	resWords [ 20][ 0] = "IN";
	resWords [ 21][ 0] = "IS";
	resWords [ 22][ 0] = "LOOP";
	resWords [ 23][ 0] = "MOD";
	resWords [ 24][ 0] = "MODULE";
	resWords [ 25][ 0] = "NIL";
	resWords [ 26][ 0] = "OF";
	resWords [ 27][ 0] = "OR";
	resWords [ 28][ 0] = "POINTER";
	resWords [ 29][ 0] = "PROCEDURE";
	resWords [ 30][ 0] = "RECORD";
	resWords [ 31][ 0] = "REPEAT";
	resWords [ 32][ 0] = "RETURN";
	resWords [ 33][ 0] = "THEN";
	resWords [ 34][ 0] = "TO";
	resWords [ 35][ 0] = "TYPE";
	resWords [ 36][ 0] = "UNTIL";
	resWords [ 37][ 0] = "VAR";
	resWords [ 38][ 0] = "WHILE";
	resWords [ 39][ 0] = "WITH";
//	resWords [ 40][ 0] = "INTEGER";

	resWordTokens [  0] = BOOLEAN_SYM;
	resWordTokens [  1] = CHAR_SYM;
	resWordTokens [  2] = FALSE_SYM;
	resWordTokens [  3] = NEW_SYM;
	resWordTokens [  4] = REAL_SYM;
	resWordTokens [  5] = TRUE_SYM;
	resWordTokens [  6] = ARRAY_SYM;
	resWordTokens [  7] = BEGIN_SYM;
	resWordTokens [  8] = BY_SYM;
	resWordTokens [  9] = CASE_SYM;
	resWordTokens [ 10] = CONST_SYM;
	resWordTokens [ 11] = DIV_SYM;
	resWordTokens [ 12] = DO_SYM;
	resWordTokens [ 13] = ELSE_SYM;
	resWordTokens [ 14] = ELSIF_SYM;
	resWordTokens [ 15] = END_SYM;
	resWordTokens [ 16] = EXIT_SYM;
	resWordTokens [ 17] = FOR_SYM;
	resWordTokens [ 18] = IF_SYM;
	resWordTokens [ 19] = IMPORT_SYM;
	resWordTokens [ 20] = IN_SYM;
	resWordTokens [ 21] = IS_SYM;
	resWordTokens [ 22] = LOOP_SYM;
	resWordTokens [ 23] = MOD_SYM;
	resWordTokens [ 24] = MODULE_SYM;
	resWordTokens [ 25] = NIL_SYM;
	resWordTokens [ 26] = OF_SYM;
	resWordTokens [ 27] = OR_SYM;
	resWordTokens [ 28] = POINTER_SYM;
	resWordTokens [ 29] = PROCEDURE_SYM;
	resWordTokens [ 30] = RECORD_SYM;
	resWordTokens [ 31] = REPEAT_SYM;
	resWordTokens [ 32] = RETURN_SYM;
	resWordTokens [ 33] = THEN_SYM;
	resWordTokens [ 34] = TO_SYM;
	resWordTokens [ 35] = TYPE_SYM;
	resWordTokens [ 36] = UNTIL_SYM;
	resWordTokens [ 37] = VAR_SYM;
	resWordTokens [ 38] = WHILE_SYM;
	resWordTokens [ 39] = WITH_SYM;
//	resWordTokens [ 40] = INTEGER_SYM;

}

void initSpecialSyms()
{
	int i;
	for ( i = 0; i < 127; i++ )
	{
		specialSymbols[i] = null;
	}

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
	specialSymbols[';'] = SEMIC;
	specialSymbols['^'] = hat;
	specialSymbols['#'] = notEqual;
	specialSymbols[','] = comma;
	specialSymbols['.'] = period;

}

void initSymNames()
{

	int i;
	for ( i = 0; i < 127; i ++ )
	{
		symNames[i][0] = "\0";
	}

	 symNames[     ARRAY_SYM][ 0] = "ARRAY_SYM";
	 symNames[     BEGIN_SYM][ 0] = "BEGIN_SYM";
	 symNames[        BY_SYM][ 0] = "BY_SYM";
	 symNames[      CASE_SYM][ 0] = "CASE_SYM";
	 symNames[     CONST_SYM][ 0] = "CONST_SYM";
	 symNames[       DIV_SYM][ 0] = "DIV_SYM";
	 symNames[        DO_SYM][ 0] = "DO_SYM";
	 symNames[      ELSE_SYM][ 0] = "ELSE_SYM";
	 symNames[     ELSIF_SYM][ 0] = "ELSIF_SYM";
	 symNames[       END_SYM][ 0] = "END_SYM";
	 symNames[      EXIT_SYM][ 0] = "EXIT_SYM";
	 symNames[       FOR_SYM][ 0] = "FOR_SYM";
	 symNames[        IF_SYM][ 0] = "IF_SYM";
	 symNames[    IMPORT_SYM][ 0] = "IMPORT_SYM";
	 symNames[        IN_SYM][ 0] = "IN_SYM";
	 symNames[        IS_SYM][ 0] = "IS_SYM";
	 symNames[      LOOP_SYM][ 0] = "LOOP_SYM";
	 symNames[       MOD_SYM][ 0] = "MOD_SYM";
	 symNames[    MODULE_SYM][ 0] = "MODULE_SYM";
	 symNames[       NIL_SYM][ 0] = "NIL_SYM";
	 symNames[        OF_SYM][ 0] = "OF_SYM";
	 symNames[        OR_SYM][ 0] = "OR_SYM";
	 symNames[       AND_SYM][ 0] = "AND_SYM";
	 symNames[   POINTER_SYM][ 0] = "POINTER_SYM";
	 symNames[ PROCEDURE_SYM][ 0] = "PROCEDURE_SYM";
	 symNames[    RECORD_SYM][ 0] = "RECORD_SYM";
	 symNames[    REPEAT_SYM][ 0] = "REPEAT_SYM";
	 symNames[    RETURN_SYM][ 0] = "RETURN_SYM";
	 symNames[      THEN_SYM][ 0] = "THEN_SYM";
	 symNames[        TO_SYM][ 0] = "TO_SYM";
	 symNames[      TYPE_SYM][ 0] = "TYPE_SYM";
	 symNames[     UNTIL_SYM][ 0] = "UNTIL_SYM";
	 symNames[       VAR_SYM][ 0] = "VAR_SYM";
	 symNames[     WHILE_SYM][ 0] = "WHILE_SYM";
	 symNames[      WITH_SYM][ 0] = "WITH_SYM";
	 symNames[   BOOLEAN_SYM][ 0] = "BOOLEAN_SYM";
	 symNames[      CHAR_SYM][ 0] = "CHAR_SYM";
	 symNames[     FALSE_SYM][ 0] = "FALSE_SYM";
	// symNames[    INTEGER_SYM][0] = "INTEGER_SYM";
	 symNames[       NEW_SYM][ 0] = "NEW_SYM";
	 symNames[      REAL_SYM][ 0] = "REAL_SYM";
	 symNames[      TRUE_SYM][ 0] = "TRUE_SYM";
	 symNames[	     SET_SYM][ 0] = "SET_SYM";
	 symNames[         ident][ 0] = "IDENT";
	 symNames[        number][ 0] = "NUMBER";
	 symNames[        lparen][ 0] = "LPAREN";
	 symNames[        rparen][ 0] = "RPAREN";
	 symNames[        lcurly][ 0] = "LCURLY";
	 symNames[        rcurly][ 0] = "RCURL";
	 symNames[          plus][ 0] = "PLUS";
	 symNames[         minus][ 0] = "MINUS";
	 symNames[           mul][ 0] = "MUL";
	 symNames[         slash][ 0] = "SLASH";
	 symNames[         rbrac][ 0] = "RBRAC";
	 symNames[         lbrac][ 0] = "LBRAC";
	 symNames[         equal][ 0] = "EQUAL";
	 symNames[         colon][ 0] = "COLON";
	 symNames[            lt][ 0] = "LT";
	 symNames[           lte][ 0] = "LTE";
	 symNames[            gt][ 0] = "GT";
	 symNames[           gte][ 0] = "GTE";
	 symNames[         SEMIC][ 0] = "SEMIC";
	 symNames[          null][ 0] = "NULL";
	 symNames[        assign][ 0] = "ASSIGN";
	 symNames[           hat][ 0] = "HAT";
	 symNames[      notEqual][ 0] = "NOT_EQUAL";
	 symNames[         comma][ 0] = "COMMA";
	 symNames[        period][ 0] = "PERIOD";
	 symNames[        string][ 0] = "STRING";
	 symNames[         tilde][ 0] = "TILDE";
	 symNames[        eofSym][ 0] = "EOF";
}

//
//	Scans the file and outputs the tokens to the screen.
//
void scan()
{
	initScanner();
	initSymNames();
	initSpecialSyms();
	initcompile();

	fputs("\nScanning ... Begin.\n\n", stdout);
	getChar();
	//while (currTok != eofSym)
	//{
		//nextSym();
		Module();
		//fputs("Parse???",stdout);
		//}

	fputs("\nScanning complete.\n\n", stdout);

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

/*
End of scanner.
=====================================================================================================
Beginning of Parser.																				
*/

void expect (Token t)
{
	if (currTok != t)
	{
		fputs("\nln: ", stdout);
		printf("%d", lineNo);
		fputs("  ERROR: Unexpexted token ", stdout);
		fputs(symNames[currTok][0], stdout);
		fputs(". ", stdout);
		fputs(symNames[t][0], stdout);
		fputs(" expected\n\n", stdout);
		
		fputs(currLine, stdout);
		
		int i;
		for( i = 0; i < inptr-2; i++)
			fputc('-', stdout);
		fputs("^\n\n", stdout);
		
		
	}

	nextSym();
}

void qualident()
{
	fputs("This is a qualident\n", stdout);
	expect(ident);
	if(currTok == period)
	{
		nextSym();
		expect(ident);
	}
	
	fputs("Done qualident.\n", stdout);
}

void type ( int ttp)
{
	fputs("This is type\n", stdout); 
	
	if ( currTok == ident )
		qualident();
	else if ( currTok == RECORD_SYM | currTok == ARRAY_SYM | currTok == POINTER_SYM | currTok == PROCEDURE_SYM )  
		StrucType( ttp);
	fputs("Done type\n", stdout);
}

void SimplExpr ( int ttp)
{
	if ( currTok == plus | currTok == minus )
	{
		Token addop = currTok;
		nextSym();
		checktypes( ttp, inttyp);
		if (addop == minus)
		{
			genCode( opr, 0, 2);
		}
	}
	term( ttp);
	while ( currTok == plus | currTok == minus | currTok == OR_SYM)
	{
		nextSym();
		term( ttp);
	}

}

void expr ( int ttp)
{
	fputs("This is expr\n", stdout);
	SimplExpr( ttp);
	fputs("Done expr\n", stdout);
	if( currTok == equal | currTok == notEqual | currTok == lt | currTok == lte | currTok == gt | currTok == gte | currTok == IN_SYM | currTok == IS_SYM )
	{
		nextSym();
		SimplExpr( ttp);
	}
}

void ActParams ()
{
	fputs("This is ActParams\n", stdout);
	int ttp;
	expr( ttp);
	fputs("HERE?\n", stdout);
	while ( currTok == comma )
	{
		nextSym();
		expr( ttp);
	}
	expect(rparen);
	fputs("Done ActParams\n", stdout);
}

void designator ()
{
	fputs("This is designator\n", stdout);
	qualident();
	while ( currTok == period | currTok == lbrac | currTok == hat ) 
	{
		if ( currTok == period)
		{
			nextSym();
			expect(ident);
		}
			 
		else if ( currTok ==  lbrac )
		{
			int ttp;
			nextSym();
			expr( ttp);
			while( currTok == comma )
			{
				nextSym();
				expr( ttp);
			}
			expect(rbrac); 
		}
		else if ( currTok == hat )
			nextSym();
		/*else if ( currTok == lparen )
		{
			fputs("Am I here?\n", stdout);
			nextSym();
			qualident();
			expect(rparen);
		}*/
		
	}
	fputs("Done designator\n", stdout);
}

void set()
{
	fputs("This is set\n", stdout);
	int ttp;
	do
	{
		expr( ttp);
		if ( currTok == period )
		{
			nextSym();
			if ( currTok == period )
			{
				expr( ttp);
			}
		}
	}while ( currTok == comma ); 

	expect(rcurly);
	fputs("Done set\n", stdout);
}

void factor( int ttp)
{
	fputs("This is factor\n", stdout);
	if ( currTok == string | currTok == number | currTok == NIL_SYM | currTok == TRUE_SYM | currTok == FALSE_SYM )
	{
		nextSym();
	}
	else if ( currTok == ident )
	{
		fputs("Why not here??\n", stdout);
		designator();
		if( currTok == lparen)
		{
			nextSym();
			ActParams();	
		}
	
	}
	else if ( currTok == lparen )
	{
		nextSym();
		expr( ttp);
		fputs("Here... \n", stdout);
		expect(rparen);
	}
	else if ( currTok == tilde )
	{
		nextSym();
		factor( ttp);
	}
	else if ( currTok == lcurly )
	{
		nextSym();
		set();
	}
	fputs("Done factor\n", stdout);
}

void term( int ttp)
{
	fputs("This is term \n", stdout);
	factor( ttp);
	while ( currTok == mul | currTok == slash | currTok == DIV_SYM | currTok == MOD_SYM | currTok == AND_SYM )
	{
		factor( ttp);
	}
	fputs("Done term\n", stdout);
}

void StatSeq ()
{
	fputs("This is statseq\n", stdout);
	stat();
	while (currTok == SEMIC)
	{
		nextSym();
		stat();
	}
	fputs("Exiting StatSeq\n", stdout);
	
}

void AssignStat ()
{
	fputs("This is AssignStat\n", stdout);
	int ttp;
	expr( ttp);
	fputs("Done AssignStat\n", stdout);
}

void RepeatStat ()
{
	fputs("Start RepeatStat \n", stdout);
	int ttp;
	StatSeq();
	expect(UNTIL_SYM);
	expr( ttp);
	fputs("Done RepeatStat\n", stdout);
}

/*
void ProcCall ()
{
	fputs("This is ProcCall\n", stdout);
}
*/

void IfStat()
{
	int ttp;
	fputs("This is IfStat\n", stdout);
	expr( ttp);
	expect(THEN_SYM);
	StatSeq();
	
	while ( currTok ==  ELSIF_SYM)
	{
		nextSym();
		expr( ttp);
		nextSym();
		expect(THEN_SYM);
		StatSeq();
	}
	if ( currTok == ELSE_SYM )
	{
		nextSym();
		StatSeq();
	}
	expect(END_SYM);
	fputs("Done IfStat\n", stdout);
}

void caseP ()
{
	fputs("CASE HERE YO\n", stdout);
	if( currTok == string | currTok == number | currTok == ident )
	{
		do
		{
			nextSym();
			if ( currTok == period )
			{
				nextSym();
				if ( currTok == period )
				{
					nextSym();
					if ( currTok ==  string | currTok == number | currTok == ident)
					{
						nextSym();
					}
						
				}
			}
			
		}while ( currTok == comma);
		
		expect(SEMIC);
		
		StatSeq();		
	}
	fputs("CASE END YO\n", stdout);
	
}

void CaseStat ()
{
	fputs("This is CaseStat\n", stdout);
	int ttp;
	expr( ttp);
	expect(OF_SYM);
	caseP();
	while (currTok == OR_SYM)
	{
		nextSym();
		caseP();
	}
	expect(END_SYM);
	fputs("Done CaseStat\n", stdout);
}

void WhileStat()
{
	fputs("Start WhileStat \n", stdout);
	int ttp;
	expr( ttp);
	expect(DO_SYM);
	StatSeq();
	while ( currTok == ELSIF_SYM)
	{
		nextSym();
		expr( ttp);
		expect(DO_SYM);
		StatSeq();
	}
	expect(END_SYM);
	fputs("Done WhileStat\n", stdout);
}

void ForStat()
{
	int ttp1, ttp2;
	fputs("Start ForStat\n", stdout);
	expect(ident);
	expect(assign);
	expr( ttp1);
	expect(TO_SYM);
	expr( ttp2);
	if (currTok == BY_SYM)
	{
		nextSym();
		expr( ttp1);
	}
	expect(DO_SYM);
	StatSeq();
	expect(END_SYM);
	fputs("Done ForStat\n", stdout);
}

void stat ()
{
	int ttp;
	fputs("ENTERING stat\n", stdout);
	//  REPEATSTAT
	if ( currTok == REPEAT_SYM )
	{
		RepeatStat();
	} 
	// ASSIGNSTAT or PROC CALL
	else if ( currTok == ident )
	{
		int stp = searchid(currWord);
		designator();

		if( currTok == assign)
		{
			nextSym();
			AssignStat();
		}
		else if( currTok == lparen )
		{
			nextSym();
			ActParams();
		}	
		
	}
	// IFSTAT
	else if ( currTok == IF_SYM)
	{
		nextSym();
		IfStat();
		
	}
	// CASESTAT
	else if ( currTok == CASE_SYM )
	{
		nextSym();
		CaseStat();
			
	}	
	// WHILESTAT
	else if ( currTok == WHILE_SYM )
	{
		fputs("This is whileSYM\n", stdout);
		nextSym();
		WhileStat();
		
	} 
	// FORSTAT
	else if ( currTok == FOR_SYM)
	{
		nextSym();
		ForStat();
	}
	fputs("Done stat\n", stdout);
}

void FormParams ()
{
	fputs("This is FormParams\n", stdout);
	do
	{
		if(currTok == VAR_SYM)
			nextSym();

		expect(ident);

		while (currTok == comma)
		{
			nextSym();
			expect(ident);
		}

		expect(colon);
		
		if ( currTok == ARRAY_SYM )
			expect(OF_SYM);
		qualident();
		//nextSym();

	}while(currTok == SEMIC);

	expect(rparen);
	expect(colon);
	qualident();
	fputs("End FormParams\n", stdout);
}

void ImportList ()
{
	fputs("This is importList\n", stdout);
	//expect(IMPORT_SYM);

	expect(ident);
	if(currTok == assign)
	{
		nextSym();
		expect(ident);
	}
	
	while(currTok == comma)
	{
		nextSym();
		expect(ident);
		if(currTok == assign)
		{
			nextSym();
			expect(ident);
		}
	}

	expect(SEMIC);
	
	fputs("End ImportList\n", stdout);
}

void StrucType ()
{
	fputs("This is StrucType\n", stdout);
	int ttp;
	if(currTok == RECORD_SYM)
	{
		nextSym();
		enterScope();
		if(currTok == lparen)
		{
			nextSym();
			qualident();
			expect(rparen);
		}
		
		do
		{
			expect(ident);
			if(currTok == mul)
				nextSym();

			if(currTok == comma)
			{
				nextSym();
				expect(ident);
				if(currTok == mul)
					nextSym();
			}
			expect(colon);
			type( ttp);
		}while(currTok == SEMIC);
		
		exitScope();
		expect(END_SYM);
	}
	else if(currTok == ARRAY_SYM)
	{
		
		do
		{
			nextSym();
			expr( ttp);
		}while(currTok == comma);

		expect(OF_SYM);
		type( ttp);
	}
	else if(currTok == POINTER_SYM)
	{
		nextSym();
		expect(TO_SYM);
		type( ttp);
	}
	else if(currTok == PROCEDURE_SYM)
	{
		nextSym();
		if(currTok == lparen)
			nextSym();
			FormParams();
	}
	fputs("Done StrucType\n", stdout);
}

void ProcDecl ()
{
	fputs("Start ProcDecl\n", stdout);
	int displ = -2;							// displacement for param addr
	expect(ident);
	if(currTok == mul)
		nextSym();
	
	enterScope();

	if(currTok == lparen)
	{
		nextSym();
		FormParams();
	}
		
	expect(SEMIC);

	/* ProcBody */
	DeclSeq();
	if(currTok == BEGIN_SYM)
	{
		nextSym();
		StatSeq();
	}
	
	if(currTok == RETURN_SYM)
	{	
		nextSym();
		int ttpR;
		expr( ttpR);
	}
	expect(END_SYM);
	expect(ident);
	exitScope();
	fputs("Done ProcDecl\n", stdout);
}

void DeclSeq ()
{
	fputs("This is DeclSeq\n", stdout);

	if (currTok == CONST_SYM)		/* ConstDecl */
	{
		int ttpC;	
		nextSym();
		while (currTok == ident)
		{
			nextSym();
			if(currTok == mul)
				nextSym();
			expect(equal);
			expr( ttpC);
			expect(SEMIC);
		}
	}
	else if (currTok == TYPE_SYM)	/* TypeDecl */
	{
		nextSym();
		while(currTok == ident)
		{
			nextSym();
			if(currTok == mul)
				nextSym();
			expect(equal);
			StrucType();
			expect(SEMIC);
		}
	}
	else if (currTok == VAR_SYM)	/* VarDecl */
	{
		nextSym();
		while(currTok == ident)
		{
			int ttpV;
			insertid( currWord, varcls);
			printsymtab();
			nextSym();
			if(currTok == mul)
				nextSym();
			if(currTok == comma)
			{
				nextSym();
				
				if (currTok == ident)
				{
					insertid(currWord, varcls);
					printsymtab();
					nextSym();
				}

				if(currTok == mul)
					nextSym();
				expect(colon);

			}
			type( ttpV);
			expect(SEMIC);
		}

	}
	while (currTok == PROCEDURE_SYM)
	{
			nextSym();
			ProcDecl();
			expect(SEMIC);
	}

	fputs("End of DeclSeq\n", stdout);
}

void Module ()
{
	fputs("Enter Module \n", stdout);
	nextSym();
	expect(MODULE_SYM);
	expect(ident);
	expect(SEMIC);

	enterScope();

	if(currTok == IMPORT_SYM)
	{
		nextSym();
		ImportList();
	}
		

	DeclSeq();

	if(currTok == BEGIN_SYM)
	{
		nextSym();
		StatSeq();
	}
		

	expect(END_SYM);
	expect(ident);
	if( currTok != period )
	{
		fputs("\nln: ", stdout);
		printf("%d", lineNo);
		fputs("  ERROR: Unexpexted token ", stdout);
		fputs(symNames[currTok][0], stdout);
		fputs(". ", stdout);
		fputs(symNames[period][0], stdout);
		fputs(" expected\n\n", stdout);
		
		fputs(currLine, stdout);
		
		int i;
		for(i = 0; i < inptr-2; i++)
			fputc('-', stdout);
		fputs("^\n\n", stdout);
	}	

	fputs("Reached the end of Module", stdout);
}














