enum Token  { 	
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
		    };

Token currTok;
char currChar;
char [80] currLine;		// 80 character limit is arbitrary
int main()
{	
	// so cleeeeaaaaaan
	return 42;
}