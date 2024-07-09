#include <iostream>
#include "2005021_SymbolTable.h"

string removeSpaces(const string &str) {
    string result;
    for (char c : str) {
        if (!isspace(c)) {
            result += c;
        }
    }
    return result;
}

int splitString(string line, string delimiter, string args[]){
    int count = 0;
    int end = line.find(delimiter); 
    while (end != -1) {
        args[count++] = removeSpaces(line.substr(0, end));
        line.erase(line.begin(), line.begin() + end + 1);
        end = line.find(delimiter);
    }
    args[count++] = removeSpaces(line.substr(0, line.length()));
    return count;
}

int main(){
    SymbolTable *symbolTable;

    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    
    string bucketCount;
    getline(cin, bucketCount);

    symbolTable = new SymbolTable(stoull(bucketCount));

    int cmd_no = 0;
    while(!feof(stdin)){
        cmd_no++;
        string line, argv[10];
        getline(cin, line);
        if(line != "\0"){
            int argc = splitString(line, " ", argv);

            cout << "Cmd " + to_string(cmd_no) + ": " << line << endl;

            if(argv[0] == "I"){
                if(argc != 3){
                    cout << "\tWrong number of arugments for the command I" << endl; 
                }else{
                    symbolTable->Insert(argv[1], argv[2]);
                }
            }else if(argv[0] == "L"){
                if(argc != 2){
                    cout << "\tWrong number of arugments for the command L" << endl; 
                }else{
                    symbolTable->LookUp(argv[1]);
                }
            }else if(argv[0] == "D"){
                if(argc != 2){
                    cout << "\tWrong number of arugments for the command D" << endl; 
                }else{
                    symbolTable->Remove(argv[1]);
                }
            }else if(argv[0] == "P"){
                if(argc != 2){
                    cout << "\tWrong number of arugments for the command P" << endl; 
                }else{
                    if(argv[1] == "A"){
                        symbolTable->PrintAllScopeTable();
                    }else if(argv[1] == "C"){
                        symbolTable->PrintCurrentScopeTable();
                    }else{
                        cout << "\tInvalid argument for the command P" << endl;
                    }
                }
            }else if(argv[0] == "S"){
                if(argc != 1){
                    cout << "\tWrong number of arugments for the command S" << endl; 
                }else{
                    symbolTable->EnterScope();
                }
            }else if(argv[0] == "E"){
                if(argc != 1){
                    cout << "\tWrong number of arugments for the command E" << endl; 
                }else{
                    symbolTable->ExitScope();
                }
            }else if(argv[0] == "Q"){
                if(argc != 1){
                    cout << "Wrong number of arugments for the command Q" << endl; 
                }else{
                    delete symbolTable;
                }
            }
        }
    }
    fclose(stdin);
    fclose(stdout);
    
    return 0;
}