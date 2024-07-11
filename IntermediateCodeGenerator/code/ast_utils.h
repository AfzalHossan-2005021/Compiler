#pragma once

#include <iostream>
#include <fstream>
#include "lex_utils.h"

using namespace std;

class ASTNode
{
public:
    virtual string toString()
    {
        return "";
    }
};

class ASTNodeList
{
private:
    ASTNode *nodePtr;

public:
    ASTNodeList *nextNodePtr;
    ASTNodeList(ASTNode *nodePtr)
    {
        this->nodePtr = nodePtr;
        this->nextNodePtr = nullptr;
    }
    ASTNode *getNodePtr()
    {
        return nodePtr;
    }
};

class ASTLeafNode : public ASTNode
{
protected:
    int lineNo;
    SymbolInfo *symbolInfo;

public:
    ASTLeafNode(SymbolInfo *symbolInfo, int lineNo)
    {
        this->symbolInfo = symbolInfo;
        this->lineNo = lineNo;
    }
    void setSymbolInfo(SymbolInfo *symbolInfo)
    {
        this->symbolInfo = symbolInfo;
    }
    SymbolInfo *getSymbolInfo()
    {
        return symbolInfo;
    }
    int getLineNo()
    {
        return lineNo;
    }
    string toString()
    {
        return symbolInfo->getType() + " : " + symbolInfo->getName() + "\t<Line: " + to_string(lineNo) + ">";
    }
};

class ASTInternalNode : public ASTNode
{
protected:
    string rule;
    ASTNodeList *children;
    ASTNodeList *lastChild;
    int fisrtLineNo, lastLineNo;
    bool zero = false;
    bool error = false;
    string typeSpecifier = "UNDEFINED";
    string nextLabel;
    string trueLabel;
    string falseLabel;
    bool isCondition;
    bool isSimpleExpression = false;
    bool isNOTLogical = false;

public:
    ASTInternalNode(string rule, int fisrtLineNo, int lastLineNo)
    {
        this->rule = rule;
        this->fisrtLineNo = fisrtLineNo;
        this->lastLineNo = lastLineNo;
        this->children = nullptr;
        this->lastChild = nullptr;
    }
    void setRule(string rule)
    {
        this->rule = rule;
    }
    string getRule()
    {
        return rule;
    }
    ASTInternalNode *addChild(ASTNode *node)
    {
        if (children == nullptr)
        {
            this->children = new ASTNodeList(node);
            this->lastChild = children;
        }
        else
        {
            this->lastChild->nextNodePtr = new ASTNodeList(node);
            this->lastChild = lastChild->nextNodePtr;
        }
        return this;
    }
    ASTNodeList *getChildren()
    {
        return children;
    }
    void setZero()
    {
        zero = true;
    }
    bool isZero()
    {
        return zero;
    }
    void setError()
    {
        error = true;
    }
    bool isError()
    {
        return error;
    }
    void setTypeSpecifier(string typeSpecifier)
    {
        this->typeSpecifier = typeSpecifier;
    }
    string getTypeSpecifier()
    {
        return typeSpecifier;
    }
    int getFisrtLineNo()
    {
        return fisrtLineNo;
    }
    int getLastLineNo()
    {
        return lastLineNo;
    }
    void setNextLabel(string label)
    {
        nextLabel = label;
    }
    string getNextLabel()
    {
        return nextLabel;
    }
    void setTrueLabel(string label)
    {
        trueLabel = label;
    }
    string getTrueLabel()
    {
        return trueLabel;
    }
    void setFalseLabel(string label)
    {
        falseLabel = label;
    }
    string getFalseLabel()
    {
        return falseLabel;
    }
    void setIsCondition(bool isCondition)
    {
        this->isCondition = isCondition;
    }
    bool getIsCondition()
    {
        return isCondition;
    }
    void setIsSimpleExpression(bool isSimpleExpression)
    {
        this->isSimpleExpression = isSimpleExpression;
    }
    bool getIsSimpleExpression()
    {
        return isSimpleExpression;
    }
    bool getIsNOTLogical()
    {
        return isNOTLogical;
    }
    void setIsNOTLogical(bool isNOTLogical)
    {
        this->isNOTLogical = isNOTLogical;
    }
    string toString()
    {
        if (error)
        {
            return rule + "\t<Line: " + to_string(fisrtLineNo) + ">";
        }
        return rule + " \t<Line: " + to_string(fisrtLineNo) + "-" + to_string(lastLineNo) + ">";
    }
};

class ASTVariableNode : public ASTInternalNode
{
protected:
    VariableInfo *variableInfo;
    int variableOffset;
    int arraryIndex;
    string scopeId;

public:
    ASTVariableNode(string rule, VariableInfo *variableInfo, int fisrtLineNo, int lastLineNo, string scopeId = "") : ASTInternalNode(rule, fisrtLineNo, lastLineNo)
    {
        this->variableInfo = variableInfo;
        this->variableOffset = 0;
        this->scopeId = scopeId;
        this->arraryIndex = -1;
    }
    VariableInfo *getVariableInfo()
    {
        return variableInfo;
    }
    void setArraryIndex(int arraryIndex)
    {
        this->arraryIndex = arraryIndex;
    }
    int getArraryIndex()
    {
        return arraryIndex;
    }
    void setOffset(int offset)
    {
        variableOffset = offset;
    }
    int getOffset()
    {
        return variableOffset;
    }
    void setScopeId(string scopeId)
    {
        this->scopeId = scopeId;
    }
    string getScopeId()
    {
        return scopeId;
    }
};

class ASTFunctionNode : public ASTInternalNode
{
protected:
    FunctionInfo *functionInfo;
public:
    ASTFunctionNode(string rule, FunctionInfo *functionInfo, int fisrtLineNo, int lastLineNo) : ASTInternalNode(rule, fisrtLineNo, lastLineNo)
    {
        this->functionInfo = functionInfo;
    }
    FunctionInfo *getFunctionInfo()
    {
        return functionInfo;
    }
    void setDefined()
    {
        functionInfo->setDefined();
    }
    bool isDefined()
    {
        return functionInfo->isDefined();
    }
};

class AST
{
private:
    ASTNode *root;
    bool isCodeStarted;
    int funcParamCount;
    bool printLibraries;
    int labelCount = 1;
    string returnLabel;
    bool isReturnCalled;
    ofstream asmFile;
    int getDataSize(string typeSpecifier)
    {
        if (typeSpecifier == "INT")
        {
            return 2;
        }
        else if (typeSpecifier == "FLOAT")
        {
            return 4;
        }
        else if (typeSpecifier == "DOUBLE")
        {
            return 8;
        }
        else if (typeSpecifier == "CHAR")
        {
            return 1;
        }
        return 0;
    }
    ASTNodeList *getChild(ASTInternalNode *internalNode, int position)
    {
        ASTNodeList *childPtr = internalNode->getChildren();
        for (int i = 1; i < position; i++)
        {
            if (childPtr == nullptr)
                break;
            childPtr = childPtr->nextNodePtr;
        }
        return childPtr;
    }

    string gen_newline()
    {
        string newline_func = "\n\
new_line PROC\n\
    PUSH AX\n\
    PUSH DX\n\
    MOV DL, 0AH\n\
    MOV AH, 2\n\
    INT 21H\n\
    MOV DL, 0DH\n\
    MOV AH, 2\n\
    INT 21H\n\
    POP DX\n\
    POP AX\n\
    RET\n\
new_line ENDP\n";
        return newline_func;
    }

    string gen_println()
    {
        string println_func = "\
print_output PROC\t;print what is in ax\n\
    PUSH AX\n\
    PUSH BX\n\
    PUSH CX\n\
    PUSH DX\n\
    PUSH SI\n\
    LEA SI, number\n\
    MOV BX, 10\n\
    ADD SI, 4\n\
    CMP AX, 0\n\
    JNGE NEGATE\n\
    PRINT:\n\
    XOR DX, DX\n\
    DIV BX\n\
    MOV [SI], DL\n\
    ADD [SI], '0'\n\
    DEC SI\n\
    CMP AX, 0\n\
    JNE PRINT\n\
    INC SI\n\
    LEA DX, SI\n\
    MOV AH, 9\n\
    INT 21H\n\
    CALL new_line\n\
    POP SI\n\
    POP DX\n\
    POP CX\n\
    POP BX\n\
    POP AX\n\
    RET\n\
    NEGATE:\n\
    PUSH AX\n\
    MOV AH, 2\n\
    MOV DL, '-'\n\
    INT 21H\n\
    POP AX\n\
    NEG AX\n\
    JMP PRINT\n\
print_output ENDP\n";
        return println_func;
    }

    void generateStartingCode()
    {
        string starting_code = "\
.STACK 1000H\n\
.MODEL SMALL\n\
.Data\n\
    number DB '00000$'\n";
        asmFile << starting_code;
    }
    void generateEndingCode()
    {
        if (printLibraries == true)
        {
            asmFile << gen_newline() << endl;
            asmFile << gen_println() << endl;
        }
        asmFile << "END main\n";
    }
    void printASM(string _asm)
    {
        asmFile << _asm;
    }
    void genGlobalVar(string _var_name, int _var_size = 1)
    {
        string var_declaration = "\t" + _var_name + " DW " + to_string(_var_size) + " DUP (0000H)\n";
        asmFile << var_declaration;
    }
    void genPROC(string _proc_name)
    {
        string proc_instruction = _proc_name + " PROC\n";
        asmFile << proc_instruction;
    }

    void genENDP(string _proc_name)
    {
        string endp_instruction = _proc_name + " ENDP\n";
        asmFile << endp_instruction;
    }

    void genMOV(string _reg1, string _reg2, int _lineno = 0)
    {
        if (_lineno == 0)
        {
            string mov_instruction = "\tMOV " + _reg1 + ", " + _reg2 + "\n";
            asmFile << mov_instruction;
        }
        else
        {
            string mov_instruction = "\tMOV " + _reg1 + ", " + _reg2 + "       ; Line " + to_string(_lineno) + "\n";
            asmFile << mov_instruction;
        }
    }

    void genSUB(string _reg1, string _reg2)
    {
        string sub_instruction = "\tSUB " + _reg1 + ", " + _reg2 + "\n";
        asmFile << sub_instruction;
    }

    void genADD(string _reg1, string _reg2)
    {
        string add_instruction = "\tADD " + _reg1 + ", " + _reg2 + "\n";
        asmFile << add_instruction;
    }

    void genNOT(string _reg)
    {
        string not_instruction = "\tNOT " + _reg + "\n";
        asmFile << not_instruction;
    }

    void genNEG(string _reg)
    {
        string neg_instruction = "\tNEG " + _reg + "\n";
        asmFile << neg_instruction;
    }

    void genPUSH(string _reg)
    {
        string push_instruction = "\tPUSH " + _reg + "\n";
        asmFile << push_instruction;
    }

    void genPOP(string _reg, int _lineno = 0)
    {
        if (_lineno == 0)
        {
            string pop_instruction = "\tPOP " + _reg + "\n";
            asmFile << pop_instruction;
        }
        else
        {
            string pop_instruction = "\tPOP " + _reg + "       ; Line " + to_string(_lineno) + "\n";
            asmFile << pop_instruction;
        }
    }

    void genINT(string _int_no)
    {
        string int_instruction = "\tINT " + _int_no + "\n";
        asmFile << int_instruction;
    }
    void genCALL(string _proc_name)
    {
        string call_instruction = "\tCALL " + _proc_name + "\n";
        asmFile << call_instruction;
    }

    void genCWD()
    {
        string cwd_instruction = "\tCWD\n";
        asmFile << cwd_instruction;
    }

    void genMUL(string _reg)
    {
        string mul_instruction = "\tMUL " + _reg + "\n";
        asmFile << mul_instruction;
    }

    void genDIV(string _reg)
    {
        string div_instruction = "\tDIV " + _reg + "\n";
        asmFile << div_instruction;
    }

    void genINC(string _reg)
    {
        string inc_instruction = "\tINC " + _reg + "\n";
        asmFile << inc_instruction;
    }

    void genDEC(string _reg)
    {
        string dec_instruction = "\tDEC " + _reg + "\n";
        asmFile << dec_instruction;
    }

    void genJL(string _label)
    {
        string jl_instruction = "\tJL " + _label + "\n";
        asmFile << jl_instruction;
    }

    void genJLE(string _label)
    {
        string jle_instruction = "\tJLE " + _label + "\n";
        asmFile << jle_instruction;
    }

    void genJG(string _label)
    {
        string jg_instruction = "\tJG " + _label + "\n";
        asmFile << jg_instruction;
    }

    void genJGE(string _label)
    {
        string jge_instruction = "\tJGE " + _label + "\n";
        asmFile << jge_instruction;
    }

    void genJE(string _label)
    {
        string je_instruction = "\tJE " + _label + "\n";
        asmFile << je_instruction;
    }

    void genJNE(string _label)
    {
        string jne_instruction = "\tJNE " + _label + "\n";
        asmFile << jne_instruction;
    }

    void genJMP(string _label)
    {
        string jmp_instruction = "\tJMP " + _label + "\n";
        asmFile << jmp_instruction;
    }

    void genCMP(string _reg1, string _reg2)
    {
        string cmp_instruction = "\tCMP " + _reg1 + ", " + _reg2 + "\n";
        asmFile << cmp_instruction;
    }

    void genRET(int popCount = 0)
    {
        if (popCount > 0)
        {
            string ret_instruction = "\tRET " + to_string(popCount) + "\n";
            asmFile << ret_instruction;
        }
        else
        {
            string ret_instruction = "\tRET\n";
            asmFile << ret_instruction;
        }
    }
    string genLabel()
    {
        string label = "L" + to_string(labelCount);
        labelCount++;
        return label;
    }
    void printLabel(string label)
    {
        asmFile << label << ":" << endl;
    }
    string get_lineno_comment(int _lineno)
    {
        return "       ; Line " + to_string(_lineno);
    }

    string get_lineno_comment(int _start_lineno, int _end_lineno)
    {
        return "\tline no: " + to_string(_start_lineno) + "-" + to_string(_end_lineno);
    }

public:
    AST()
    {
        root = nullptr;
        isCodeStarted = false;
        printLibraries = false;
    }
    void setRoot(ASTNode *root)
    {
        this->root = root;
    }
    ASTNode *getRoot()
    {
        return root;
    }
    void printTree(ofstream &file)
    {
        printTree(file, root, 0);
    }
    void printTree(ofstream &file, ASTNode *node, int depth)
    {
        if (node == nullptr)
        {
            return;
        }
        for (int i = 0; i < depth; i++)
        {
            file << " ";
        }
        file << node->toString() << endl;
        ASTInternalNode *internalNode = dynamic_cast<ASTInternalNode *>(node);
        if (internalNode != nullptr)
        {
            ASTNodeList *children = internalNode->getChildren();
            while (children != nullptr)
            {
                printTree(file, children->getNodePtr(), depth + 1);
                children = children->nextNodePtr;
            }
        }
    }
    void generateIntermediateCode(string fileName, SymbolTable *table)
    {
        asmFile.open(fileName);
        generateStartingCode();
        genetateGlobalVariables(table);
        generateIntermediateCode(root);
        generateEndingCode();
        asmFile.close();
    }
    void genetateGlobalVariables(SymbolTable *table)
    {
        SymbolInfo **hashTable = table->getCurrentScopeTableHashTable();
        int size = table->getTotalBuckets();
        for (int i = 0; i < size; i++)
        {
            SymbolInfo *symbolInfo = hashTable[i];
            while (symbolInfo != nullptr)
            {
                if (symbolInfo->getType() == "VARIABLE")
                {
                    VariableInfo *variableInfo = dynamic_cast<VariableInfo *>(symbolInfo);
                    if (variableInfo != nullptr)
                    {
                        if (variableInfo->getScopeId() == "1")
                        {
                            if (variableInfo->getTypeSpecifier() == "INT")
                            {
                                genGlobalVar(variableInfo->getName());
                            }
                        }
                    }
                }
                else if (symbolInfo->getType() == "ARRAY")
                {
                    ArrayInfo *arrayInfo = dynamic_cast<ArrayInfo *>(symbolInfo);
                    if (arrayInfo != nullptr)
                    {
                        if (arrayInfo->getScopeId() == "1" && arrayInfo->getArraySize() > 0)
                        {
                            if (arrayInfo->getTypeSpecifier() == "INT")
                            {
                                genGlobalVar(arrayInfo->getName(), arrayInfo->getArraySize());
                            }
                        }
                    }
                }
                symbolInfo = symbolInfo->nestSymbolInfo;
            }
        }
    }
    void generateIntermediateCode(ASTNode *node)
    {
        if (node != nullptr)
        {
            ASTInternalNode *internalNode = dynamic_cast<ASTInternalNode *>(node);
            if (internalNode != nullptr)
            {
                string rule = internalNode->getRule();

                if (rule == "func_definition : type_specifier ID LPAREN RPAREN compound_statement")
                {
                    ASTFunctionNode *functionNode = dynamic_cast<ASTFunctionNode *>(internalNode);
                    if (functionNode != nullptr)
                    {
                        isReturnCalled = false;
                        if (!isCodeStarted)
                        {
                            asmFile << ".CODE" << endl;
                            isCodeStarted = true;
                            labelCount = 1;
                        }
                        string funcName = functionNode->getFunctionInfo()->getName();
                        int funcStackOffset = functionNode->getFunctionInfo()->getFuncStackOffset();
                        funcParamCount = functionNode->getFunctionInfo()->getParameterCount();
                        genPROC(funcName);
                        if (funcName == "main")
                        {
                            genMOV("AX", "@DATA");
                            genMOV("DS", "AX");
                        }
                        genPUSH("BP");
                        genMOV("BP", "SP");
                        genSUB("SP", to_string(funcStackOffset));

                        ASTNodeList *child = getChild(internalNode, 5);
                        ASTInternalNode *compoundStatementNode = dynamic_cast<ASTInternalNode *>(child->getNodePtr());
                        if (compoundStatementNode != nullptr)
                        {
                            generateIntermediateCode(compoundStatementNode);
                        }
                        if (isReturnCalled)
                        {
                            printLabel(returnLabel);
                        }
                        if (funcStackOffset > 0)
                        {
                            genADD("SP", to_string(funcStackOffset));
                        }
                        genPOP("BP");
                        if (funcName == "main")
                        {
                            genMOV("AH", "4CH");
                            genINT("21H");
                        }
                        else
                        {
                            genRET(funcParamCount * 2);
                        }
                        genENDP(funcName);
                    }
                }
                else if (rule == "func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement")
                {
                    ASTFunctionNode *functionNode = dynamic_cast<ASTFunctionNode *>(internalNode);
                    if (functionNode != nullptr)
                    {
                        isReturnCalled = false;
                        if (!isCodeStarted)
                        {
                            asmFile << ".CODE" << endl;
                            isCodeStarted = true;
                        }
                        string funcName = functionNode->getFunctionInfo()->getName();
                        int funcStackOffset = functionNode->getFunctionInfo()->getFuncStackOffset();
                        funcParamCount = functionNode->getFunctionInfo()->getParameterCount();
                        genPROC(funcName);
                        genPUSH("BP");
                        genMOV("BP", "SP");
                        genSUB("SP", to_string(funcStackOffset));
                        ASTNodeList *child = getChild(internalNode, 6);
                        ASTInternalNode *compoundStatementNode = dynamic_cast<ASTInternalNode *>(child->getNodePtr());
                        if (compoundStatementNode != nullptr)
                        {
                            generateIntermediateCode(compoundStatementNode);
                        }
                        if (isReturnCalled)
                        {
                            printLabel(returnLabel);
                        }
                        if (funcStackOffset > 0)
                        {
                            genADD("SP", to_string(funcStackOffset));
                        }
                        genPOP("BP");
                        genRET(funcParamCount * 2);
                        genENDP(funcName);
                    }
                }
                else if (rule == "compound_statement : LCURL statements RCURL")
                {
                    ASTInternalNode *statementListNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 2)->getNodePtr());
                    if (statementListNode != nullptr)
                    {
                        statementListNode->setNextLabel(internalNode->getNextLabel());
                        generateIntermediateCode(statementListNode);
                    }
                }

                else if (rule == "statements : statements statement")
                {
                    ASTInternalNode *statementListNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    ASTInternalNode *statementNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 2)->getNodePtr());
                    if (statementListNode != nullptr && statementNode != nullptr)
                    {
                        generateIntermediateCode(statementListNode);
                        string label = genLabel();
                        statementNode->setNextLabel(label);
                        generateIntermediateCode(statementNode);
                        printLabel(statementNode->getNextLabel());
                    }
                }
                else if (rule == "statements : statement")
                {
                    ASTInternalNode *statementNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (statementNode != nullptr)
                    {
                        string label = genLabel();
                        statementNode->setNextLabel(label);
                        generateIntermediateCode(statementNode);
                        printLabel(statementNode->getNextLabel());
                    }
                }
                else if (rule == "statement : expression_statement")
                {
                    ASTInternalNode *expressionStatementNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (expressionStatementNode != nullptr)
                    {
                        string nextLabel = genLabel();
                        expressionStatementNode->setNextLabel(nextLabel);
                        expressionStatementNode->setIsCondition(false);
                        generateIntermediateCode(expressionStatementNode);
                        printLabel(expressionStatementNode->getNextLabel());
                        genPOP("AX");
                    }
                }
                else if (rule == "statement : compound_statement")
                {
                    ASTInternalNode *compoundStatementNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (compoundStatementNode != nullptr)
                    {
                        compoundStatementNode->setNextLabel(internalNode->getNextLabel());
                        generateIntermediateCode(compoundStatementNode);
                    }
                }
                else if (rule == "statement : var_declaration")
                {
                    ASTInternalNode *varDeclarationNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (varDeclarationNode != nullptr)
                    {
                        varDeclarationNode->setNextLabel(internalNode->getNextLabel());
                        generateIntermediateCode(varDeclarationNode);
                    }
                }
                else if (rule == "statement : FOR LPAREN expression_statement expression_statement expression RPAREN statement")
                {
                    ASTInternalNode *expressionStatementNode1 = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    ASTInternalNode *expressionStatementNode2 = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 4)->getNodePtr());
                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 5)->getNodePtr());
                    ASTInternalNode *statementNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 7)->getNodePtr());
                    if (expressionStatementNode1 != nullptr && expressionStatementNode2 != nullptr && expressionNode != nullptr && statementNode != nullptr)
                    {
                        string begin = genLabel();
                        string label1 = genLabel();
                        string label2 = genLabel();
                        expressionStatementNode1->setNextLabel(begin);
                        expressionStatementNode2->setTrueLabel(label2);
                        expressionStatementNode2->setFalseLabel(internalNode->getNextLabel());
                        expressionNode->setNextLabel(begin);
                        statementNode->setNextLabel(label1);

                        expressionStatementNode1->setIsCondition(false);
                        expressionStatementNode2->setIsCondition(true);
                        expressionNode->setIsCondition(false);

                        generateIntermediateCode(expressionStatementNode1);
                        genPOP("AX");
                        printLabel(begin);
                        generateIntermediateCode(expressionStatementNode2);
                        printLabel(label1);
                        generateIntermediateCode(expressionNode);
                        genPOP("AX");
                        genJMP(begin);
                        printLabel(label2);
                        generateIntermediateCode(statementNode);
                        genJMP(label1);
                    }
                }
                else if (rule == "statement : IF LPAREN expression RPAREN statement")
                {
                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    ASTInternalNode *statementNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 5)->getNodePtr());
                    if (expressionNode != nullptr && statementNode != nullptr)
                    {
                        string label1 = genLabel();
                        expressionNode->setTrueLabel(label1);
                        statementNode->setNextLabel(internalNode->getNextLabel());
                        expressionNode->setFalseLabel(statementNode->getNextLabel());
                        expressionNode->setIsCondition(true);

                        generateIntermediateCode(expressionNode);
                        printLabel(expressionNode->getTrueLabel());
                        generateIntermediateCode(statementNode);
                    }
                }
                else if (rule == "statement : IF LPAREN expression RPAREN statement ELSE statement")
                {
                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    ASTInternalNode *statementNode1 = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 5)->getNodePtr());
                    ASTInternalNode *statementNode2 = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 7)->getNodePtr());
                    if (expressionNode != nullptr && statementNode1 != nullptr && statementNode2 != nullptr)
                    {
                        string label1 = genLabel();
                        string label2 = genLabel();

                        expressionNode->setTrueLabel(label1);
                        expressionNode->setFalseLabel(label2);
                        statementNode1->setNextLabel(internalNode->getNextLabel());
                        statementNode2->setNextLabel(internalNode->getNextLabel());
                        expressionNode->setIsCondition(true);

                        generateIntermediateCode(expressionNode);
                        printLabel(expressionNode->getTrueLabel());
                        generateIntermediateCode(statementNode1);
                        genJMP(internalNode->getNextLabel());
                        printLabel(expressionNode->getFalseLabel());
                        generateIntermediateCode(statementNode2);
                    }
                }
                else if (rule == "statement : WHILE LPAREN expression RPAREN statement")
                {
                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    ASTInternalNode *statementNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 5)->getNodePtr());
                    if (expressionNode != nullptr && statementNode != nullptr)
                    {
                        string begin = genLabel();
                        string label1 = genLabel();

                        expressionNode->setTrueLabel(label1);
                        expressionNode->setFalseLabel(internalNode->getNextLabel());
                        expressionNode->setNextLabel(internalNode->getNextLabel());
                        statementNode->setNextLabel(begin);
                        expressionNode->setIsCondition(true);

                        printLabel(begin);
                        generateIntermediateCode(expressionNode);
                        printLabel(expressionNode->getTrueLabel());
                        generateIntermediateCode(statementNode);
                        genJMP(begin);
                    }
                }
                else if (rule == "statement : RETURN expression SEMICOLON")
                {
                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 2)->getNodePtr());
                    if (expressionNode != nullptr)
                    {
                        if (!isReturnCalled)
                        {
                            isReturnCalled = true;
                            returnLabel = genLabel();
                        }
                        expressionNode->setNextLabel(internalNode->getNextLabel());
                        expressionNode->setIsCondition(false);
                        generateIntermediateCode(expressionNode);
                        genPOP("AX");
                        genJMP(returnLabel);
                    }
                }

                else if (rule == "statement : PRINTLN LPAREN ID RPAREN SEMICOLON")
                {
                    ASTLeafNode *idNode = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 3)->getNodePtr());
                    if (idNode != nullptr)
                    {
                        SymbolInfo *symbolInfo = idNode->getSymbolInfo();
                        if (symbolInfo != nullptr)
                        {
                            if (symbolInfo->getType() == "VARIABLE")
                            {
                                VariableInfo *variableInfo = dynamic_cast<VariableInfo *>(symbolInfo);
                                if (variableInfo != nullptr)
                                {
                                    if (variableInfo->getScopeId() == "1")
                                    {
                                        genMOV("AX", variableInfo->getName(), internalNode->getLastLineNo());
                                    }
                                    else if (variableInfo->getOffset() > 0)
                                    {
                                        genMOV("AX", "[BP-" + to_string(variableInfo->getOffset()) + "]", internalNode->getLastLineNo());
                                    }
                                    else if (variableInfo->getParamOffset() > 0)
                                    {
                                        genMOV("AX", "[BP+" + to_string(funcParamCount * 2 - variableInfo->paramOffset + 4) + "]", internalNode->getLastLineNo());
                                    }
                                    genCALL("print_output");
                                    genCALL("new_line");
                                }
                            }
                        }
                    }
                    if (printLibraries == false)
                    {
                        printLibraries = true;
                    }
                }
                else if (rule == "expression_statement : expression SEMICOLON")
                {
                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (expressionNode != nullptr)
                    {
                        expressionNode->setTrueLabel(internalNode->getTrueLabel());
                        expressionNode->setFalseLabel(internalNode->getFalseLabel());
                        expressionNode->setNextLabel(internalNode->getNextLabel());
                        expressionNode->setIsCondition(internalNode->getIsCondition());
                        generateIntermediateCode(expressionNode);
                    }
                }

                else if (rule == "variable : ID")
                {
                    ASTLeafNode *idNode = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (idNode != nullptr)
                    {
                        SymbolInfo *symbolInfo = idNode->getSymbolInfo();
                        if (symbolInfo != nullptr)
                        {
                            if (symbolInfo->getType() == "VARIABLE")
                            {
                                VariableInfo *variableInfo = dynamic_cast<VariableInfo *>(symbolInfo);
                                if (variableInfo != nullptr)
                                {
                                    if (variableInfo->getScopeId() == "1")
                                    {
                                        genMOV("AX", variableInfo->getName(), internalNode->getLastLineNo());
                                    }
                                    else if (variableInfo->getOffset() > 0)
                                    {
                                        genMOV("AX", "[BP-" + to_string(variableInfo->getOffset()) + "]", internalNode->getLastLineNo());
                                    }
                                    else if (variableInfo->getParamOffset() > 0)
                                    {
                                        genMOV("AX", "[BP+" + to_string(funcParamCount * 2 - variableInfo->paramOffset + 4) + "]", internalNode->getLastLineNo());
                                    }
                                    genPUSH("AX");
                                }
                            }
                        }
                    }
                }
                else if (rule == "variable : ID LSQUARE expression RSQUARE")
                {
                    ASTLeafNode *idNode = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 1)->getNodePtr());
                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    if (idNode != nullptr && expressionNode != nullptr)
                    {
                        expressionNode->setIsCondition(false);
                        SymbolInfo *symbolInfo = idNode->getSymbolInfo();
                        if (symbolInfo != nullptr)
                        {
                            if (symbolInfo->getType() == "ARRAY")
                            {
                                ArrayInfo *arrayInfo = dynamic_cast<ArrayInfo *>(symbolInfo);
                                if (arrayInfo != nullptr)
                                {
                                    generateIntermediateCode(expressionNode);
                                    genPOP("BX");
                                    genMOV("AX", "2");
                                    genMUL("BX");
                                    genMOV("BX", "AX");

                                    if (arrayInfo->getScopeId() == "1")
                                    {
                                        genMOV("AX", arrayInfo->getName() + "[BX]");
                                    }
                                    else if (arrayInfo->getOffset() > 0)
                                    {
                                        genMOV("AX", to_string(2 * arrayInfo->getArraySize()));
                                        genSUB("AX", "BX");
                                        genMOV("BX", "AX");
                                        genMOV("SI", "BX");
                                        genNEG("SI");
                                        genMOV("AX", "[BP+SI]");
                                    }
                                    genPUSH("AX");
                                }
                            }
                        }
                    }
                }
                else if (rule == "expression : logic_expression")
                {
                    ASTInternalNode *logicExpressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (logicExpressionNode != nullptr)
                    {
                        if(internalNode->getIsCondition()){
                            logicExpressionNode->setIsCondition(internalNode->getIsCondition());
                            logicExpressionNode->setTrueLabel(internalNode->getTrueLabel());
                            logicExpressionNode->setFalseLabel(internalNode->getFalseLabel());
                            logicExpressionNode->setNextLabel(internalNode->getNextLabel());
                            generateIntermediateCode(logicExpressionNode);
                        }
                        else
                        {
                            string trueLabel = genLabel();
                            string falseLabel = genLabel();
                            logicExpressionNode->setTrueLabel(trueLabel);
                            logicExpressionNode->setFalseLabel(falseLabel);
                            logicExpressionNode->setNextLabel(internalNode->getNextLabel());
                            logicExpressionNode->setIsCondition(false);
                            generateIntermediateCode(logicExpressionNode);
                            if(!logicExpressionNode->getIsSimpleExpression()){
                                printLabel(logicExpressionNode->getTrueLabel());
                                genMOV("AX", "1");
                                genPUSH("AX");
                                genJMP(logicExpressionNode->getNextLabel());
                                printLabel(logicExpressionNode->getFalseLabel());
                                genMOV("AX", "0");
                                genPUSH("AX");
                                genJMP(logicExpressionNode->getNextLabel());
                            }
                        }
                    }
                }

                else if (rule == "expression : variable ASSIGNOP logic_expression")
                {
                    ASTVariableNode *variableNode = dynamic_cast<ASTVariableNode *>((getChild(internalNode, 1))->getNodePtr());
                    ASTInternalNode *logicExpressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    if (variableNode != nullptr && logicExpressionNode != nullptr)
                    {
                        string nextLabel = genLabel();
                        string trueLabel = genLabel();
                        string falseLabel = genLabel();
                        logicExpressionNode->setTrueLabel(trueLabel);
                        logicExpressionNode->setFalseLabel(falseLabel);
                        logicExpressionNode->setNextLabel(nextLabel);
                        logicExpressionNode->setIsCondition(false);
                        generateIntermediateCode(logicExpressionNode);
                        if(!logicExpressionNode->getIsSimpleExpression()){
                            printLabel(logicExpressionNode->getTrueLabel());
                            genMOV("AX", "1");
                            genPUSH("AX");
                            genJMP(logicExpressionNode->getNextLabel());
                            printLabel(logicExpressionNode->getFalseLabel());
                            genMOV("AX", "0");
                            genPUSH("AX");
                            genJMP(logicExpressionNode->getNextLabel());
                        }
                        printLabel(logicExpressionNode->getNextLabel());
                        VariableInfo *variableInfo = variableNode->getVariableInfo();
                        if (variableInfo != nullptr)
                        {
                            if (variableInfo->getType() == "VARIABLE")
                            {
                                genPOP("AX");
                                if (variableInfo->getScopeId() == "1")
                                {
                                    genMOV(variableInfo->getName(), "AX", internalNode->getLastLineNo());
                                }
                                else if (variableInfo->getOffset() > 0)
                                {
                                    genMOV("[BP-" + to_string(variableInfo->getOffset()) + "]", "AX", internalNode->getLastLineNo());
                                }
                                else if (variableInfo->getParamOffset() > 0)
                                {
                                    genMOV("[BP+" + to_string(funcParamCount * 2 - variableInfo->paramOffset + 4) + "]", "AX", internalNode->getLastLineNo());
                                }
                            }
                            else if (variableInfo->getType() == "ARRAY")
                            {
                                ArrayInfo *arrayInfo = dynamic_cast<ArrayInfo *>(variableInfo);
                                if (arrayInfo != nullptr)
                                {
                                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(variableNode, 3)->getNodePtr());
                                    expressionNode->setIsCondition(false);
                                    generateIntermediateCode(expressionNode);
                                    genPOP("BX");
                                    genMOV("AX", "2");
                                    genMUL("BX");
                                    genMOV("BX", "AX");

                                    if (arrayInfo->getScopeId() == "1")
                                    {
                                        genPOP("AX");
                                        genMOV(arrayInfo->getName() + "[BX]", "AX");
                                    }
                                    else if (arrayInfo->getOffset() > 0)
                                    {
                                        genMOV("AX", to_string(2 * arrayInfo->getArraySize()));
                                        genSUB("AX", "BX");
                                        genMOV("BX", "AX");
                                        genPOP("AX");
                                        genMOV("SI", "BX");
                                        genNEG("SI");
                                        genMOV("[BP+SI]", "AX");
                                    }
                                }
                            }
                            if (internalNode->getIsCondition())
                            {
                                genPOP("AX");
                                genCMP("AX", "0");
                                genJNE(internalNode->getTrueLabel());
                                genJMP(internalNode->getFalseLabel());
                            }
                            else
                            {
                                genPUSH("AX");
                            }
                        }
                    }
                }
                else if (rule == "logic_expression : rel_expression LOGICOP rel_expression")
                {
                    ASTInternalNode *childRelExpressionNode1 = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    ASTInternalNode *childRelExpressionNode2 = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    string logicOp = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 2)->getNodePtr())->getSymbolInfo()->getName();
                    if (childRelExpressionNode1 != nullptr && childRelExpressionNode2 != nullptr)
                    {
                        childRelExpressionNode1->setIsCondition(true);
                        childRelExpressionNode2->setIsCondition(true);
                        childRelExpressionNode1->setNextLabel(internalNode->getNextLabel());
                        childRelExpressionNode2->setNextLabel(internalNode->getNextLabel());
                        string label1 = genLabel();
                        if (logicOp == "&&")
                        {
                            childRelExpressionNode1->setTrueLabel(label1);
                            childRelExpressionNode1->setFalseLabel(internalNode->getFalseLabel());
                            childRelExpressionNode2->setTrueLabel(internalNode->getTrueLabel());
                            childRelExpressionNode2->setFalseLabel(internalNode->getFalseLabel());
                            generateIntermediateCode(childRelExpressionNode1);
                            printLabel(childRelExpressionNode1->getTrueLabel());
                            generateIntermediateCode(childRelExpressionNode2);
                        }
                        else if (logicOp == "||")
                        {
                            childRelExpressionNode1->setTrueLabel(internalNode->getTrueLabel());
                            childRelExpressionNode1->setFalseLabel(label1);
                            childRelExpressionNode2->setTrueLabel(internalNode->getTrueLabel());
                            childRelExpressionNode2->setFalseLabel(internalNode->getFalseLabel());
                            generateIntermediateCode(childRelExpressionNode1);
                            printLabel(childRelExpressionNode1->getFalseLabel());
                            generateIntermediateCode(childRelExpressionNode2);
                        }
                    }
                }
                else if (rule == "logic_expression : rel_expression")
                {
                    ASTInternalNode *childRelExpressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (childRelExpressionNode != nullptr)
                    {
                        childRelExpressionNode->setIsCondition(internalNode->getIsCondition());
                        childRelExpressionNode->setNextLabel(internalNode->getNextLabel());
                        childRelExpressionNode->setTrueLabel(internalNode->getTrueLabel());
                        childRelExpressionNode->setFalseLabel(internalNode->getFalseLabel());
                        generateIntermediateCode(childRelExpressionNode);
                        internalNode->setIsSimpleExpression(childRelExpressionNode->getIsSimpleExpression());
                        internalNode->setIsNOTLogical(childRelExpressionNode->getIsNOTLogical());
                    }
                }

                else if (rule == "rel_expression : simple_expression RELOP simple_expression")
                {
                    ASTInternalNode *childSimpleExpressionNode1 = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    ASTInternalNode *childSimpleExpressionNode2 = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    string relOp = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 2)->getNodePtr())->getSymbolInfo()->getName();
                    if (childSimpleExpressionNode1 != nullptr && childSimpleExpressionNode2 != nullptr)
                    {
                        string label1 = genLabel();
                        string label2 = genLabel();
                        childSimpleExpressionNode1->setTrueLabel(internalNode->getTrueLabel());
                        childSimpleExpressionNode1->setFalseLabel(internalNode->getFalseLabel());
                        childSimpleExpressionNode1->setNextLabel(label1);
                        childSimpleExpressionNode1->setIsCondition(internalNode->getIsCondition());

                        childSimpleExpressionNode2->setTrueLabel(internalNode->getTrueLabel());
                        childSimpleExpressionNode2->setFalseLabel(internalNode->getFalseLabel());
                        childSimpleExpressionNode2->setNextLabel(label2);
                        childSimpleExpressionNode2->setIsCondition(internalNode->getIsCondition());

                        generateIntermediateCode(childSimpleExpressionNode1);
                        printLabel(childSimpleExpressionNode1->getNextLabel());
                        generateIntermediateCode(childSimpleExpressionNode2);
                        printLabel(childSimpleExpressionNode2->getNextLabel());

                        genPOP("DX", internalNode->getLastLineNo());
                        genPOP("AX", internalNode->getLastLineNo());
                        genCMP("AX", "DX");
                        if (relOp == "<")
                        {
                            genJL(internalNode->getTrueLabel());
                            genJMP(internalNode->getFalseLabel());
                        }
                        else if (relOp == "<=")
                        {
                            genJLE(internalNode->getTrueLabel());
                            genJMP(internalNode->getFalseLabel());
                        }
                        else if (relOp == ">")
                        {
                            genJG(internalNode->getTrueLabel());
                            genJMP(internalNode->getFalseLabel());
                        }
                        else if (relOp == ">=")
                        {
                            genJGE(internalNode->getTrueLabel());
                            genJMP(internalNode->getFalseLabel());
                        }
                        else if (relOp == "==")
                        {
                            genJE(internalNode->getTrueLabel());
                            genJMP(internalNode->getFalseLabel());
                        }
                        else if (relOp == "!=")
                        {
                            genJNE(internalNode->getTrueLabel());
                            genJMP(internalNode->getFalseLabel());
                        }
                    }
                }
                else if (rule == "rel_expression : simple_expression")
                {
                    ASTInternalNode *childSimpleExpressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (childSimpleExpressionNode != nullptr)
                    {
                        childSimpleExpressionNode->setIsCondition(internalNode->getIsCondition());
                        childSimpleExpressionNode->setTrueLabel(internalNode->getTrueLabel());
                        childSimpleExpressionNode->setFalseLabel(internalNode->getFalseLabel());
                        childSimpleExpressionNode->setNextLabel(internalNode->getNextLabel());
                        generateIntermediateCode(childSimpleExpressionNode);
                        if(internalNode->getIsCondition()){
                            genPOP("AX", internalNode->getLastLineNo());
                            genCMP("AX", "0");
                            if(childSimpleExpressionNode->getIsNOTLogical()){
                                genJE(internalNode->getTrueLabel());
                                genJMP(internalNode->getFalseLabel());
                            }
                            else{
                                genJNE(internalNode->getTrueLabel());
                                genJMP(internalNode->getFalseLabel());
                            }
                        }
                        internalNode->setIsSimpleExpression(true);
                        internalNode->setIsNOTLogical(childSimpleExpressionNode->getIsNOTLogical());
                    }
                }

                else if (rule == "simple_expression : simple_expression ADDOP term")
                {
                    ASTInternalNode *childSimpleExpressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    ASTInternalNode *childTermNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    string addOp = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 2)->getNodePtr())->getSymbolInfo()->getName();
                    if (childSimpleExpressionNode != nullptr && childTermNode != nullptr)
                    {
                        string simpleExpressionNextLabel = genLabel();
                        childSimpleExpressionNode->setIsCondition(false);
                        childSimpleExpressionNode->setNextLabel(simpleExpressionNextLabel);

                        string termNextLabel = genLabel();
                        childTermNode->setIsCondition(false);
                        childTermNode->setNextLabel(termNextLabel);

                        generateIntermediateCode(childSimpleExpressionNode);
                        printLabel(childSimpleExpressionNode->getNextLabel());
                        generateIntermediateCode(childTermNode);
                        printLabel(childTermNode->getNextLabel());

                        genPOP("DX", internalNode->getLastLineNo());
                        genPOP("AX", internalNode->getLastLineNo());
                        if (addOp == "+"){
                            genADD("AX", "DX");
                        }
                        else if (addOp == "-"){
                            genSUB("AX", "DX");
                        }
                        genPUSH("AX");
                    }
                }
                else if (rule == "simple_expression : term")
                {
                    ASTInternalNode *childTermNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (childTermNode != nullptr)
                    {
                        childTermNode->setIsCondition(internalNode->getIsCondition());
                        childTermNode->setTrueLabel(internalNode->getTrueLabel());
                        childTermNode->setFalseLabel(internalNode->getFalseLabel());
                        childTermNode->setNextLabel(internalNode->getNextLabel());
                        generateIntermediateCode(childTermNode);
                        internalNode->setIsNOTLogical(childTermNode->getIsNOTLogical());
                    }
                }
                else if (rule == "term : unary_expression")
                {
                    ASTInternalNode *unary_expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (unary_expressionNode != nullptr)
                    {
                        unary_expressionNode->setIsCondition(internalNode->getIsCondition());
                        unary_expressionNode->setTrueLabel(internalNode->getTrueLabel());
                        unary_expressionNode->setFalseLabel(internalNode->getFalseLabel());
                        unary_expressionNode->setNextLabel(internalNode->getNextLabel());
                        generateIntermediateCode(unary_expressionNode);
                        internalNode->setIsNOTLogical(unary_expressionNode->getIsNOTLogical());
                    }
                }

                else if (rule == "term : term MULOP unary_expression")
                {
                    ASTInternalNode *childTerm = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    ASTInternalNode *unary_expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    string mulOp = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 2)->getNodePtr())->getSymbolInfo()->getName();
                    if (childTerm != nullptr && unary_expressionNode != nullptr)
                    {
                        string termNextLabel = genLabel();
                        childTerm->setIsCondition(false);
                        childTerm->setNextLabel(termNextLabel);

                        string unary_expressionNextLabel = genLabel();
                        unary_expressionNode->setIsCondition(false);
                        unary_expressionNode->setNextLabel(unary_expressionNextLabel);

                        generateIntermediateCode(childTerm);
                        printLabel(childTerm->getNextLabel());
                        generateIntermediateCode(unary_expressionNode);
                        printLabel(unary_expressionNode->getNextLabel());
                        
                        genPOP("CX", internalNode->getLastLineNo());
                        genPOP("AX", internalNode->getLastLineNo());
                        genCWD();
                        if (mulOp == "*")
                        {
                            genMUL("CX");
                            genPUSH("AX");
                        }
                        else if (mulOp == "/")
                        {
                            genDIV("CX");
                            genPUSH("AX");
                        }
                        else if (mulOp == "%")
                        {
                            genDIV("CX");
                            genPUSH("DX");
                        }
                    }
                }
                else if (rule == "unary_expression : ADDOP unary_expression")
                {
                    ASTLeafNode *addOpNode = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (addOpNode != nullptr)
                    {
                        string addOp = addOpNode->getSymbolInfo()->getName();
                        if (addOp == "-")
                        {
                            ASTInternalNode *unaryExpressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 2)->getNodePtr());
                            if (unaryExpressionNode != nullptr)
                            {
                                string label1 = genLabel();
                                unaryExpressionNode->setIsCondition(false);
                                unaryExpressionNode->setIsCondition(internalNode->getIsCondition());
                                unaryExpressionNode->setNextLabel(label1);
                                generateIntermediateCode(unaryExpressionNode);
                                printLabel(unaryExpressionNode->getNextLabel());
                                genPOP("AX");
                                genNEG("AX");
                                genPUSH("AX");
                                genJMP(internalNode->getNextLabel());
                            }
                        }
                    }
                }
                else if (rule == "unary_expression : NOT unary_expression")
                {
                    ASTInternalNode *unaryExpressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 2)->getNodePtr());
                    if (unaryExpressionNode != nullptr)
                    {
                        string label = genLabel();
                        internalNode->setIsNOTLogical(true);
                        unaryExpressionNode->setNextLabel(label);
                        unaryExpressionNode->setIsCondition(false);
                        generateIntermediateCode(unaryExpressionNode);
                        printLabel(unaryExpressionNode->getNextLabel());
                        if(!internalNode->getIsCondition())
                        {  
                            string trueLabel = genLabel();
                            string falseLabel = genLabel();
                            genPOP("AX");
                            genCMP("AX", "0");
                            genJNE(trueLabel);
                            genJMP(falseLabel);
                            printLabel(trueLabel);
                            genMOV("AX", "0");
                            genPUSH("AX");
                            genJMP(internalNode->getNextLabel());
                            printLabel(falseLabel);
                            genMOV("AX", "1");
                            genPUSH("AX");
                            genJMP(internalNode->getNextLabel());
                        }
                    }
                }
                else if (rule == "unary_expression : factor")
                {
                    ASTInternalNode *factorNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (factorNode != nullptr)
                    {
                        factorNode->setIsCondition(internalNode->getIsCondition());
                        factorNode->setTrueLabel(internalNode->getTrueLabel());
                        factorNode->setFalseLabel(internalNode->getFalseLabel());
                        factorNode->setNextLabel(internalNode->getNextLabel());
                        generateIntermediateCode(factorNode);
                    }
                }
                else if (rule == "factor : LPAREN expression RPAREN")
                {
                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 2)->getNodePtr());
                    if (expressionNode != nullptr)
                    {
                        expressionNode->setIsCondition(internalNode->getIsCondition());
                        expressionNode->setTrueLabel(internalNode->getTrueLabel());
                        expressionNode->setFalseLabel(internalNode->getFalseLabel());
                        expressionNode->setNextLabel(internalNode->getNextLabel());
                        generateIntermediateCode(expressionNode);                      
                    }
                }

                else if (rule == "factor : ID LPAREN argument_list RPAREN")
                {
                    ASTInternalNode *argument_listNode = dynamic_cast<ASTInternalNode *>(getChild(internalNode, 3)->getNodePtr());
                    generateIntermediateCode(argument_listNode);
                    string funcName = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 1)->getNodePtr())->getSymbolInfo()->getName();
                    genCALL(funcName);
                    genPUSH("AX");
                }
                else if (rule == "factor : variable INCOP")
                {
                    ASTVariableNode *variableNode = dynamic_cast<ASTVariableNode *>((getChild(internalNode, 1))->getNodePtr());
                    generateIntermediateCode(getChild(internalNode, 1)->getNodePtr());
                    genINC("AX");
                    genPUSH("AX");
                    generateCodeForVarAssign(variableNode);
                }
                else if (rule == "factor : variable DECOP")
                {
                    ASTVariableNode *variableNode = dynamic_cast<ASTVariableNode *>((getChild(internalNode, 1))->getNodePtr());
                    generateIntermediateCode(getChild(internalNode, 1)->getNodePtr());
                    genDEC("AX");
                    genPUSH("AX");
                    generateCodeForVarAssign(variableNode);
                }
                else if (rule == "factor : CONST_INT")
                {
                    ASTLeafNode *constIntNode = dynamic_cast<ASTLeafNode *>(getChild(internalNode, 1)->getNodePtr());
                    if (constIntNode != nullptr)
                    {
                        genMOV("AX", constIntNode->getSymbolInfo()->getName(), internalNode->getLastLineNo());
                        genPUSH("AX");
                    }
                }
                else
                {
                    ASTNodeList *children = internalNode->getChildren();
                    while (children != nullptr)
                    {
                        ASTInternalNode *childInternalNode = dynamic_cast<ASTInternalNode *>(children->getNodePtr());
                        if (childInternalNode != nullptr)
                        {
                            childInternalNode->setNextLabel(internalNode->getNextLabel());
                        }
                        generateIntermediateCode(children->getNodePtr());
                        children = children->nextNodePtr;
                    }
                }
            }
        }
    }
    void generateCodeForVarAssign(ASTVariableNode *variableNode)
    {
        VariableInfo *variableInfo = variableNode->getVariableInfo();
        if (variableInfo != nullptr)
        {
            if (variableInfo->getType() == "VARIABLE")
            {
                genPOP("AX");
                if (variableInfo->getScopeId() == "1")
                {
                    genMOV(variableInfo->getName(), "AX", variableNode->getLastLineNo());
                }
                else if (variableInfo->getOffset() > 0)
                {
                    genMOV("[BP-" + to_string(variableInfo->getOffset()) + "]", "AX", variableNode->getLastLineNo());
                }
                else if (variableInfo->getParamOffset() > 0)
                {
                    genMOV("[BP+" + to_string(funcParamCount * 2 - variableInfo->paramOffset + 4) + "]", "AX", variableNode->getLastLineNo());
                }
            }
            else if (variableInfo->getType() == "ARRAY")
            {
                ArrayInfo *arrayInfo = dynamic_cast<ArrayInfo *>(variableInfo);
                if (arrayInfo != nullptr)
                {
                    ASTInternalNode *expressionNode = dynamic_cast<ASTInternalNode *>(getChild(variableNode, 3)->getNodePtr());
                    generateIntermediateCode(expressionNode);
                    genPOP("BX");
                    genMOV("AX", "2");
                    genMUL("BX");
                    genMOV("BX", "AX");

                    if (arrayInfo->getScopeId() == "1")
                    {
                        genPOP("AX");
                        genMOV(arrayInfo->getName() + "[BX]", "AX");
                    }
                    else if (arrayInfo->getOffset() > 0)
                    {
                        genMOV("AX", to_string(2 * arrayInfo->getArraySize()));
                        genSUB("AX", "BX");
                        genMOV("BX", "AX");
                        genPOP("AX");
                        genMOV("SI", "BX");
                        genNEG("SI");
                        genMOV("[BP+SI]", "AX");
                    }
                }
            }
        }
    }
};