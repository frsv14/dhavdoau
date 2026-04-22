#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include "Node.h"

enum class SymbolKind {
    Class,
    Method,
    Variable,
    Parameter
};

class Symbol {
public:
    std::string name;
    std::string type;
    SymbolKind kind;

    Symbol(const std::string& n,
           const std::string& t,
           SymbolKind k)
        : name(n), type(t), kind(k) {}
};

class Scope {
private:
    Scope* parent;
    std::string name;
    std::vector<Scope*> children;
    std::unordered_map<std::string, Symbol*> symbols;

public:
    Scope(Scope* p = nullptr, const std::string& n = "") : parent(p), name(n) {}
    
    ~Scope() {
        for (auto& pair : symbols)
            delete pair.second;
        for (auto child : children)
            delete child;  
    }

    Scope* enterScope(const std::string& scopeName = "") {
        Scope* child = new Scope(this, scopeName);
        children.push_back(child);
        return child;
    }

    Scope* getParent() {
        return parent;
    }

    bool insert(Symbol* sym) {
        if (symbols.count(sym->name)) {
            std::cerr << "Semantic Error: Duplicate declaration of "
                      << sym->name << std::endl;
            return false;
        }
        symbols[sym->name] = sym;
        return true;
    }

    Symbol* lookup(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end())
            return it->second;

        return parent ? parent->lookup(name) : nullptr;
    }

    void print(int level = 0) {
        for (int i = 0; i < level; ++i)
            std::cout << "  ";

        std::cout << "Scope Level " << level << std::endl;

        for (auto& pair : symbols) {
            for (int i = 0; i < level; ++i)
                std::cout << "  ";

            std::cout << "  "
                      << pair.second->name
                      << " : "
                      << pair.second->type
                      << std::endl;
            
            switch (pair.second->kind) {
                case SymbolKind::Class:
                    std::cout << "    (Class)" << std::endl;
                    break;
                case SymbolKind::Method:
                    std::cout << "    (Method)" << std::endl;
                    break;
                case SymbolKind::Variable:
                    std::cout << "    (Variable)" << std::endl;
                    break;
                case SymbolKind::Parameter:
                    std::cout << "    (Parameter)" << std::endl;
                    break;
            }
        }

        for (auto child : children)
            child->print(level + 1);
    }
};

class SymbolTable {
private:
    Scope* root;
    Scope* current;

public:
    SymbolTable() {
        root = new Scope(nullptr);
        current = root;
    }

    void enterScope(const std::string& scopeName = "") {
        current = current->enterScope(scopeName);
    }

    void exitScope() {
        if (current->getParent())
            current = current->getParent();
    }

    bool insert(Symbol* sym) {
        return current->insert(sym);
    }

    Symbol* lookup(const std::string& name) {
        return current->lookup(name);
    }

    void print() {
        std::cout << "\n======= SYMBOL TABLE =======\n";
        root->print();
        std::cout << "============================\n";
    }
};

class SymbolTableBuilder {
private:
    SymbolTable table;

public:
    void build(Node* root) {
        visit(root);
    }

    SymbolTable& getTable() {
        return table;
    }

private:

    void visit(Node* node) {
        if (!node) return;

        if (node->type == "Program") {
            for (auto child : node->children)
                visit(child);
        }

        else if (node->type == "ClassDecl") {

            table.insert(new Symbol(
                node->value, 
                "-",
                SymbolKind::Class));

            table.enterScope("Class: " + node->value);

            for (auto child : node->children)
                visit(child);

            table.exitScope();
        }

        else if (node->type == "MethodDecl") {

            std::string returnType = "";
            if (!node->children.empty())
                returnType = node->children.front()->value;

            table.insert(new Symbol(
                node->value,
                returnType,
                SymbolKind::Method));

            table.enterScope("Method: " + node->value);

            for (auto child : node->children)
                visit(child);

            table.exitScope();
        }

        else if (node->type == "Param") {

            std::string paramType = "";
            if (!node->children.empty())
                paramType = node->children.front()->value;

            table.insert(new Symbol(
                node->value,
                paramType,
                SymbolKind::Parameter));
        }

        else if (node->type == "VarDecl" ||
                 node->type == "VarDeclAssign") {

            std::string varType = "";
            if (!node->children.empty())
                varType = node->children.front()->value;

            table.insert(new Symbol(
                node->value,
                varType,
                SymbolKind::Variable));

            if (node->type == "VarDeclAssign") {
                auto it = node->children.begin();
                std::advance(it, 1);
                if (it != node->children.end())
                    visit(*it);
            }
        }

        else if (node->type == "main") {
            table.enterScope("main");
            for (auto child : node->children)
                visit(child);
            table.exitScope();
        }

        else if (node->type == "Block") {
            table.enterScope("Block");
            for (auto child : node->children)
                visit(child);
            table.exitScope();
        }

        else {
            for (auto child : node->children)
                visit(child);
        }
    }
};

#endif