#pragma once

#include <iostream>
#include <fstream>

using namespace std;

class SymbolInfo
{
protected:
    string name;
    string type;

public:
    SymbolInfo *nestSymbolInfo;
    SymbolInfo(string name, string type)
    {
        this->name = name;
        this->type = type;
        this->nestSymbolInfo = nullptr;
    }
    void setName(string name)
    {
        this->name = name;
    }
    string getName()
    {
        return this->name;
    }
    void setType(string type)
    {
        this->type = type;
    }
    string getType()
    {
        return this->type;
    }
    virtual string toString()
    {
        return "<" + name + "," + type + ">";
    }
};

class VariableInfo : public SymbolInfo
{
protected:
    string typeSspecifier;

public:
    VariableInfo(string name, string typeSspecifier = "UNDEFINED", string type = "VARIABLE") : SymbolInfo(name, type)
    {
        this->typeSspecifier = typeSspecifier;
    }
    string getTypeSpecifier()
    {
        return this->typeSspecifier;
    }
    void setTypeSpecifier(string typeSspecifier)
    {
        this->typeSspecifier = typeSspecifier;
    }
    virtual string toString()
    {
        return "<" + name + "," + typeSspecifier + ">";
    }
};

class ArrayInfo : public VariableInfo
{
protected:
    int arraySize;

public:
    ArrayInfo(string name, int arraySize, string typeSspecifier = "UNDEFINED", string type = "ARRAY") : VariableInfo(name, typeSspecifier, type)
    {
        this->arraySize = arraySize;
    }
    int getArraySize()
    {
        return this->arraySize;
    }
    virtual string toString()
    {
        return "<" + name + "," + type + ">";
    }
};

class VariableList
{
private:
    int size;
    VariableInfo *list;
    VariableInfo *lastVariable;

public:
    VariableList()
    {
        this->size = 0;
        this->list = nullptr;
        this->lastVariable = nullptr;
    }
    void addVariable(VariableInfo *variableInfo)
    {
        if (list == nullptr)
        {
            this->list = variableInfo;
            this->lastVariable = list;
        }
        else
        {
            this->lastVariable->nestSymbolInfo = variableInfo;
            this->lastVariable = (VariableInfo *)lastVariable->nestSymbolInfo;
        }
        size++;
    }
    int getSize()
    {
        return size;
    }
    VariableInfo *getHead()
    {
        return list;
    }
    bool findVariable(string variableName)
    {
        VariableInfo *listIterator = list;
        while (listIterator != nullptr)
        {
            if (listIterator->getName() == variableName)
            {
                return true;
            }
            else
            {
                listIterator = (VariableInfo *)listIterator->nestSymbolInfo;
            }
        }
        return false;
    }
    bool isEqualTo(VariableList *parameterList)
    {
        if (parameterList == nullptr)
        {
            return false;
        }
        if (size != parameterList->getSize())
        {
            return false;
        }
        VariableInfo *listIterator = list;
        VariableInfo *parameterListIterator = parameterList->list;
        while (listIterator != nullptr && parameterListIterator != nullptr)
        {
            if (listIterator->getTypeSpecifier() != parameterListIterator->getTypeSpecifier())
            {
                return false;
            }
            else
            {
                listIterator = (VariableInfo *)listIterator->nestSymbolInfo;
                parameterListIterator = (VariableInfo *)parameterListIterator->nestSymbolInfo;
            }
        }
        return true;
    }
};

class FunctionInfo : public VariableInfo
{
protected:
    bool defined = false;
    string returnType;
    VariableList *parameterList;

public:
    FunctionInfo(string name, string returnType, VariableList *parameterList = nullptr, string type = "FUNCTION") : VariableInfo(name, returnType, type)
    {
        this->returnType = returnType;
        this->parameterList = parameterList;
    }
    bool isDefined()
    {
        return defined;
    }
    void setDefined()
    {
        defined = true;
    }
    string getReturnType()
    {
        return returnType;
    }
    void setParameterList(VariableList *parameterList)
    {
        this->parameterList = parameterList;
    }
    VariableList *getParameterList()
    {
        return parameterList;
    }
    int getParameterCount()
    {
        if (parameterList == nullptr)
        {
            return 0;
        }
        return parameterList->getSize();
    }
    bool isCompatibleWith(FunctionInfo *functionInfo)
    {
        if (functionInfo == nullptr)
        {
            return false;
        }
        if (returnType != functionInfo->getReturnType())
        {
            return false;
        }
        if (parameterList == nullptr && functionInfo->getParameterList() == nullptr)
        {
            return true;
        }
        if (parameterList == nullptr || functionInfo->getParameterList() == nullptr)
        {
            return false;
        }
        return parameterList->isEqualTo(functionInfo->getParameterList());
    }
    virtual string toString()
    {
        return "<" + name + "," + type + "," + returnType + ">";
    }
};

class ScopeTable
{
private:
    string id;
    unsigned long long nextChildSerial;
    unsigned long long totalBuckets;
    SymbolInfo **scopeHashTable;

    unsigned long long Hash(string str)
    {
        unsigned long long hash = 0;
        unsigned long long i = 0;
        unsigned long long len = str.length();

        for (i = 0; i < len; i++)
        {
            hash = (str[i]) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }
    unsigned long long getChainIndex(string SymbolName)
    {
        return Hash(SymbolName) % totalBuckets;
    }

public:
    ScopeTable *parentScope;
    ScopeTable(string id, unsigned long long totalBuckets, ScopeTable *parentScope = nullptr)
    {
        this->id = id;
        this->totalBuckets = totalBuckets;
        this->parentScope = parentScope;
        this->scopeHashTable = new SymbolInfo *[totalBuckets];
        for (int i = 0; i < totalBuckets; i++)
        {
            scopeHashTable[i] = nullptr;
        }
        this->nextChildSerial = 1;
    }
    ~ScopeTable()
    {
        parentScope = nullptr;
        for (int i = 0; i < totalBuckets; i++)
        {
            SymbolInfo *chainRoot = scopeHashTable[i];
            scopeHashTable[i] = nullptr;
            while (chainRoot != nullptr)
            {
                SymbolInfo *toBeDeleted = chainRoot;
                chainRoot = chainRoot->nestSymbolInfo;
                delete toBeDeleted;
            }
        }
        delete[] scopeHashTable;
    }
    bool Insert(SymbolInfo *symbolInfo)
    {
        bool successfulInsertion;
        unsigned long long symbolInfoIndex = -1;
        unsigned long long chainIndex = getChainIndex(symbolInfo->getName());

        if (scopeHashTable[chainIndex] == nullptr)
        {
            scopeHashTable[chainIndex] = symbolInfo;
            symbolInfoIndex++;
            successfulInsertion = true;
        }
        else
        {
            symbolInfoIndex++;
            SymbolInfo *chainIterator = scopeHashTable[chainIndex];
            if (chainIterator->getName() == symbolInfo->getName())
            {
                successfulInsertion = false;
            }
            else
            {
                symbolInfoIndex++;
                while (chainIterator->nestSymbolInfo != nullptr)
                {
                    if (chainIterator->nestSymbolInfo->getName() == symbolInfo->getName())
                    {
                        successfulInsertion = false;
                        break;
                    }
                    else
                    {
                        symbolInfoIndex++;
                        chainIterator = chainIterator->nestSymbolInfo;
                    }
                }
                if (chainIterator->nestSymbolInfo == nullptr)
                {
                    successfulInsertion = true;
                    chainIterator->nestSymbolInfo = symbolInfo;
                }
            }
        }

        if (successfulInsertion == true)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    SymbolInfo *LookUp(string symbolName)
    {
        unsigned long long symbolInfoIndex = -1;
        unsigned long long chainIndex = getChainIndex(symbolName);
        SymbolInfo *chainIterator = scopeHashTable[chainIndex];
        while (chainIterator != nullptr)
        {
            symbolInfoIndex++;
            if (chainIterator->getName() == symbolName)
            {
                return chainIterator;
            }
            else
            {
                chainIterator = chainIterator->nestSymbolInfo;
            }
        }
        return chainIterator;
    }
    bool Delete(string &symbolName)
    {
        bool successfulDeletion = false;
        unsigned long long symbolInfoIndex = -1;
        unsigned long long chainIndex = getChainIndex(symbolName);

        if (scopeHashTable[chainIndex] != nullptr)
        {
            if (scopeHashTable[chainIndex]->getName() == symbolName)
            {
                symbolInfoIndex++;
                SymbolInfo *toBeDeleted = scopeHashTable[chainIndex];
                SymbolInfo *toBeLinked = toBeDeleted->nestSymbolInfo;
                scopeHashTable[chainIndex] = toBeLinked;
                delete toBeDeleted;
                successfulDeletion = true;
            }
            else
            {
                SymbolInfo *chainIterator = scopeHashTable[chainIndex];
                while (chainIterator->nestSymbolInfo != nullptr)
                {
                    symbolInfoIndex++;
                    if (chainIterator->nestSymbolInfo->getName() == symbolName)
                    {
                        SymbolInfo *toBeDeleted = chainIterator->nestSymbolInfo;
                        SymbolInfo *toBeLinked = toBeDeleted->nestSymbolInfo;
                        chainIterator->nestSymbolInfo = toBeLinked;
                        delete toBeDeleted;
                        successfulDeletion = true;
                        break;
                    }
                    else
                    {
                        chainIterator = chainIterator->nestSymbolInfo;
                    }
                }
            }
        }

        if (successfulDeletion == true)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    string toString()
    {
        string scopeTable = "";
        scopeTable += "\tScopeTable# " + id + "\n";
        for (int i = 0; i < totalBuckets; i++)
        {
            SymbolInfo *toBePrinted = scopeHashTable[i];
            if (toBePrinted != nullptr)
            {
                scopeTable += "\t" + to_string(i + 1) + "--> ";
                while (toBePrinted != nullptr)
                {
                    scopeTable += toBePrinted->toString() + " ";
                    toBePrinted = toBePrinted->nestSymbolInfo;
                }
                scopeTable += "\n";
            }
        }
        return scopeTable;
    }
    string getId()
    {
        return id;
    }
    unsigned long long getNextChildSerial()
    {
        return nextChildSerial++;
    }
};

class SymbolTable
{
private:
    int latestScopeTableId;
    ScopeTable *scopeTableList;
    ScopeTable *currentScopeTable;
    unsigned long long totalBuckets;

public:
    SymbolTable(unsigned long long totalBuckets)
    {
        this->latestScopeTableId = 1;
        this->scopeTableList = nullptr;
        this->totalBuckets = totalBuckets;
        createNewScopeTable(to_string(latestScopeTableId));
    }
    ~SymbolTable()
    {
        while (currentScopeTable != nullptr)
        {
            deleteCurrentScope();
        }
    }
    void EnterScope()
    {
        latestScopeTableId++;
        createNewScopeTable(to_string(latestScopeTableId));
    }
    void ExitScope()
    {
        if (currentScopeTable->parentScope != nullptr)
        {
            deleteCurrentScope();
        }
    }
    bool Insert(SymbolInfo *symbolInfo)
    {
        return currentScopeTable->Insert(symbolInfo);
    }
    bool Remove(string symbolName)
    {
        return currentScopeTable->Delete(symbolName);
    }
    SymbolInfo *LookUpCurrentScope(string symbolName)
    {
        return currentScopeTable->LookUp(symbolName);
    }
    SymbolInfo *LookUp(string symbolName)
    {
        SymbolInfo *lookUpResult = nullptr;
        ScopeTable *tmpScopeTable = currentScopeTable;
        while (tmpScopeTable != nullptr)
        {
            lookUpResult = tmpScopeTable->LookUp(symbolName);
            if (lookUpResult)
            {
                return lookUpResult;
            }
            else
            {
                tmpScopeTable = tmpScopeTable->parentScope;
            }
        }
        return lookUpResult;
    }
    void createNewScopeTable(string newScopeTableId)
    {
        if (scopeTableList == nullptr)
        {
            scopeTableList = new ScopeTable(newScopeTableId, totalBuckets);
            currentScopeTable = scopeTableList;
        }
        else
        {
            ScopeTable *tmpScopeTabe = new ScopeTable(newScopeTableId, totalBuckets, currentScopeTable);
            currentScopeTable = tmpScopeTabe;
        }
    }
    void deleteCurrentScope()
    {
        ScopeTable *temp = currentScopeTable;
        currentScopeTable = currentScopeTable->parentScope;
        delete temp;
    }
    string PrintCurrentScopeTable()
    {
        return currentScopeTable->toString();
    }
    string PrintAllScopeTable()
    {
        string str = "";
        ScopeTable *temp = currentScopeTable;
        while (temp != nullptr)
        {
            str += temp->toString();
            temp = temp->parentScope;
        }
        return str;
    }
};

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
    int arraryIndex;

public:
    ASTVariableNode(string rule, VariableInfo *variableInfo, int fisrtLineNo, int lastLineNo) : ASTInternalNode(rule, fisrtLineNo, lastLineNo)
    {
        this->variableInfo = variableInfo;
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

public:
    AST()
    {
        root = nullptr;
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
};