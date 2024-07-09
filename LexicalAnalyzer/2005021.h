#include <iostream>

using namespace std;

class SymbolInfo
{
private:
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

    string getName()
    {
        return name;
    }

    string getType()
    {
        return type;
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

    unsigned long long getChainIndex(string &SymbolName)
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

    bool Insert(string symbolName, string symbolType)
    {
        bool successfulInsertion;
        unsigned long long symbolInfoIndex = -1;
        unsigned long long chainIndex = getChainIndex(symbolName);

        if (scopeHashTable[chainIndex] == nullptr)
        {
            scopeHashTable[chainIndex] = new SymbolInfo(symbolName, symbolType);
            symbolInfoIndex++;
            successfulInsertion = true;
        }
        else
        {
            symbolInfoIndex++;
            SymbolInfo *chainIterator = scopeHashTable[chainIndex];
            if (chainIterator->getName() == symbolName)
            {
                successfulInsertion = false;
            }
            else
            {
                symbolInfoIndex++;
                while (chainIterator->nestSymbolInfo != nullptr)
                {
                    if (chainIterator->nestSymbolInfo->getName() == symbolName)
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
                    chainIterator->nestSymbolInfo = new SymbolInfo(symbolName, symbolType);
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

    SymbolInfo *LookUp(string &symbolName)
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
            scopeTable += "\t" + to_string(i + 1);
            SymbolInfo *toBePrinted = scopeHashTable[i];
            while (toBePrinted != nullptr)
            {
                scopeTable += " --> (" + toBePrinted->getName() + "," + toBePrinted->getType() + ")";
                toBePrinted = toBePrinted->nestSymbolInfo;
            }
            scopeTable += "\n";
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
    ScopeTable *scopeTableList;
    ScopeTable *currentScopeTable;
    unsigned long long totalBuckets;

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
        ScopeTable *toBeDeleted = currentScopeTable;
        currentScopeTable = currentScopeTable->parentScope;
        delete toBeDeleted;
    }

public:
    SymbolTable(unsigned long long totalBuckets)
    {
        this->scopeTableList = nullptr;
        this->totalBuckets = totalBuckets;
        string mainScopeTableId = "1";
        createNewScopeTable(mainScopeTableId);
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
        string newScopeTableId = currentScopeTable->getId() + "." + to_string(currentScopeTable->getNextChildSerial());
        createNewScopeTable(newScopeTableId);
    }

    void ExitScope()
    {
        if (currentScopeTable->parentScope != nullptr)
        {
            deleteCurrentScope();
        }
    }

    bool Insert(string symbolName, string symbolType)
    {
        return currentScopeTable->Insert(symbolName, symbolType);
    }

    bool Remove(string symbolName)
    {
        return currentScopeTable->Delete(symbolName);
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

    string PrintCurrentScopeTable()
    {
        return currentScopeTable->toString();
    }

    string PrintAllScopeTable()
    {
		string allScopeTable = "";
        ScopeTable *tmpScopeTable = currentScopeTable;
        while (tmpScopeTable != nullptr)
        {
            allScopeTable += tmpScopeTable->toString();
            tmpScopeTable = tmpScopeTable->parentScope;
        }
		return allScopeTable;
    }
};