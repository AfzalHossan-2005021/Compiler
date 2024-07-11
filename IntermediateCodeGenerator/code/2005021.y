%code top {
	#include<iostream>
	#include<fstream>
	#include <sstream>
}

%code requires {
	#include "lex_utils.h"
	#include "ast_utils.h"
}

%union
{
	SymbolInfo *symbolInfoPtr;
	ASTVariableNode *astVariableNodePtr;
	ASTFunctionNode *astFunctionNodePtr;
	ASTInternalNode *astInternalNodePtr;
	ASTNode *astNodePtr;
	AST *astPtr;
}

%code provides {
	#define YY_DECL\
  		yytoken_kind_t yylex (YYSTYPE* yylval, YYLTYPE* yylloc)
  	YY_DECL;

  	void yyerror (const YYLTYPE *loc, string msg);
}

%code{
	using namespace std;

	extern FILE* yyin;
	ofstream errorFile;
	ofstream parseTreeFile;
	ofstream logFile;

	AST *ast;
	SymbolTable *symbolTable;
	VariableList *parameterList;
	VariableList *variableList;
	FunctionInfo *currentFunctionInfo;
	int funcStackOffset;

	int totalLines = 0, totalErrors = 0;

	void writeError(string msg);
	void writeLog(string msg);
}

%define api.pure full
%define api.token.prefix {TOKEN_}

%token <symbolInfoPtr> IF ELSE FOR WHILE INT FLOAT VOID RETURN
%token <symbolInfoPtr> ADDOP MULOP INCOP DECOP RELOP ASSIGNOP LOGICOP NOT
%token <symbolInfoPtr> LPAREN RPAREN LCURL RCURL LSQUARE RSQUARE COMMA SEMICOLON
%token <symbolInfoPtr> PRINTLN
%token <symbolInfoPtr> CONST_INT
%token <symbolInfoPtr> CONST_FLOAT
%token <symbolInfoPtr> ID

%type <astVariableNodePtr> parameter_list declaration_list arguments variable
%type <astFunctionNodePtr> func_declaration func_definition
%type <astInternalNodePtr> start program unit compound_statement var_declaration statements statement expression_statement expression logic_expression rel_expression simple_expression term unary_expression factor type_specifier argument_list

%precedence RPAREN
%precedence ELSE

%left ADDOP MULOP RELOP LOGICOP INCOP DECOP
%right ASSIGNOP NOT

%start start

%%

start : program {
		string rule = "start : program";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		writeLog("start : program " );
		ast->setRoot($$);
	}
;

program : program unit {
		string rule = "program : program unit";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1)->addChild($2);
		writeLog("program : program unit " ) ; 
	}
	| unit {
		string rule = "program : unit";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		writeLog("program : unit "  ) ; 
	}
;

unit : func_declaration {
		string rule = "unit : func_declaration";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		writeLog("unit : func_declaration ");
	}
	| func_definition {
		string rule = "unit : func_definition";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		writeLog("unit : func_definition  ");
	}
	| var_declaration {
		string rule = "unit : var_declaration";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		writeLog("unit : var_declaration  " );
	}
;

func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON {
		string rule = "func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON";

		FunctionInfo *functionInfo = new FunctionInfo($2->getName(), $1->getTypeSpecifier(), parameterList);
		parameterList = new VariableList();

		$$ = new ASTFunctionNode(rule, functionInfo, @$.first_line, @$.last_line);
		$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild(new ASTLeafNode($3, @3.first_line))->addChild($4)->addChild(new ASTLeafNode($5, @5.first_line))->addChild(new ASTLeafNode($6, @6.first_line));

		SymbolInfo* symbolInfo = symbolTable->LookUp($2->getName());
		if(symbolInfo == nullptr){
			symbolTable->Insert(functionInfo);
		} else if(symbolInfo->getType() == "VARIABLE"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": '" + symbolInfo->getName() + "' redeclared as different kind of symbol";
			writeError(errorMsg);
		} else if(symbolInfo->getType() == "FUNCTION" && !functionInfo->isCompatibleWith((FunctionInfo*)symbolInfo)){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Conflicting types for '" + symbolInfo->getName() + "'";
			writeError(errorMsg);
		} else{
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Multiple definition of '" + symbolInfo->getName() + "'";
			writeError(errorMsg);
		}
		writeLog("func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON ");
	}
	| type_specifier ID LPAREN RPAREN SEMICOLON {
		string rule = "func_declaration : type_specifier ID LPAREN RPAREN SEMICOLON";

		FunctionInfo *functionInfo = new FunctionInfo($2->getName(), $1->getTypeSpecifier());

		SymbolInfo* symbolInfo = symbolTable->LookUp($2->getName());
		if(symbolInfo == nullptr){
			symbolTable->Insert(functionInfo);
		} else if(symbolInfo->getType() == "VARIABLE"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": '" + symbolInfo->getName() + "' redeclared as different kind of symbol";
			writeError(errorMsg);
		} else if(symbolInfo->getType() == "FUNCTION" && !functionInfo->isCompatibleWith((FunctionInfo*)symbolInfo)){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Conflicting types for '" + symbolInfo->getName() + "'";
			writeError(errorMsg);
		} else{
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Multiple definition of '" + symbolInfo->getName() + "'";
			writeError(errorMsg);
		}

		$$ = new ASTFunctionNode(rule, functionInfo, @$.first_line, @$.last_line);
		$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild(new ASTLeafNode($3, @3.first_line))->addChild(new ASTLeafNode($4, @4.first_line))->addChild(new ASTLeafNode($5, @5.first_line));
		writeLog("func_declaration : type_specifier ID LPAREN RPAREN SEMICOLON ");
	}
;

func_definition : type_specifier ID LPAREN parameter_list RPAREN {
		FunctionInfo *functionInfo = new FunctionInfo($2->getName(), $1->getTypeSpecifier(), parameterList);

		SymbolInfo* symbolInfo = symbolTable->LookUp($2->getName());

		 if(symbolInfo == nullptr){
			functionInfo -> setDefined();
			symbolTable->Insert(functionInfo);
		}
		else if(symbolInfo->getType() == "VARIABLE"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": '" + symbolInfo->getName() + "' redeclared as different kind of symbol";
			writeError(errorMsg);
		}
		else if(symbolInfo->getType() == "FUNCTION"){
			FunctionInfo *tmpFunctionInfo = (FunctionInfo*)symbolInfo;
			if(tmpFunctionInfo->isDefined()){
				string errorMsg = "Line# " + to_string(@2.first_line) + ": Multiple definition of '" + tmpFunctionInfo->getName() + "'";
				writeError(errorMsg);
			}
			else if(!functionInfo->isCompatibleWith(tmpFunctionInfo)){
				string errorMsg = "Line# " + to_string(@2.first_line) + ": Conflicting types for '" + tmpFunctionInfo->getName() + "'";
				writeError(errorMsg);
			}
			else{
				delete functionInfo;
				functionInfo = tmpFunctionInfo;
			}
		}
		else{
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Multiple definition of '" + symbolInfo->getName() + "'";
			writeError(errorMsg);
		}
		currentFunctionInfo = functionInfo;

		symbolTable->EnterScope();
		funcStackOffset = 0;
		int paramOffset = 0;
		int paramCount = 0;
		if(parameterList != nullptr){
			VariableInfo *parameterListIterator = parameterList->getHead();
			while(parameterListIterator != nullptr){
				VariableInfo *tmpVariableInfo = new VariableInfo(parameterListIterator->getName(), parameterListIterator->getTypeSpecifier());
				tmpVariableInfo->setScopeId(symbolTable->getCurrentScopeTableId());
				paramOffset += 2;
				tmpVariableInfo->setParamOffset(paramOffset);
				paramCount++;
				bool isInserted = symbolTable->Insert(tmpVariableInfo);
				if(! isInserted) break;
				parameterListIterator = (VariableInfo*)(parameterListIterator->nestSymbolInfo);
			}
		}
		functionInfo->setParameterCount(paramCount);
	} compound_statement {
		string rule = "func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement";

		parameterList = new VariableList();

		writeLog("func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement ");
		if($4->isError()){
			string errorMsg = "Line# " + to_string(@$.first_line) + ": Syntax error at parameter list of function definition";
			writeError(errorMsg);
		}
		currentFunctionInfo->setFuncStackOffset(funcStackOffset);
		$$ = new ASTFunctionNode(rule, currentFunctionInfo, @$.first_line, @$.last_line);
		$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild(new ASTLeafNode($3, @3.first_line))->addChild($4)->addChild(new ASTLeafNode($5, @5.first_line))->addChild($7);
		
	}
	| type_specifier ID LPAREN RPAREN {
		FunctionInfo *functionInfo = new FunctionInfo($2->getName(), $1->getTypeSpecifier());
		
		SymbolInfo* symbolInfo = symbolTable->LookUp($2->getName());

		if(symbolInfo == nullptr){
			functionInfo -> setDefined();
			symbolTable->Insert(functionInfo);
		}
		else if(symbolInfo->getType() == "VARIABLE"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": '" + symbolInfo->getName() + "' redeclared as different kind of symbol";
			writeError(errorMsg);
		}
		else if(symbolInfo->getType() == "FUNCTION"){
			FunctionInfo *tmpFunctionInfo = (FunctionInfo*)symbolInfo;
			if(tmpFunctionInfo->isDefined()){
				string errorMsg = "Line# " + to_string(@2.first_line) + ": Multiple definition of '" + tmpFunctionInfo->getName() + "'";
				writeError(errorMsg);
			}
			else if(!functionInfo->isCompatibleWith(tmpFunctionInfo)){
				string errorMsg = "Line# " + to_string(@2.first_line) + ": Conflicting types for '" + tmpFunctionInfo->getName() + "'";
				writeError(errorMsg);
			}
			else{
				delete functionInfo;
				functionInfo = tmpFunctionInfo;
				functionInfo -> setDefined();
			}
		}
		else{
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Multiple definition of '" + symbolInfo->getName() + "'";
			writeError(errorMsg);
		}
		currentFunctionInfo = functionInfo;
		funcStackOffset = 0;
		symbolTable->EnterScope();

	} compound_statement {
		string rule = "func_definition : type_specifier ID LPAREN RPAREN compound_statement";

		currentFunctionInfo->setFuncStackOffset(funcStackOffset);

		$$ = new ASTFunctionNode(rule, currentFunctionInfo, @$.first_line, @$.last_line);
		$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild(new ASTLeafNode($3, @3.first_line))->addChild(new ASTLeafNode($4, @4.first_line))->addChild($6);

		writeLog("func_definition : type_specifier ID LPAREN RPAREN compound_statement");
	}
;

parameter_list : parameter_list COMMA type_specifier ID {
		string rule = "parameter_list : parameter_list COMMA type_specifier ID";
		VariableInfo *variableInfo = new VariableInfo($4->getName(), $3->getTypeSpecifier());

		if(parameterList->findVariable($4->getName())){
			string errorMsg = "Line# " + to_string(@4.first_line) + ": Redefinition of parameter '" + $4->getName() + "'";
			writeError(errorMsg);
		}
		$$ = new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line);
		if($1->isError()){
			$$->setError();
			$$->setRule("parameter_list : error");
			delete $1;
			delete $2;
			delete $3;
			delete $4;
		} else {
			$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3)->addChild(new ASTLeafNode($4, @4.first_line));
			parameterList->addVariable(variableInfo);
			writeLog("parameter_list  : parameter_list COMMA type_specifier ID");
		}
	}
	| parameter_list COMMA type_specifier {
		string rule = "parameter_list : parameter_list COMMA type_specifier";
		// VariableInfo *variableInfo = new VariableInfo(string(), $3->getTypeSpecifier());
		VariableInfo *variableInfo = new VariableInfo("NN", $3->getTypeSpecifier());
		$$ = new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line);
		if($1->isError()){
			$$->setError();
			$$->setRule("parameter_list : error");
			delete $1;
			delete $2;
			delete $3;
		} else {
			$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3);
			parameterList->addVariable(variableInfo);
			writeLog("parameter_list  : parameter_list COMMA type_specifier ");
		}
	}
	| type_specifier ID {
		string rule = "parameter_list : type_specifier ID";
		VariableInfo *variableInfo = new VariableInfo($2->getName(), $1->getTypeSpecifier());
		parameterList->addVariable(variableInfo);
		$$ = new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line);
		$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line));
		writeLog("parameter_list  : type_specifier ID");
	}
	| type_specifier {
		string rule = "parameter_list : type_specifier";
		// VariableInfo *variableInfo = new VariableInfo(string(), $1->getTypeSpecifier());
		VariableInfo *variableInfo = new VariableInfo("NN", $1->getTypeSpecifier());
		parameterList->addVariable(variableInfo);
		$$ = new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line);
		$$->addChild($1);
		writeLog("parameter_list  : type_specifier ");
	}
	| error {
		string rule = "parameter_list : error";
		$$ = new ASTVariableNode(rule, nullptr, @$.first_line, @$.last_line);
		$$->setError();
		yyclearin;
	}
;

compound_statement : LCURL statements RCURL {
		string rule = "compound_statement : LCURL statements RCURL";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild($2)->addChild(new ASTLeafNode($3, @3.first_line));
		writeLog("compound_statement : LCURL statements RCURL  ");
		logFile << symbolTable->PrintAllScopeTable();
		symbolTable->ExitScope();
	}
	| LCURL RCURL {
		string rule = "compound_statement : LCURL RCURL";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild(new ASTLeafNode($2, @2.first_line));
		writeLog("compound_statement : LCURL RCURL  ");
		logFile << symbolTable->PrintAllScopeTable();
		symbolTable->ExitScope();
	}
;

var_declaration : type_specifier declaration_list SEMICOLON {
		string rule = "var_declaration : type_specifier declaration_list SEMICOLON";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild($1)->addChild($2)->addChild(new ASTLeafNode($3, @3.first_line));
		$$->setTypeSpecifier($1->getTypeSpecifier());

		VariableInfo *variableListIterator = variableList->getHead();
		if($1->getTypeSpecifier() == "VOID"){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": Variable or field '" + variableListIterator->getName() + "' declared void";
			writeError(errorMsg);
		} else {
			while(variableListIterator != nullptr){
				SymbolInfo *symbolInfo = symbolTable->LookUpCurrentScope(variableListIterator->getName());
				if(symbolInfo == nullptr){
					variableListIterator->setTypeSpecifier($1->getTypeSpecifier());
					symbolTable->Insert(variableListIterator);
				}
				else if (symbolInfo->getType() == "FUNCTION"){
					string errorMsg = "Line# " + to_string(@1.first_line) + ": '" + symbolInfo->getName() + "' redeclared as different kind of symbol";
					writeError(errorMsg);
				}
				else {
					VariableInfo *tmpVariableInfo = (VariableInfo*)symbolInfo;
					if(tmpVariableInfo->getTypeSpecifier() != $1->getTypeSpecifier()){
						string errorMsg = "Line# " + to_string(@1.first_line) + ": Conflicting types for'" + tmpVariableInfo->getName() + "'";
						writeError(errorMsg);
					} else {
						string errorMsg = "Line# " + to_string(@1.first_line) + ": Redefinition of '" + tmpVariableInfo->getName() + "'";
						writeError(errorMsg);
					}
				}
				VariableInfo *tmpVariableInfo = variableListIterator;
				variableListIterator = (VariableInfo*)(variableListIterator->nestSymbolInfo);
				tmpVariableInfo->nestSymbolInfo = nullptr;
			}
		}
		if($2->isError()){
			string errorMsg = "Line# " + to_string(@$.first_line) + ": Syntax error at declaration list of variable declaration";
			writeError(errorMsg);
		}
		variableList = new VariableList();;
		writeLog("var_declaration : type_specifier declaration_list SEMICOLON  ");
	}
;

type_specifier : INT {
		string rule = "type_specifier : INT";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line));
		$$->setTypeSpecifier("INT");
		writeLog("type_specifier	: INT ");
	}
	| FLOAT {
		string rule = "type_specifier : FLOAT";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line));
		$$->addChild(new ASTLeafNode($1, @1.first_line));
		$$->setTypeSpecifier("FLOAT");
		writeLog("type_specifier	: FLOAT ");
	}
	| VOID {
		string rule = "type_specifier : VOID";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line));
		$$->addChild(new ASTLeafNode($1, @1.first_line));
		$$->setTypeSpecifier("VOID");
		writeLog("type_specifier	: VOID");
	}
;

declaration_list : declaration_list COMMA ID {
		string rule = "declaration_list : declaration_list COMMA ID";
		funcStackOffset += 2;
		VariableInfo *variableInfo = new VariableInfo($3->getName());
		variableInfo->setScopeId(symbolTable->getCurrentScopeTableId());
		variableInfo->setOffset(funcStackOffset);
		$$ = (new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line, symbolTable->getCurrentScopeTableId()));

		if($1->isError()){
			$$->setError();
			$$->setRule("declaration_list : error");
			delete $1;
			delete $2;
			delete $3;
		} else {
			writeLog("declaration_list : declaration_list COMMA ID  ");
			$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild(new ASTLeafNode($3, @3.first_line));
			variableList->addVariable(variableInfo);
		}
	}
	| declaration_list COMMA ID LSQUARE CONST_INT RSQUARE {
		string rule = "declaration_list : declaration_list COMMA ID LSQUARE CONST_INT RSQUARE";
		ArrayInfo *arrayInfo = new ArrayInfo($3->getName(), stoi($5->getName()));
		arrayInfo->setScopeId(symbolTable->getCurrentScopeTableId());
		funcStackOffset += 2 * arrayInfo->getArraySize();
		arrayInfo->setOffset(funcStackOffset);
		$$ = (new ASTVariableNode(rule, arrayInfo, @$.first_line, @$.last_line, symbolTable->getCurrentScopeTableId()));
		if($1->isError()){
			$$->setError();
			$$->setRule("declaration_list : error");
			delete $1;
			delete $2;
			delete $3;
			delete $4;
			delete $5;
			delete $6;
		} else {
			$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild(new ASTLeafNode($3, @3.first_line))->addChild(new ASTLeafNode($4, @4.first_line))->addChild(new ASTLeafNode($5, @5.first_line))->addChild(new ASTLeafNode($6, @6.first_line));
			writeLog("declaration_list : declaration_list COMMA ID LSQUARE CONST_INT RSQUARE ");
			variableList->addVariable(arrayInfo);
		}
	}
	| ID {
		string rule = "declaration_list : ID";
		VariableInfo *variableInfo = new VariableInfo($1->getName());
		variableInfo->setScopeId(symbolTable->getCurrentScopeTableId());
		funcStackOffset += 2;
		variableInfo->setOffset(funcStackOffset);
		variableList->addVariable(variableInfo);
		$$ = (new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line, symbolTable->getCurrentScopeTableId()));
		$$->addChild(new ASTLeafNode($1, @1.first_line));
		writeLog("declaration_list : ID ");
	}
	| ID LSQUARE CONST_INT RSQUARE {
		string rule = "declaration_list : ID LSQUARE CONST_INT RSQUARE";
		ArrayInfo *arrayInfo = new ArrayInfo($1->getName(), stoi($3->getName()));
		arrayInfo->setScopeId(symbolTable->getCurrentScopeTableId());
		funcStackOffset += 2 * arrayInfo->getArraySize();
		arrayInfo->setOffset(funcStackOffset);
		variableList->addVariable(arrayInfo);
		$$ = (new ASTVariableNode(rule, arrayInfo, @$.first_line, @$.last_line, symbolTable->getCurrentScopeTableId()));
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild(new ASTLeafNode($2, @2.first_line))->addChild(new ASTLeafNode($3, @3.first_line))->addChild(new ASTLeafNode($4, @4.first_line));
		writeLog("declaration_list : ID LSQUARE CONST_INT RSQUARE ");
	}
	| error {
		string rule = "declaration_list : error";
		$$ = new ASTVariableNode(rule, nullptr, @$.first_line, @$.last_line);
		$$->setError();
		yyclearin;
	}
;

statements : statement {
		string rule = "statements : statement";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line));
		$$->addChild($1);
		writeLog("statements : statement  ");
	}
	| statements statement {
		string rule = "statements : statements statement";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line));
		$$->addChild($1)->addChild($2);
		writeLog("statements : statements statement  ");
	}
;

statement : var_declaration {
		string rule = "statement : var_declaration";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line));
		$$->addChild($1);
		writeLog("statement : var_declaration ");
	}
	| expression_statement {
		string rule = "statement : expression_statement";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line));
		$$->addChild($1);
		writeLog("statement : expression_statement  ");
	}
	| {
		symbolTable->EnterScope();
	} compound_statement {
		string rule = "statement : compound_statement";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line));
		$$->addChild($2);
		writeLog("statement : compound_statement ");
	}
	| FOR LPAREN expression_statement expression_statement expression RPAREN statement {
		string rule = "statement : FOR LPAREN expression_statement expression_statement expression RPAREN statement";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3)->addChild($4)->addChild($5)->addChild(new ASTLeafNode($6, @6.first_line))->addChild($7);
		writeLog("statement : FOR LPAREN expression_statement expression_statement expression RPAREN statement");
	}
	| IF LPAREN expression RPAREN statement {
		string rule = "statement : IF LPAREN expression RPAREN statement";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3)->addChild(new ASTLeafNode($4, @4.first_line))->addChild($5);
		writeLog("statement : IF LPAREN expression RPAREN statement ");
	}
	| IF LPAREN expression RPAREN statement ELSE statement {
		string rule = "statement : IF LPAREN expression RPAREN statement ELSE statement";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3)->addChild(new ASTLeafNode($4, @4.first_line))->addChild($5)->addChild(new ASTLeafNode($6, @6.first_line))->addChild($7);
		writeLog("statement : IF LPAREN expression RPAREN statement ELSE statement ");
	}
	| WHILE LPAREN expression RPAREN statement {
		string rule = "statement : WHILE LPAREN expression RPAREN statement";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3)->addChild(new ASTLeafNode($4, @4.first_line))->addChild($5);
		writeLog("statement : WHILE LPAREN expression RPAREN statement");
	}
	| PRINTLN LPAREN ID RPAREN SEMICOLON {
		string rule = "statement : PRINTLN LPAREN ID RPAREN SEMICOLON";
		SymbolInfo *symbolInfo = symbolTable->LookUp($3->getName());

		if(symbolInfo == nullptr){
			string errorMsg = "Line# " + to_string(@3.first_line) + ": Undeclared variable";
			writeError(errorMsg);
		} else {
			$3 = symbolInfo;
		}
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild(new ASTLeafNode($2, @2.first_line))->addChild(new ASTLeafNode($3, @3.first_line))->addChild(new ASTLeafNode($4, @4.first_line))->addChild(new ASTLeafNode($5, @5.first_line));
		writeLog("statement : PRINTLN LPAREN ID RPAREN SEMICOLON ");
	}
	| RETURN expression SEMICOLON {
		string rule = "statement : RETURN expression SEMICOLON";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild($2)->addChild(new ASTLeafNode($3, @3.first_line));
		writeLog("statement : RETURN expression SEMICOLON");
	}
;

expression_statement : SEMICOLON {
		string rule = "expression_statement : SEMICOLON";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line));
		writeLog("	expression_statement : SEMICOLON		");
	}
	| expression SEMICOLON {
		string rule = "expression_statement : expression SEMICOLON";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line));
		$$->setTypeSpecifier($1->getTypeSpecifier());
		writeLog("expression_statement : expression SEMICOLON 		 ");
		if($1->isError()){
			string errorMsg = "Line# " + to_string(@$.first_line) + ": Syntax error at expression of expression statement";
			writeError(errorMsg);
		}
	}
;

variable : ID {
		string rule = "variable : ID";

		VariableInfo *variableInfo = new VariableInfo($1->getName());

		SymbolInfo *symbolInfo = symbolTable->LookUp($1->getName());
		if(symbolInfo == nullptr){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": Undeclared variable '" + $1->getName() + "'";
			writeError(errorMsg);
		} else {
			variableInfo = (VariableInfo*)symbolInfo;
			$1 = variableInfo;
		}
		$$ = new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line));
		$$->setTypeSpecifier(variableInfo->getTypeSpecifier());
		writeLog("variable : ID 	 ");
	}
	| ID LSQUARE expression RSQUARE {
		string rule = "variable : ID LSQUARE expression RSQUARE";

		VariableInfo *variableInfo = new VariableInfo($1->getName());

		SymbolInfo *symbolInfo = symbolTable->LookUp($1->getName());
		if(symbolInfo == nullptr){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": Undeclared variable '" + $1->getName() + "'";
			writeError(errorMsg);
		}
		else if(symbolInfo->getType() == "FUNCTION"){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": '" + symbolInfo->getName() + "' redeclared as different kind of symbol";
			writeError(errorMsg);
		}
		else if(symbolInfo->getType() == "VARIABLE"){
			string errorMsg = "Line# " + to_string(@1.first_line) +  ": '" + symbolInfo->getName() + "' is not an array";
			writeError(errorMsg);
		}
		else{
			variableInfo = (VariableInfo*)symbolInfo;
			$1 = variableInfo;
		}
		if($3->isError()){
			string errorMsg = "Line# " + to_string(@$.first_line) + ": Syntax error at expression of expression statement";
			writeError(errorMsg);
		} else if($3->getTypeSpecifier() != "INT"){
			string errorMsg = "Line# " + to_string(@3.first_line) + ": Array subscript is not an integer";
			writeError(errorMsg);
		}

		$$ = new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line);
		$$->addChild(new ASTLeafNode($1, @1.first_line))->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3)->addChild(new ASTLeafNode($4, @4.first_line));
		$$->setTypeSpecifier(variableInfo->getTypeSpecifier());
		writeLog("variable : ID LSQUARE expression RSQUARE  	 ");
	}
;

expression : logic_expression {
		string rule = "expression : logic_expression";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild($1);
		$$->setTypeSpecifier($1->getTypeSpecifier());
		writeLog("expression 	: logic_expression	 ");
	}
	| variable ASSIGNOP logic_expression {
		string rule = "expression : variable ASSIGNOP logic_expression";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3);
		$$->setTypeSpecifier($1->getTypeSpecifier());

		if($3->getTypeSpecifier() == "VOID"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Void cannot be used in expression ";
			writeError(errorMsg);
		}
		else if($1->getTypeSpecifier() == "INT" && $3->getTypeSpecifier() == "FLOAT"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Warning: possible loss of data in assignment of FLOAT to INT";
			writeError(errorMsg);
		}
		writeLog("expression 	: variable ASSIGNOP logic_expression 		 ");
	}
	| error {
		string rule = "expression : error";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->setError();
		yyclearin;
	}
;

logic_expression : rel_expression {
		string rule = "logic_expression : rel_expression";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		$$->setTypeSpecifier($1->getTypeSpecifier());
		writeLog("logic_expression : rel_expression 	 ");
	}
	| rel_expression LOGICOP rel_expression {
		string rule = "logic_expression : rel_expression LOGICOP rel_expression";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3);
		$$->setTypeSpecifier("INT");

		if($1->getTypeSpecifier() == "VOID" || $3->getTypeSpecifier() == "VOID"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": cannot use logical operator on void type";
			writeError(errorMsg);
		}
		writeLog("logic_expression : rel_expression LOGICOP rel_expression 	 	 ");
	}
;

rel_expression : simple_expression {
		string rule = "rel_expression : simple_expression";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		$$->setTypeSpecifier($1->getTypeSpecifier());
		writeLog("rel_expression	: simple_expression ");
	}
	| simple_expression RELOP simple_expression	{
		string rule = "rel_expression : simple_expression RELOP simple_expression";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3);
		$$->setTypeSpecifier("INT");
		if($1->getTypeSpecifier() == "VOID" || $3->getTypeSpecifier() == "VOID"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": cannot use relational operator on void type";
			writeError(errorMsg);
		}
		writeLog("rel_expression	: simple_expression RELOP simple_expression	  ");
	}
;

simple_expression : term {
		string rule = "simple_expression : term";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		$$->setTypeSpecifier($1->getTypeSpecifier());
		writeLog("simple_expression : term ");
	}
	| simple_expression ADDOP term {
		string rule = "simple_expression : simple_expression ADDOP term";

		if($1->getTypeSpecifier() == "VOID" || $3->getTypeSpecifier() == "VOID"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Void cannot be used in expression ";
			writeError(errorMsg);
		}
		
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3);
		if($1->getTypeSpecifier() == "FLOAT" || $3->getTypeSpecifier() == "FLOAT"){
			$$->setTypeSpecifier("FLOAT");
		} else {
			$$->setTypeSpecifier("INT");
		}
		writeLog("simple_expression : simple_expression ADDOP term  ");
	}
;

term : unary_expression {
		string rule = "term : unary_expression";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		$$->setTypeSpecifier($1->getTypeSpecifier());
		writeLog("term :	unary_expression ");
	}
	| term MULOP unary_expression {
		string rule = "term : term MULOP unary_expression";

		if($1->getTypeSpecifier() == "VOID" || $3->getTypeSpecifier() == "VOID"){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Void cannot be used in expression ";
			writeError(errorMsg);
		}
		else if($2->getName() == "%" && ($1->getTypeSpecifier() != "INT" || $3->getTypeSpecifier() != "INT")){
			string errorMsg = "Line# " + to_string(@2.first_line) + ": Operands of modulus must be integers ";
			writeError(errorMsg);
		}
		if($2->getName() == "/" || $2->getName() == "%"){
			if($3->isZero()){
				string errorMsg = "Line# " + to_string(@$.first_line) + ": Warning: division by zero i=0f=1Const=0";
				writeError(errorMsg);
			}
		}
		
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3);
		if($2->getName() == "%"){
			$$->setTypeSpecifier("INT");
		} else if($1->getTypeSpecifier() == "FLOAT" || $3->getTypeSpecifier() == "FLOAT"){
			$$->setTypeSpecifier("FLOAT");
		} else {
			$$->setTypeSpecifier("INT");
		}
		writeLog("term :	term MULOP unary_expression ");
	}
;

unary_expression : ADDOP unary_expression {
		string rule = "unary_expression : ADDOP unary_expression";
		if($2->getTypeSpecifier() == "VOID"){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": Void cannot be used in expression ";
			writeError(errorMsg);
		}
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild(new ASTLeafNode($1, @1.first_line))->addChild($2);
		$$->setTypeSpecifier($2->getTypeSpecifier());
		writeLog("unary_expression : ADDOP unary_expression ");
	}
	| NOT unary_expression {
		string rule = "unary_expression : NOT unary_expression";
		if($2->getTypeSpecifier() == "VOID"){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": cannot use logical operator on void type";
			writeError(errorMsg);
		}
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild(new ASTLeafNode($1, @1.first_line))->addChild($2);
		$$->setTypeSpecifier("INT");
		writeLog("unary_expression : NOT unary_expression  ");
	}
	| factor {
		string rule = "unary_expression : factor";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		$$->setTypeSpecifier($1->getTypeSpecifier());
		if($1->isZero()) $$->setZero();
		writeLog("unary_expression : factor ") ;
	}
;

factor : variable {
		string rule = "factor : variable";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1);
		$$->setTypeSpecifier($1->getTypeSpecifier());
		writeLog("factor	: variable ");
	}
	| ID LPAREN argument_list RPAREN {
		string rule = "factor : ID LPAREN argument_list RPAREN";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild(new ASTLeafNode($1, @1.first_line))->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3)->addChild(new ASTLeafNode($4, @4.first_line));

		SymbolInfo* symbolInfo = symbolTable->LookUp($1->getName());
		if(symbolInfo == nullptr){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": Undeclared function '" + $1->getName() + "'";
			writeError(errorMsg);
		}
		else if(symbolInfo->getType() == "VARIABLE"){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": '" + $1->getName() + "' is not a function";
			writeError(errorMsg);
		}
		else if(symbolInfo->getType() == "FUNCTION"){
			FunctionInfo *functionInfo = (FunctionInfo*)symbolInfo;
			if(functionInfo->getParameterCount() > variableList->getSize()){
				string errorMsg = "Line# " + to_string(@1.first_line) + ": Too few arguments to function '" + $1->getName() + "'";
				writeError(errorMsg);
			}
			else if(functionInfo->getParameterCount() < variableList->getSize()){
				string errorMsg = "Line# " + to_string(@1.first_line) + ": Too many arguments to function '" + $1->getName() + "'";
				writeError(errorMsg);
			}
			else if(functionInfo->getParameterList() != nullptr && !functionInfo->getParameterList()->isEqualTo(variableList)){
				VariableInfo *listIterator = variableList->getHead();
				VariableInfo *variableListIterator = functionInfo->getParameterList()->getHead();
				int argNo = 1;
				while (listIterator != nullptr && variableListIterator != nullptr){
					if (listIterator->getTypeSpecifier() != variableListIterator->getTypeSpecifier()){
						string errorMsg = "Line# " + to_string(@1.first_line) + ": Type mismatch for argument " + to_string(argNo) + " of '" + $1->getName() + "'";
						writeError(errorMsg);
					}
					argNo++;
					listIterator = (VariableInfo *)listIterator->nestSymbolInfo;
					variableListIterator = (VariableInfo *)variableListIterator->nestSymbolInfo;
				}
			}
			else if(!functionInfo->isDefined()){
				string errorMsg = "Line# " + to_string(@1.first_line) + ": Function '" + $1->getName() + "' is not defined";
				writeError(errorMsg);
			}
			else{
				$$->setTypeSpecifier(functionInfo->getReturnType());
			}
		}
		variableList = new VariableList();;
		writeLog("factor	: ID LPAREN argument_list RPAREN  ");
	}
	| LPAREN expression RPAREN {
		string rule = "factor : LPAREN expression RPAREN";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild(new ASTLeafNode($1, @1.first_line))->addChild($2)->addChild(new ASTLeafNode($3, @3.first_line));
		$$->setTypeSpecifier($2->getTypeSpecifier());
		writeLog("factor	: LPAREN expression RPAREN   ");
	}
	| CONST_INT {
		string rule = "factor : CONST_INT";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild(new ASTLeafNode($1, @1.first_line));
		$$->setTypeSpecifier("INT");
		if(stoi($1->getName()) == 0){
			$$->setZero();
		}
		writeLog("factor	: CONST_INT   ");
	}
	| CONST_FLOAT {
		string rule = "factor : CONST_FLOAT";
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild(new ASTLeafNode($1, @1.first_line));
		$$->setTypeSpecifier("FLOAT");
		if(stof($1->getName()) == 0.0){
			$$->setZero();
		}
		writeLog("factor	: CONST_FLOAT   ");
	}
	| variable INCOP {
		string rule = "factor : variable INCOP";
		if($1->getTypeSpecifier() != "INT"){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": Operands of increment operator must be integers";
			writeError(errorMsg);
		}
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line));
		$$->setTypeSpecifier("INT");
		writeLog("factor	: variable INCOP   ");
	}
	| variable DECOP {
		string rule = "factor : variable DECOP";
		if($1->getTypeSpecifier() != "INT"){
			string errorMsg = "Line# " + to_string(@1.first_line) + ": Operands of decrement operator must be integers";
			writeError(errorMsg);
		}
		$$ = (new ASTInternalNode(rule, @$.first_line, @$.last_line))->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line));
		$$->setTypeSpecifier("INT");
		writeLog("factor	: variable DECOP   ");
	}
;

argument_list : arguments {
		string rule = "argument_list : arguments";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		$$->addChild($1);
		writeLog("argument_list : arguments  ");
	}
	| {
		string rule = "argument_list : ";
		$$ = new ASTInternalNode(rule, @$.first_line, @$.last_line);
		writeLog("argument_list :");
	}
	;

arguments : arguments COMMA logic_expression {
		string rule = "arguments : arguments COMMA logic_expression";
		VariableInfo *variableInfo = new VariableInfo("NN", $3->getTypeSpecifier());
		variableList->addVariable(variableInfo);
		$$ = new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line);
		$$->addChild($1)->addChild(new ASTLeafNode($2, @2.first_line))->addChild($3);
		writeLog("arguments : arguments COMMA logic_expression ");
	}
	| logic_expression {
		string rule = "arguments : logic_expression";
		VariableInfo *variableInfo = new VariableInfo("NN", $1->getTypeSpecifier());
		variableList->addVariable(variableInfo);
		$$ = new ASTVariableNode(rule, variableInfo, @$.first_line, @$.last_line);
		$$->addChild($1);
		$$->setTypeSpecifier($1->getTypeSpecifier());
		writeLog("arguments : logic_expression");
	}
	| error {
		string rule = "arguments : error";
		$$ = new ASTVariableNode(rule, nullptr, @$.first_line, @$.last_line);
		$$->setError();
		yyclearin;
	}
;

%%

string removeComma(string str){
	string newStr = "";
	for(int i = 0; i < str.length(); i++){
		if(str[i] != ','){
			newStr += str[i];
		}
	}
	return newStr;
}

void optiemizeIntermediateCode(string inputFileName, string outputFileName){
	ifstream inputFile(inputFileName);
	ofstream outputFile(outputFileName);
	string line, prevLine;
	string reg1, reg2;
	string prevReg1, prevReg2;
	string op;
	string prevOp;
	bool prevLineDeleted = false;


	while(getline(inputFile, line)){
		istringstream iss(line);
		iss >> op;
		if(op == "PUSH"){
			iss >> prevReg1;
			prevReg2 = "";
			if(prevOp == "PUSH" || prevOp == "MOV"){
				outputFile << prevLine << endl;
			}
		}
		else if(op == "POP"){
			iss >> reg1;
			if(prevOp == "PUSH"){
				if(prevReg1 != reg1){
					outputFile << prevLine << endl;
					outputFile << line << endl;
				}
			} else {
				if(prevOp == "MOV"){
					outputFile << prevLine << endl;
				}
				outputFile << line << endl;
			}
			prevReg1 = reg1;
			prevReg2 = "";
		}
		else if(op == "MOV"){
			iss >> reg1 >> reg2;
			reg1 = removeComma(reg1);
			if(prevOp == "MOV"){
				if(prevReg1 == reg2 && prevReg2 == reg1){
					if(!prevLineDeleted){
						outputFile << prevLine << endl;
					}
					prevLineDeleted = true;
				}
				else {
					outputFile << prevLine << endl;
					prevLineDeleted = false;
				}
			}
			else {
				if(prevOp == "PUSH"){
					outputFile << prevLine << endl;
				}
			}
			prevReg1 = reg1;
			prevReg2 = reg2;
		}
		else {
			if(prevOp == "PUSH" || prevOp == "MOV"){
				outputFile << prevLine << endl;
				prevLineDeleted = false;
			}
			outputFile << line << endl;
		}
		prevLine = line;
		prevOp = op;
	}
	inputFile.close();
	outputFile.close();
}

void yyerror (const YYLTYPE* loc, string msg){
	logFile << "Error at line no " << loc->first_line << " : " << msg << endl;
}

void writeError(string msg){
	totalErrors++;
	errorFile << msg << endl;
}

void writeLog(string msg){
	logFile << msg << endl;
}

int main(int argc, char const *argv[]){
    if (argc != 2){
        cout<< "Usage: ./a.out <input_file>" << endl;
        exit(1);
    }
	yyin = fopen(argv[1] ,"r") ; 
	if(yyin == NULL){
		cout<<"Cannot Open Input File." << endl;
		exit(1);
	}

	ast = new AST();
	symbolTable = new SymbolTable(11);
	parameterList = new VariableList();
	variableList = new VariableList();

	parseTreeFile.open("parsetree.txt");
	errorFile.open("error.txt");
	logFile.open("log.txt");

	yyparse();

	ast->printTree(parseTreeFile);
	logFile << "Total Lines: " << totalLines << endl;
	logFile << "Total Errors: " << totalErrors << endl;
	ast->generateIntermediateCode("code.asm", symbolTable);
	optiemizeIntermediateCode("code.asm", "optimized_code.asm");

	fclose(yyin);
	logFile.close();
	errorFile.close();
	parseTreeFile.close();

  	return 0;
}