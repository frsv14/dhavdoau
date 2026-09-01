#ifndef CFG_H
#define CFG_H

#include "../Node.h"
#include "../symbol_table/Class.h"

class Cfg {
private:
    int currentBlock;
public:
    void createCFG(Node* node, AClass* currentClass = nullptr) {
        if (!node) return;

        if (node->type == "VarDecl" || node->type == "VarDeclAssign") {
            Node* nodeType = node->children.empty() ? nullptr : node->children.front();
            Variable* variable = new Variable(node->value, varType);
            put(node->value, variable);

            if (node->type == "VarDeclAssign") {
                for (auto i = std::next(node->children.begin()); i != node->children.end(); ++i) {
                    createCFG(*i);
                }
            }
        }
        

        for (auto child : node->children) 
            createCFG(child, currentClass);
    }
};

#endif