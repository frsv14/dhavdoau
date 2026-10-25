#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "Scope.h"
#include <string>

#include "Record.h"
#include "Variable.h"
#include "Method.h"
#include "Class.h"

#include <fstream>
#include <iostream>

class SymbolTable {
private:
    Scope* root;
    Scope* current;

public:
    SymbolTable() {
        root = new Scope(nullptr);
        current = this->root;
    }
    Scope* getRootScope() {return root;}
    Scope* getCurrentScope() {return current;}
    void setCurrentScope(Scope* currentScope) {current = currentScope;}
    void enterScope() {current = current->nextChild();}
    void exitScope() {current = current->getParentScope();}
    void put(std::string key, Record* item) {current->setRecords(key, item);}
    Record* lookup(std::string key) {return current->lookup(key);}
    void printTable() {root->printTable();}
    void resetTable() {root->resetScope();}

    void generateSymbolTable() {
        std::string filename = "symbol_table.dot";
        std::ofstream outstream;
        outstream.open(filename);
        if (!outstream.is_open()) {
            std::cerr << "Unable to create symbol table dot file: " << filename << std::endl;
            return;
        }

        int count = 0;
        outstream << "digraph {" << std::endl;
        generateSymbolTableContent(root, count, &outstream);
        outstream << "}" << std::endl;
        outstream.close();
        std::cout << "Successfully built a symbol table dot file \"" << filename << "\"" << std::endl;
    }
    
    void generateSymbolTableContent(Scope* scope, int& count, std::ofstream* outStream) {
        if (!scope) return;

        int id = count++;
        *outStream << "n" << id << " [label=\"Scope\"];" << std::endl;

        for (auto entry : scope->getRecords()) {
            Record* record = entry.second;
            int recordId = count++;
            *outStream << "n" << recordId << " [shape=box, label=\"" << "ID: " << entry.first << " | Record: " << record->getKind() << " | Type: " << record->getType() << "\"];" << std::endl;
            *outStream << "n" << id << " -> n" << recordId << std::endl;
        }

        for (Scope* child : scope->getChildrenScopes()) {
            if (!child) continue;

            int childId = count;
            generateSymbolTableContent(child, count, outStream);
            *outStream << "n" << id << " -> n" << childId << std::endl;
        }
    }    
};

#endif