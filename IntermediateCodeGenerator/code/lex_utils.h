#pragma once

#include <iostream>
#include <fstream>

using namespace std;

class SymbolInfo
{
protected:
    string name;
    string type;
    string scopeId;

public:
    int offset;
    int paramOffset;
    SymbolInfo *nestSymbolInfo;
    SymbolInfo(string name, string type)
    {
        this->name = name;
        this->type = type;
        this->offset = -1;
        this->paramOffset = -1;
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
    void setOffset(int offset)
    {
        this->offset = offset;
    }
    int getOffset()
    {
        return this->offset;
    }
    void setScopeId(string scopeId)
    {
        this->scopeId = scopeId;
    }
    string getScopeId()
    {
        return this->scopeId;
    }
    void setParamOffset(int paramOffset)
    {
        this->paramOffset = paramOffset;
    }
    int getParamOffset()
    {
        return this->paramOffset;
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
    int parameterCount;
    int funcStackOffset;

public:
    FunctionInfo(string name, string returnType, VariableList *parameterList = nullptr, string type = "FUNCTION") : VariableInfo(name, returnType, type)
    {
        this->returnType = returnType;
        this->parameterList = parameterList;
        this->funcStackOffset = 0;
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
    void setParameterCount(int parameterCount)
    {
        this->parameterCount = parameterCount;
    }
    void setFuncStackOffset(int funcStackOffset)
    {
        this->funcStackOffset = funcStackOffset;
    }
    int getFuncStackOffset()
    {
        return funcStackOffset;
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

        if (successfulInsertion == true) {
            return true;
        } else {
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
            if (chainIterator->getName() == symbolName){
                return chainIterator;
            } else {
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
    SymbolInfo **getHashTable()
    {
        return scopeHashTable;
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
            currentScopeTable = currentScopeTable->parentScope;
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
    string getCurrentScopeTableId()
    {
        return currentScopeTable->getId();
    }
    SymbolInfo **getCurrentScopeTableHashTable()
    {
        return currentScopeTable->getHashTable();
    }
    unsigned long long getTotalBuckets()
    {
        return totalBuckets;
    }
};