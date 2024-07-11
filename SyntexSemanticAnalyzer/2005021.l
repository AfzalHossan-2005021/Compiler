%option noyywrap yylineno

%x INTEGER
%x FLOATING_POINT
%x EXPONENTIAL_FLOATING_POINT
%x SINGLE_LINE_COMMENT
%x MULTI_LINE_COMMENT

%{
    #include <iostream>
    #include <fstream>
    #include "2005021.h"
    #include "2005021.tab.h"
    #define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno;

    using namespace std;

    // void yyerror (string errMsg);

    extern ofstream logFile;
    extern int totalLines, totalErrors;
    string token_buf;
    bool hasManyDeciamal;
    
    void writeLog(SymbolInfo *symbolInfoPtr, int lineNo){
        logFile << "Line# " << lineNo << ": Token <" << symbolInfoPtr->getType() << "> Lexeme " << symbolInfoPtr->getName() << " found" << endl;
    }
    void writeErrorLog(int lineNo, string error){
        totalErrors++;
        logFile << "Error at line# " << lineNo << ": " << error << endl;
    }
%}

addops      [+-]
mulops      [*/%]
incop       ("++")
decop       ("--")
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
whitespace  ([ \t\v\f]+)
newLine     (\r\n|\n|\r)

operator    {addops}|{mulops}|{incop}|{decop}|{relops}|{assignop}|{logicops}|{bitops}|{not}
punctuator  ("("|")"|"{"|"}"|"["|"]"|","|";")

%%

if          {yylval->symbolInfoPtr = new SymbolInfo(yytext, "IF"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_IF;}
else        {yylval->symbolInfoPtr = new SymbolInfo(yytext, "ELSE"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_ELSE;}
for         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "FOR"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_FOR;}
while       {yylval->symbolInfoPtr = new SymbolInfo(yytext, "WHILE"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_WHILE;}
int         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "INT"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_INT;}
float       {yylval->symbolInfoPtr = new SymbolInfo(yytext, "FLOAT"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_FLOAT;}
void        {yylval->symbolInfoPtr = new SymbolInfo(yytext, "VOID"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_VOID;}
return      {yylval->symbolInfoPtr = new SymbolInfo(yytext, "RETURN"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_RETURN;}
println     {yylval->symbolInfoPtr = new SymbolInfo(yytext, "PRINTLN"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_PRINTLN;}

"("         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "LPAREN"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_LPAREN;}
")"         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "RPAREN"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_RPAREN;}
"{"         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "LCURL"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_LCURL;}
"}"         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "RCURL"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_RCURL;}
"["         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "LSQUARE"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_LSQUARE;}
"]"         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "RSQUARE"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_RSQUARE;}
","         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "COMMA"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_COMMA;}
";"         {yylval->symbolInfoPtr = new SymbolInfo(yytext, "SEMICOLON"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_SEMICOLON;}

{assignop}  {yylval->symbolInfoPtr = new SymbolInfo(yytext, "ASSIGNOP"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_ASSIGNOP;}
{not}       {yylval->symbolInfoPtr = new SymbolInfo(yytext, "NOT"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_NOT;}
{incop}     {yylval->symbolInfoPtr = new SymbolInfo(yytext, "INCOP"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_INCOP;}
{decop}     {yylval->symbolInfoPtr = new SymbolInfo(yytext, "DECOP"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_DECOP;}
{addops}    {yylval->symbolInfoPtr = new SymbolInfo(yytext, "ADDOP"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_ADDOP;}
{mulops}    {yylval->symbolInfoPtr = new SymbolInfo(yytext, "MULOP"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_MULOP;}
{relops}    {yylval->symbolInfoPtr = new SymbolInfo(yytext, "RELOP"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_RELOP;}
{logicops}  {yylval->symbolInfoPtr = new SymbolInfo(yytext, "LOGICOP"); writeLog(yylval->symbolInfoPtr, yylineno); return TOKEN_LOGICOP;}


{integer}   {
                token_buf = yytext;
                hasManyDeciamal = false;
                BEGIN(INTEGER);
            }

<INTEGER>{
\.\.+{integer}? {
                token_buf += yytext;
                hasManyDeciamal = true;
                BEGIN(FLOATING_POINT);
            }

[Ee][+-]?   {
                token_buf += yytext;
                BEGIN(EXPONENTIAL_FLOATING_POINT);
            }

\.+[Ee][+-]? {
                token_buf += yytext;
                hasManyDeciamal = true;
                BEGIN(EXPONENTIAL_FLOATING_POINT);
            }

\.?[_a-df-zA-DF-Z](_|{alphaNumeric})*   {
                string error = "INVALID_ID_SUFFIX_NUM_PREFIX ";
                error += token_buf;
                writeErrorLog(yylineno, error);
                BEGIN(INITIAL);
            }
{newLine}   {
                yylval->symbolInfoPtr = new SymbolInfo(token_buf, "CONST_INT");
                writeLog(yylval->symbolInfoPtr, yylineno-1);
                BEGIN(INITIAL);
                return TOKEN_CONST_INT;
            }
.           {
                yyless(0);
                yylval->symbolInfoPtr = new SymbolInfo(token_buf, "CONST_INT");
                writeLog(yylval->symbolInfoPtr, yylineno);
                BEGIN(INITIAL);
                return TOKEN_CONST_INT;
            }
}

{floatingPoint} {
                token_buf = yytext;
                hasManyDeciamal = false;
                BEGIN(FLOATING_POINT);
            }

<FLOATING_POINT>{
[Ee][+-]?   {
                token_buf += yytext;
                BEGIN(EXPONENTIAL_FLOATING_POINT);
            }

(\.+{integer}*)+ {
                token_buf += yytext;
                hasManyDeciamal = true;
            }

\.*{integer}*[_a-df-zA-DF-Z](\.|{alphaNumeric})*    {
                string error = "INVALID_ID_SUFFIX_NUM_PREFIX ";
                error += token_buf;
                writeErrorLog(yylineno, error);
                BEGIN(INITIAL);
            }
{newLine}   {
                if(hasManyDeciamal){
                    string error = "TOO_MANY_DECIMAL_POINTS ";
                    error += token_buf;
                    writeErrorLog(yylineno, error);
                } else {
                    yylval->symbolInfoPtr = new SymbolInfo(token_buf, "CONST_FLOAT");
                    writeLog(yylval->symbolInfoPtr, yylineno-1);
                    BEGIN(INITIAL);
                    return TOKEN_CONST_FLOAT;
                }
            }

.           {
                yyless(0);
                if(hasManyDeciamal){
                    string error = "TOO_MANY_DECIMAL_POINTS ";
                    error += token_buf;
                    writeErrorLog(yylineno, error);
                } else {
                    yylval->symbolInfoPtr = new SymbolInfo(token_buf, "CONST_FLOAT");
                    writeLog(yylval->symbolInfoPtr, yylineno);
                    BEGIN(INITIAL);
                    return TOKEN_CONST_FLOAT;
                }
            }
}

<EXPONENTIAL_FLOATING_POINT>{
{integer}   {
                token_buf += yytext;
                if(hasManyDeciamal){
                    string error = "TOO_MANY_DECIMAL_POINTS ";
                    error += token_buf;
                    writeErrorLog(yylineno, error);
                } else {
                    yylval->symbolInfoPtr = new SymbolInfo(token_buf, "CONST_FLOAT");
                    writeLog(yylval->symbolInfoPtr, yylineno);
                    BEGIN(INITIAL);
                    return TOKEN_CONST_FLOAT;
                }
            }

{integer}?\.?{integer}([Ee][+-]?{integer}?\.?{integer})+ |
{floatingPoint} {
                token_buf += yytext;
                if(hasManyDeciamal){
                    string error = "TOO_MANY_DECIMAL_POINTS ";
                    error += token_buf;
                    writeErrorLog(yylineno, error);
                }else{
                    string error = "ILLFORMED_NUMBER ";
                    error += token_buf;
                    writeErrorLog(yylineno, error);
                }
                BEGIN(INITIAL);
            }

{newLine}   {
                string error = "INVALID_ID_SUFFIX_NUM_PREFIX ";
                error += token_buf;
                writeErrorLog(yylineno, error);
                BEGIN(INITIAL);
            }
{operator}                      |
{punctuator}                    |
{whitespace}                    |
{integer}?{identifier}          |
{floatingPoint}*{identifier}    {
                yyless(0);
                string error = "INVALID_ID_SUFFIX_NUM_PREFIX ";
                error += token_buf;
                writeErrorLog(yylineno, error);
                BEGIN(INITIAL);
            }
<<EOF>>     {
                totalLines = yylineno;
                BEGIN(INITIAL);
                return TOKEN_YYEOF;
            }
.           {
                yyless(0);
                string error = "INVALID_ID_SUFFIX_NUM_PREFIX ";
                error += token_buf;
                writeErrorLog(yylineno, error);
                BEGIN(INITIAL);
            }
}

"//"        {BEGIN(SINGLE_LINE_COMMENT);}
<SINGLE_LINE_COMMENT>{
{newLine}   {
                BEGIN(INITIAL);
            }
<<EOF>>     {
                totalLines = yylineno;
                BEGIN(INITIAL);
                return TOKEN_YYEOF;
            }
\\{newLine} |
.           {}
}

"/*"        {BEGIN(MULTI_LINE_COMMENT);}
<MULTI_LINE_COMMENT>{
"*"+"/"     {BEGIN(INITIAL);}
<<EOF>>     {
                totalLines = yylineno;
                BEGIN(INITIAL);
                return TOKEN_YYEOF;
            }
{newLine}   |
.           {}
}

{identifier} {
                yylval->symbolInfoPtr = new SymbolInfo(yytext, "ID");
                writeLog(yylval->symbolInfoPtr, yylineno); 
                return TOKEN_ID;
            }
{newLine}   |
{whitespace} {}
<<EOF>>     {
                totalLines = yylineno;
                return TOKEN_YYEOF;
            }
.           {
                string error = "UNRECOGNIZED CHAR ";
                error += yytext;
                writeErrorLog(yylineno, error);
            }
%%