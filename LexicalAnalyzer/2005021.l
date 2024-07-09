%option noyywrap
%option lex-compat

%x CHARACTER
%x STRING
%x INTEGER
%x FLOATING_POINT
%x EXPONENTIAL_FLOATING_POINT
%x SINGLE_LINE_COMMENT
%x MULTI_LINE_COMMENT
%x LINE_STARTING

%{
    #include<stdio.h>
    #include<iostream>
    #include "2005021.h"

    SymbolTable *symbolTable;

    FILE *tokenOut, *logOut;

    string token_buf, log_buf;
    
    int stateStartingLineNo;
    int errorCount = 0, warningCount = 0;
    int scopeSpaceCount = 0, lineSpaceCount = 0, charCount = 0;
    bool isMultiLineString, hasManyDeciamal, isUnrecongizedChar;

    void checkIndentation(){
        if(lineSpaceCount % 4 != 0){
            warningCount++;
            fprintf(logOut,"Line# %d: Warning, tab requrired but got space.\n", yylineno);
        }else if(lineSpaceCount != scopeSpaceCount){
            warningCount++;
            fprintf(logOut,"Line# %d: warning, %d of tabs needed but got %d tabs.\n", yylineno, scopeSpaceCount/4, lineSpaceCount/4);
        }
    }

    const char *getKeywordToken(){
        if(strcmp(yytext, "if") == 0) return "IF";        
        else if(strcmp(yytext, "else") == 0) return "ELSE";
        else if(strcmp(yytext, "for") == 0) return "FOR";
        else if(strcmp(yytext, "while") == 0) return "WHILE";
        else if(strcmp(yytext, "do") == 0) return "DO";
        else if(strcmp(yytext, "break") == 0) return "BREAK";
        else if(strcmp(yytext, "int") == 0) return "INT";
        else if(strcmp(yytext, "char") == 0) return "CHAR";
        else if(strcmp(yytext, "float") == 0) return "FLOAT";
        else if(strcmp(yytext, "double") == 0) return "DOUBLE";
        else if(strcmp(yytext, "void") == 0) return "VOID";
        else if(strcmp(yytext, "return") == 0) return "RETURN";
        else if(strcmp(yytext, "switch") == 0) return "SWITCH";
        else if(strcmp(yytext, "case") == 0) return "CASE";
        else if(strcmp(yytext, "default") == 0) return "DEFAULT";
        else if(strcmp(yytext, "continue") == 0) return "CONTINUE";
        else return NULL;
    }

    const char* getOperatorToken(){
        if(strcmp(yytext, "+") == 0 || strcmp(yytext, "-") == 0) return "ADDOP";
        else if(strcmp(yytext, "*") == 0 || strcmp(yytext, "/") == 0 || strcmp(yytext, "%") == 0) return "MULOP";
        else if(strcmp(yytext, "++") == 0 || strcmp(yytext, "--") == 0) return "INCOP";
        else if(strcmp(yytext, "<") == 0 || strcmp(yytext, "<=") == 0 || strcmp(yytext, ">") == 0 || strcmp(yytext, ">=") == 0 || strcmp(yytext, "==") == 0 || strcmp(yytext, "!=") == 0) return "RELOP";
        else if(strcmp(yytext, "=") == 0) return "ASSIGNOP";
        else if(strcmp(yytext, "&&") == 0 || strcmp(yytext, "||") == 0) return "LOGICOP";
        else if(strcmp(yytext, "&") == 0 || strcmp(yytext, "|") == 0 || strcmp(yytext, "^") == 0 || strcmp(yytext, "<<") == 0 || strcmp(yytext, ">>") == 0) return "BITOP";
        else if(strcmp(yytext, "!") == 0) return "NOT";
        return NULL;
    }

    const char* getPunctuatorToken(){
        if(strcmp(yytext, "(") == 0) return "LPAREN";
        else if(strcmp(yytext, ")") == 0) return "RPAREN";
        else if(strcmp(yytext, "{") == 0){
            scopeSpaceCount += 4;
            symbolTable->EnterScope();
            return "LCURL";
        }
        else if(strcmp(yytext, "}") == 0){
            scopeSpaceCount -= 4;
            checkIndentation();
            symbolTable->ExitScope();
            return "RCURL";
        }
        else if(strcmp(yytext, "[") == 0) return "LSQUARE";
        else if(strcmp(yytext, "]") == 0) return "RSQUARE";
        else if(strcmp(yytext, ",") == 0) return "COMMA";
        else if(strcmp(yytext, ";") == 0) return "SEMICOLON";
        return NULL;
    }

    const char* getCharacterToken(){
        if(strcmp(yytext, "\\0") == 0) return "\0";
        else if(strcmp(yytext, "\\\"") == 0) return "\"";
        else if(strcmp(yytext, "\\t") == 0) return "\t";
        else if(strcmp(yytext, "\\\\") == 0) return "\\";
        else if(strcmp(yytext, "\\a") == 0) return "\a";
        else if(strcmp(yytext, "\\f") == 0) return "\f";
        else if(strcmp(yytext, "\\r") == 0) return "\r";
        else if(strcmp(yytext, "\\b") == 0) return "\b";
        else if(strcmp(yytext, "\\v") == 0) return "\v";
        else if(strcmp(yytext, "\\n") == 0) return "\n";
        else if(strcmp(yytext, "\\\'") == 0) return "\'";
        return NULL;
    }
%}

addops      [+-]
mulops      [*/%]
incops      ("++"|"--")
relops      ("<"|"<="|">"|">="|"=="|"!=")
assignop    (=)
logicops    ("&&"|"||")
bitops      ("&"|"|"|"^"|"<<"|">>")
not         (!)

digit       [0-9]
integer     {digit}+
floatingPoint ({integer}?\.{integer})

alphabet    [a-zA-Z]
alphaNumeric ({alphabet}|{digit})
identifier  ((_|{alphabet})(_|{alphaNumeric})*)
newLine     (\r\n|\n|\r)

keyword     (if|else|for|while|do|break|int|char|float|double|void|return|switch|case|default|continue)
operator    {addops}|{mulops}|{incops}|{relops}|{assignop}|{logicops}|{bitops}|{not}
punctuator  ("("|")"|"{"|"}"|"["|"]"|","|";")
escape      (\\0|\\\"|\\t|\\\\|\\a|\\f|\\r|\\b|\\v|\\n|\\\')

%%

{keyword}   {
    const char* token = getKeywordToken();
    fprintf(tokenOut,"<%s, %s>\n", token, yytext);
    fprintf(logOut,"Line# %d: Token <%s> Lexeme %s found\n", yylineno, token, yytext);
            }

{operator}  {
    const char* token = getOperatorToken();
    fprintf(tokenOut,"<%s, %s>\n", token, yytext);
    fprintf(logOut,"Line# %d: Token <%s> Lexeme %s found\n", yylineno, token, yytext);
            }

{punctuator}    {
    const char* token = getPunctuatorToken();
    fprintf(tokenOut,"<%s, %s>\n", token, yytext);
    fprintf(logOut,"Line# %d: Token <%s> Lexeme %s found\n", yylineno, token, yytext);
            }

{integer}   {
                token_buf = yytext;
                log_buf = yytext;
                hasManyDeciamal = false;
                BEGIN(INTEGER);
            }

<INTEGER>{
\.\.+{integer}? {
                token_buf += yytext;
                log_buf += yytext;
                hasManyDeciamal = true;
                BEGIN(FLOATING_POINT);
            }

[Ee][+-]?   {
                token_buf += yytext;
                log_buf += yytext;
                BEGIN(EXPONENTIAL_FLOATING_POINT);
            }

\.+[Ee][+-]? {
                token_buf += yytext;
                log_buf += yytext;
                hasManyDeciamal = true;
                BEGIN(EXPONENTIAL_FLOATING_POINT);
            }

\.?[_a-df-zA-DF-Z](_|{alphaNumeric})*   {
                errorCount++;
                log_buf += yytext;
                fprintf(logOut,"Error at line# %d: INVALID_ID_SUFFIX_NUM_PREFIX %s\n", yylineno, log_buf.c_str());
                BEGIN(INITIAL);
            }

{newLine}   {
                fprintf(tokenOut,"<CONST_INT, %s>\n", token_buf.c_str());
                fprintf(logOut,"Line# %d: Token <CONST_INT> Lexeme %s found\n", yylineno-1, log_buf.c_str());

                lineSpaceCount = 0;
                BEGIN(LINE_STARTING);
            }

<<EOF>>     |
[ \t\v]     {
                fprintf(tokenOut,"<CONST_INT, %s>\n", token_buf.c_str());
                fprintf(logOut,"Line# %d: Token <CONST_INT> Lexeme %s found\n", yylineno, log_buf.c_str());
                BEGIN(INITIAL);
            }

{operator}  |
{punctuator} {
                if(yyleng == 1){
                    yyless(yyleng-1);
                }else if(yyleng == 2){
                    yyless(yyleng-2);
                }
                fprintf(tokenOut,"<CONST_INT, %s>\n", token_buf.c_str());
                fprintf(logOut,"Line# %d: Token <CONST_INT> Lexeme %s found\n", yylineno, log_buf.c_str());
                BEGIN(INITIAL);
            }

.        {
                errorCount++;
                /* comment out next two line if token isn't to be created on getting unrecognized character */
                fprintf(tokenOut,"<CONST_INT, %s>\n", token_buf.c_str());
                fprintf(logOut,"Line# %d: Token <CONST_INT> Lexeme %s found\n", yylineno, log_buf.c_str());
                yyless(yyleng-1);
                BEGIN(INITIAL);
            }
}

{floatingPoint} {
                token_buf = yytext;
                log_buf = yytext;
                hasManyDeciamal = false;
                BEGIN(FLOATING_POINT);
            }

<FLOATING_POINT>{
[Ee][+-]?   {
                token_buf += yytext;
                log_buf += yytext;
                BEGIN(EXPONENTIAL_FLOATING_POINT);
            }

(\.+{integer}*)+ {
                log_buf += yytext;
                hasManyDeciamal = true;
            }

\.*{integer}*[_a-df-zA-DF-Z](\.|{alphaNumeric})*    {
                errorCount++;
                log_buf += yytext;
                fprintf(logOut,"Error at line# %d: INVALID_ID_SUFFIX_NUM_PREFIX %s\n", yylineno, log_buf.c_str());
                BEGIN(INITIAL);
            }

{newLine}   {
                if(hasManyDeciamal){
                    errorCount++;
                    fprintf(logOut, "Error at line# %d: TOO_MANY_DECIMAL_POINTS %s\n", yylineno-1, log_buf.c_str());
                }else{
                    fprintf(tokenOut,"<CONST_FLOAT, %s>\n", token_buf.c_str());
                    fprintf(logOut,"Line# %d: Token <CONST_FLOAT> Lexeme %s found\n", yylineno-1, log_buf.c_str());
                }
                lineSpaceCount = 0;
                BEGIN(LINE_STARTING);
            }

<<EOF>>     |
[ \t\v]     {
                if(hasManyDeciamal){
                    errorCount++;
                    fprintf(logOut, "Error at line# %d: TOO_MANY_DECIMAL_POINTS %s\n", yylineno, log_buf.c_str());
                }else{
                    fprintf(tokenOut,"<CONST_FLOAT, %s>\n", token_buf.c_str());
                    fprintf(logOut,"Line# %d: Token <CONST_FLOAT> Lexeme %s found\n", yylineno, log_buf.c_str());
                }
                BEGIN(INITIAL);
            }

{operator}  |
{punctuator} {
                if(yyleng == 1){
                    yyless(yyleng-1);
                }else if(yyleng == 2){
                    yyless(yyleng-2);
                }
                if(hasManyDeciamal){
                    errorCount++;
                    fprintf(logOut, "Error at line# %d: TOO_MANY_DECIMAL_POINTS %s\n", yylineno, log_buf.c_str());
                }else{
                    fprintf(tokenOut,"<CONST_FLOAT, %s>\n", token_buf.c_str());
                    fprintf(logOut,"Line# %d: Token <CONST_FLOAT> Lexeme %s found\n", yylineno, log_buf.c_str());
                }
                BEGIN(INITIAL);
            }

.           {
                errorCount++;
                /* comment out next two line if token isn't to be created on getting unrecognized character */
                fprintf(tokenOut,"<CONST_FLOAT, %s>\n", token_buf.c_str());
                fprintf(logOut,"Line# %d: Token <CONST_FLOAT> Lexeme %s found\n", yylineno, log_buf.c_str());
                yyless(yyleng-1);
                BEGIN(INITIAL);
            }
}

<EXPONENTIAL_FLOATING_POINT>{
{integer}   {
                token_buf += yytext;
                log_buf += yytext;
                if(hasManyDeciamal){
                    errorCount++;
                    fprintf(logOut, "Error at line# %d: TOO_MANY_DECIMAL_POINTS %s\n", yylineno, log_buf.c_str());
                }else{
                    fprintf(tokenOut,"<CONST_FLOAT, %s>\n", token_buf.c_str());
                    fprintf(logOut,"Line# %d: Token <CONST_FLOAT> Lexeme %s found\n", yylineno, log_buf.c_str());
                }
                BEGIN(INITIAL);
            }

{integer}?\.?{integer}([Ee][+-]?{integer}?\.?{integer})+ |
{floatingPoint} {
                errorCount++;
                log_buf += yytext;
                if(hasManyDeciamal){
                    errorCount++;
                    fprintf(logOut, "Error at line# %d: TOO_MANY_DECIMAL_POINTS %s\n", yylineno, log_buf.c_str());
                }else{
                    fprintf(logOut,"Error at line# %d: ILLFORMED_NUMBER %s\n", yylineno, log_buf.c_str());
                }
                BEGIN(INITIAL);
            }

<<EOF>>                         |
[ \t\v]                         |
{operator}                      |
{punctuator}                    |
{integer}?{identifier}          |
{floatingPoint}*{identifier}    {
                errorCount++;
                log_buf += yytext;
                errorCount++;
                fprintf(logOut,"Error at line# %d: INVALID_ID_SUFFIX_NUM_PREFIX %s\n", yylineno, log_buf.c_str());
                BEGIN(INITIAL);
            }
{newLine}   {
                fprintf(logOut,"Error at line# %d: INVALID_ID_SUFFIX_NUM_PREFIX %s\n", yylineno, log_buf.c_str());
                lineSpaceCount = 0;
                BEGIN(LINE_STARTING);
            }
.           {
                errorCount++;
                /* comment out next two line if token isn't to be created on getting unrecognized character */
                fprintf(tokenOut,"<CONST_FLOAT, %s>\n", token_buf.c_str());
                fprintf(logOut,"Line# %d: Token <CONST_FLOAT> Lexeme %s found\n", yylineno, log_buf.c_str());
                yyless(yyleng-1);
                BEGIN(INITIAL);
            }
}

'          {
                token_buf = "";
                log_buf = yytext;
                charCount = 0;
                isUnrecongizedChar = false;
                BEGIN(CHARACTER);
            }

<CHARACTER>{
'           {
                log_buf += yytext;
                if(charCount == 0){
                    errorCount++;
                    fprintf(logOut,"Error at line# %d: EMPTY_CONST_CHAR %s\n", yylineno, log_buf.c_str());
                }else{
                    if(charCount > 1){
                        errorCount++;
                        fprintf(logOut,"Error at line# %d: MULTICHAR_CONST_CHAR %s\n", yylineno, log_buf.c_str());
                    }else{
                        if(isUnrecongizedChar){
                            errorCount++;
                            fprintf(logOut,"Error at line# %d: UNRECOGNIZED_CHAR %s\n", yylineno, log_buf.c_str());
                        }else{
                            fprintf(tokenOut,"<CONST_CHAR, %s>\n", token_buf.c_str());
                            fprintf(logOut,"Line# %d: Token <CONST_CHAR> Lexeme %s found\n", yylineno, token_buf.c_str());
                        }
                    }
                }
                BEGIN(INITIAL);
            }
{escape}    {
                token_buf += getCharacterToken();
                log_buf += yytext;
                charCount++;
            }
\\[^0abfn\nr\rtv'\"\\] {
                log_buf += yytext;
                charCount++;
                isUnrecongizedChar = true;
            }
{newLine}   {
                errorCount++;
                fprintf(logOut,"Error at line# %d: UNFINISHED_CONST_CHAR %s\n", yylineno-1, log_buf.c_str());

                lineSpaceCount = 0;
                BEGIN(LINE_STARTING);
            }
<<EOF>>     {
                errorCount++;
                fprintf(logOut,"Error at line# %d: UNFINISHED_CONST_CHAR %s\n", yylineno, log_buf.c_str());
                BEGIN(INITIAL);
            }
.           {   
                token_buf += yytext;
                log_buf += yytext;
                charCount++;
            }
}

\"          {
                token_buf = "";
                log_buf = yytext;
                isMultiLineString = false; 
                stateStartingLineNo = yylineno;
                BEGIN(STRING);
            }

<STRING>{
\"          {
                log_buf += yytext;
                if(isMultiLineString){
                    fprintf(tokenOut,"<MULTI LINE STRING, %s>\n", token_buf.c_str());
                    fprintf(logOut,"Line# %d: Token <MULTI LINE STRING> Lexeme %s found\n", stateStartingLineNo, log_buf.c_str());
                }else{
                    fprintf(tokenOut,"<SINGLE LINE STRING, %s>\n", token_buf.c_str());
                    fprintf(logOut,"Line# %d: Token <SINGLE LINE STRING> Lexeme %s found\n", stateStartingLineNo, log_buf.c_str());
                }
                BEGIN(INITIAL);
            }

\\{newLine} {
                isMultiLineString = true;
                log_buf += yytext;
            }
{escape}    {
                token_buf += getCharacterToken();
                log_buf += yytext;
            }

{newLine}   {   
                errorCount++;
                fprintf(logOut,"Error at line# %d: UNFINISHED_STRING %s\n", yylineno-1, log_buf.c_str());

                lineSpaceCount = 0;
                BEGIN(LINE_STARTING);
            }

<<EOF>>     {   
                errorCount++;
                fprintf(logOut,"Error at line# %d: UNFINISHED_STRING %s\n", yylineno, log_buf.c_str());
                BEGIN(INITIAL);
            }

\\          |
[^\\\r\n\"]+  {
                token_buf += yytext;
                log_buf += yytext;
            }
}


"//"        {
                log_buf = yytext;
                stateStartingLineNo = yylineno;
                BEGIN(SINGLE_LINE_COMMENT);
            }

<SINGLE_LINE_COMMENT>{
\\          |
[^\\\r\n]+  |
\\{newLine} {
                log_buf += yytext;
            }

{newLine}   {
                lineSpaceCount = 0;
                fprintf(logOut,"Line# %d: Token <SINGLE LINE COMMENT> Lexeme %s found\n", stateStartingLineNo, log_buf.c_str());
                BEGIN(LINE_STARTING);
            }

<<EOF>>     {
                fprintf(logOut,"Line# %d: Token <SINGLE LINE COMMENT> Lexeme %s found\n", stateStartingLineNo, log_buf.c_str());
                BEGIN(INITIAL);
            }
}

"/*"        {
                log_buf = yytext;
                stateStartingLineNo = yylineno;
                BEGIN(MULTI_LINE_COMMENT);
            }

<MULTI_LINE_COMMENT>{

[^*\r\n]*       |
"*"+[^*/\r\n]*  |
{newLine}       {
                log_buf += yytext;
            }

"*"+"/"     {
                log_buf += yytext;
                fprintf(logOut,"Line# %d: Token <MULTI LINE COMMENT> Lexeme %s found\n", stateStartingLineNo, log_buf.c_str());
                BEGIN(INITIAL);
            }
<<EOF>>     {
                errorCount++;
                fprintf(logOut, "Error at line# %d: UNFINISHED_COMMENT %s\n", yylineno, log_buf.c_str());
                BEGIN(INITIAL);
            }
}

<LINE_STARTING>{
\t          {
                lineSpaceCount += 4;
            }

[ ]         {
                lineSpaceCount++;
            }            

{newLine}         {
                lineSpaceCount = 0;
            }

"}"         {
                yyless(yyleng-1);
                BEGIN(INITIAL);
            }

.           {
                checkIndentation();
                yyless(yyleng-1);
                BEGIN(INITIAL);
            }
            
}

{identifier} {
                fprintf(tokenOut,"<ID, %s>\n", yytext);
                fprintf(logOut,"Line# %d: Token <ID> Lexeme %s found\n", yylineno, yytext);

                int inserted = symbolTable->Insert(yytext, "ID");
                if(inserted){
                    fprintf(logOut, "%s", symbolTable->PrintAllScopeTable().c_str());
                }else{
                    fprintf(logOut, "\t%s already exists in the current ScopeTable\n", yytext);
                }
            }

{newLine}         {
                lineSpaceCount = 0;
                BEGIN(LINE_STARTING);
            }

[ \t\v\f]   {}

.           {
                errorCount++;
                fprintf(logOut,"Error at line# %d: UNRECOGNIZED_CHAR %s\n", yylineno, yytext);
            }

%%

int main(int argc,char *argv[]){
	
	if(argc != 2){
		printf("Please provide input file name and try again\n");
		return 0;
	}
	
	FILE *fin = fopen(argv[1], "r");
	if(fin==NULL){
		printf("Cannot open specified file\n");
		return 0;
	}

	logOut= fopen("2005021_log.txt","w");
	tokenOut= fopen("2005021_token.txt","w");

    symbolTable = new SymbolTable(10);

	yyin = fin;
    BEGIN(LINE_STARTING);
	yylex();

    fprintf(logOut, "%s", symbolTable->PrintAllScopeTable().c_str());
    fprintf(logOut,"Total lines: %d\n", yylineno);
    fprintf(logOut,"Total errors: %d\n", errorCount);
    fprintf(logOut,"Total warnings: %d\n", warningCount);

	fclose(yyin);
	fclose(tokenOut);
	fclose(logOut);
	return 0;
}