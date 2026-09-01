#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "Scope.h"
#include "../Node.h"
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

    /////////////////////////
    std::string escapeDotLabel(const std::string& text) {
        std::string escaped;
        escaped.reserve(text.size());

        for (char c : text) {
            if (c == '"' || c == '\\') {
                escaped.push_back('\\');
            }
            if (c == '\n') {
                escaped.push_back('\\');
                escaped.push_back('n');
            } else {
                escaped.push_back(c);
            }
        }

        return escaped;
    }

    std::string writeScopeToDot(Scope* scope, std::ofstream& out, int& nextNodeId) {
        if (!scope) {
            return "";
        }

        int scopeId = nextNodeId++;
        std::string scopeNode = "n" + std::to_string(scopeId);
        out << "  " << scopeNode << " [label=\"Scope\", shape=ellipse];" << std::endl;

        for (const auto& entry : scope->getRecords()) {
            Record* record = entry.second;
            int recordId = nextNodeId++;
            std::string recordNode = "n" + std::to_string(recordId);
            std::string recordLabel = escapeDotLabel(
                "id: " + entry.first + " | Record: " + record->getKind() + " | type: " + record->getType()
            );
            out << "  " << recordNode << " [label=\"" << recordLabel << "\", shape=note];" << std::endl;
            out << "  " << scopeNode << " -> " << recordNode << ";" << std::endl;
        }

        for (Scope* child : scope->getChildrenScopes()) {
            std::string childNode = writeScopeToDot(child, out, nextNodeId);
            if (!childNode.empty()) {
                out << "  " << scopeNode << " -> " << childNode << " [style=dashed];" << std::endl;
            }
        }

        return scopeNode;
    }
    ///////////////////////////////
public:
    SymbolTable() {
        root = new Scope(nullptr);
        current = this->root;
    }
    Scope* getRootScope() {return root;}
    void setCurrentScope(Scope* currentScope) {current = currentScope;}
    void enterScope() {current = current->nextChild();}
    void exitScope() {current = current->getParentScope();}
    void put(std::string key, Record* item) {current->setRecords(key, item);}
    Record* lookup(std::string key) {return current->lookup(key);}
    void printTable() {root->printTable();}
    void resetTable() {root->resetScope();}

    void createTable(Node* node, AClass* currentClass = nullptr) {
        if (!node) return;

        if (node->type == "Program") {
            // Program
            for (auto child : node->children) 
                createTable(child);
            return;
        }

        if (node->type == "Class") {
            // Class
            AClass* aClass = new AClass(node->value, node->type);
            put(node->value, aClass);

            enterScope();
            node->scope = current;

            for (auto child : node->children) 
                createTable(child, aClass);
            exitScope();
            return;
        }

        if (node->type == "Method" || node->type == "Main") {
            std::string methodName = node->value;
            std::string returnType = (node->type == "Main") ? "IntType" : "";

            if (node->type != "Main" && node->children.size() > 1) {
                auto it = node->children.begin();
                it++;
                if (it != node->children.end()) {
                    returnType = (*it)->type;
                }
            }

            Method* method = new Method(methodName, returnType);
            put(method->getId(), method);
            
            if (currentClass != nullptr) {
                currentClass->addMethod(method->getId(), method);
            }

            enterScope();
            node->scope = current;

            if (!node->children.empty()) {
                Node* paramsNode = node->children.front();
                for (auto child : paramsNode->children) {
                    if (child->type != "Param") 
                        continue;

                    std::string paramType = child->children.empty() ? "Empty param type" : child->children.front()->type;
                    Variable* parameter = new Variable(child->value, paramType);
                    method->addParameter(parameter);
                    put(parameter->getId(), parameter);
                }
            }

            for (auto child : node->children) {
                if (child->type != "Params" && currentClass != nullptr) {
                    createTable(child, currentClass);
                }
                else if (child->type != "Params") {
                    createTable(child);
                }
            }
            exitScope();
            return;
        }

        if (node->type == "VarDecl" || node->type == "VarDeclAssign") {
            Node* nodeType = node->children.empty() ? nullptr : node->children.front();
            std::string varType;
            if(nodeType != nullptr && (nodeType->type == "TypeID" || nodeType->type == "Identifier"))
                varType = nodeType->value;
            else if (nodeType != nullptr) 
                varType = nodeType->type;

            Variable* variable = new Variable(node->value, varType);
            put(node->value, variable);

            if (node->type == "VarDeclAssign") {
                for (auto i = std::next(node->children.begin()); i != node->children.end(); ++i) {
                    createTable(*i);
                }
            }
        }

        if (node->type == "Block") {
            enterScope();
            node->scope = current;

            for (auto child : node->children) 
                createTable(child);
            exitScope();
            return;
        }

        for (auto child : node->children) 
            createTable(child, currentClass);
    }

    //////////////////////////////
    void exportToDot(const std::string& filename = "symbol_table.dot") {
        std::ofstream out(filename);
        if (!out.is_open()) {
            std::cerr << "Unable to create symbol table DOT file: " << filename << std::endl;
            return;
        }

        int nextNodeId = 0;
        out << "digraph SymbolTable {" << std::endl;
        out << "  graph [rankdir=TB];" << std::endl;
        out << "  node [shape=box, style=rounded];" << std::endl;
        writeScopeToDot(root, out, nextNodeId);
        out << "}" << std::endl;
        out.close();

        std::cout << "Built a symbol-table graph at " << filename << std::endl;
    }
    //////////////////////////////
};

#endif