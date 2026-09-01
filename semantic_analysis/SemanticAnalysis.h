#ifndef SEMANTICANALYSIS_H
#define SEMANTICANALYSIS_H

#include <iterator>
#include <ostream>
#include <string>
#include <iostream>
#include <cmath>

#include "../Node.h"
#include "../symbol_table/Class.h"
#include "../symbol_table/SymbolTable.h"
#include "../symbol_table/Scope.h"
#include "../symbol_table/Class.h"
#include "../symbol_table/Method.h"

//TODO: FIX THE EVALUATE FUNCTION SO THAT IT TAKES ONE NODE AND EVALUATES ITS TYPE,
//      FOR EXAMPLE, SEND A CHILD NODE, IF ITS ALREADY HAS ITS TYPE DEFINED THEN JUST SEND IT BACK, IF IT IS AN EXPRESSION LIKE +, -, *, / AND SO ON,
//      THEN EVALUATE ITS FINAL TYPE.
//      THEN IN THE TRAVERSAL FUNCTIONS IF STATEMENTS WHICH CALLED THE EVALUATE FUNCITON, EVALUATE IF THE CHILD NODE TYPES ARE THE SAME OR NOT.
//      THIS IS HOW IT WAS SHOWED IN THE PRESENTATION ASWELL, AND IT MAKES IT A LOT EASIER WHEN DEALING WITH OTHER STUFF SINCE YOU DONT HAVE TO COVER EVERY EDGE CASE IN SEPERATE FUNCTIONS THAT ALMOST LOOK THE SAME
class SemanticAnalysis {
private:
    std::string currentMethod;
public:
    void traversal(SymbolTable& st, Node* currentNode, Scope* currentScope) {
    Scope* nextScope = currentScope;

    if (currentNode->type == "Block" || currentNode->type == "Method" || currentNode->type == "Main") {
        nextScope = currentNode->scope;
        if (currentNode->type == "Method" || currentNode->type == "Main") {
            currentMethod = currentNode->value;
        }
        st.setCurrentScope(nextScope);
    }
    
    // TODO: HANDLE IF EXPRESSION LIST IS NOT EMPTY!!!
    if (currentNode->type == "MethodCall") {
        int count = 0;
        std::string type;
        std::cout << "----- MethodCall children -----" << std::endl;

        for (auto child : currentNode->children) {
            if (count == 0) {
                type = st.lookup(child->value)->getType();
                count++;
            }
            else if (count == 1) {
                AClass* currentClass = dynamic_cast<AClass*>(st.lookup(type));
                duplicateIdentifier(currentScope, child);

                if (currentClass->lookupMethod(child->value) == nullptr) 
                    std::cout << "Undeclared identifier " << child->value << std::endl; 
                count++;
            }

        std::cout << "-------------------------------" << std::endl;
        }
        return;
    }

    if (currentNode->type == "Return") {
        std::cout << "----- Return -----" << std::endl;
        Node* firstChild = (*currentNode->children.begin());
        std::string type = firstChild->type;
        if (type == "Add" || type == "Sub" || type == "Mul" || type == "Div" || type == "Lt" || type == "Gt")
            type = evaluate(currentScope, firstChild, st); // TODO: THIS CURRENTLY WONT WORK SINCE EVALUATE EXPECTES 2 CHILD NODES IF ITS A MATH EXPRESSION LIKE ADD OR SOMETHING
        
        if (st.lookup(currentMethod)->getType() == "IntType" && type != "IntLiteral")
                std::cout << "Type missmatch, Expecting " << st.lookup(currentMethod)->getType() << " but got " << type << std::endl;
        else if (st.lookup(currentMethod)->getType() == "FloatType" && (type != "FloatLiteral" && type != "IntLiteral"))
            std::cout << "Type missmatch, Expecting " << st.lookup(currentMethod)->getType() << " but got " << type << std::endl;
        std::cout << "------------------" << std::endl;
    }

    if (currentNode->type == "If" || currentNode->type == "IfElse") {
        std::cout << "----- If/IfElse children -----" << std::endl;
        std::string type = evaluate(currentScope, (*currentNode->children.begin()), st);
        if (type != "IntType" && type != "FloatType" && type != "IntLiteral" && type != "FloatLiteral")
            std::cout << "Error: Did not get a type int or float at " << to_string(currentNode->lineno) << std::endl;

        std::cout << "------------------------------" << std::endl;
    }
    
    if (currentNode->type == "Identifier") {
        std::cout << "----- Identifier -----" << std::endl;
        undeclaredIdentifier(st, currentNode);
        duplicateIdentifier(currentScope, currentNode);
        std::cout << "----------------------------------" << std::endl;
    }

    if (currentNode->type == "VarDecl") {
        std::cout << "----- VarDecl -----" << std::endl;
        duplicateIdentifier(currentScope, currentNode);
        std::cout << "----------------------------------" << std::endl;
    }

    if (currentNode->type == "VarDeclAssign") {
        std::cout << "----- VarDeclAssign children -----" << std::endl;
        auto i = currentNode->children.begin();
        std::string lhs = evaluate(currentScope, *i, st);
        i++;
        std::string rhs = evaluate(currentScope, *i, st);

        if (lhs == "IntType" && rhs != "IntLiteral")
            std::cout << "Type missmatch, Expecting " << lhs << " but got " << rhs << std::endl;
        else if (lhs == "FloatType" && (rhs != "FloatLiteral" && rhs != "IntLiteral"))
            std::cout << "Type missmatch, Expecting " << lhs << " but got " << rhs << std::endl;
        else if (lhs == "BoolType" && (rhs != "True" && rhs != "False"))
            std::cout << "Type missmatch, Expecting " << lhs << " but got " << rhs << std::endl;
        std::cout << "----------------------------------" << std::endl;
    }

    if (!currentNode->children.empty()) {
        for (auto child : currentNode->children)
            traversal(st, child, nextScope);
    }
}

    void undeclaredIdentifier(SymbolTable& st, Node* currentNode) {
        std::string type = st.lookup(currentNode->value)->getType();
        AClass* currentClass = dynamic_cast<AClass*>(st.lookup(type));

        if (st.lookup(currentNode->value) == nullptr && currentClass->lookupMethod(currentNode->value) == nullptr) 
            std::cout << "Undeclared identifier " << currentNode->value << std::endl; 
    }

    void duplicateIdentifier(Scope* currentScope, Node* currentNode) {
        std::multimap<std::string, Record*> records = currentScope->getRecords();
        int count = 0;

        for (auto it : records) {
            if (it.first == currentNode->value)
                count++;
        }
        if (count > 1) 
            std::cout << "Duplicated identifier " << currentNode->value << std::endl;
    }

    std::string evaluate(Scope* currentScope, Node* currentNode, SymbolTable& st) {
        std::string retType; 

        if (currentNode->type == "Identifier") {
            retType = st.lookup(currentNode->value)->getType();
        }
        else
            retType = currentNode->type;

        if (retType == "Add" || retType == "Sub" || retType == "Mul" || retType == "Div" || retType == "Lt" || retType == "Gt" || retType == "Pow") {
            std::string result = expressionEvaluate(currentNode, st, currentScope);
            if (result == "Error")
                return "Error";

            float value = stof(result);
            if (value == floor(value))
                retType = "IntLiteral";
            else
                retType = "FloatLiteral";
        }
        return retType;
    }

    std::string expressionEvaluate(Node* currentNode, SymbolTable& st, Scope* currentScope) {
        // Recursivley check expressions correctness
        // Ex. 1 + 1, 1 - 4, 1 + false, 5 / (2 + 5)
        // must check both left and right child nodes because they could contain another expression which needs to be evaluated
        // Use some sort of deep search that starts att leaf nodes snd recursivley moves upwards (post-order)
        // Might need symbol table since and expression could be two identifiers 
        // Ex. a + b where a = 1 and b = 2, c - d where c = 3.0 and d = false
        // TODO: FIX ERROR HANDLING, CURRENTLY ITS WIERD
        // TODO: FIX HOW IT HANDLES IDENTIFIERS (CURRENT THOUGHT EITHER CHANGE THE FUNCTION TO LOOK AT STRING TYPES INSTEAD OF NUMBERS OR ADD THE VALUE OF A IDENTIFIER TO THE SYMBOL TABLE)
        std::string type = currentNode->type;
        if (type == "Identifier" || type == "FuncCall") {
            type = st.lookup(currentNode->value)->getType();
            if (type == "FloatType")
                return "1.5";
            else if (type == "IntType")
                return "1";
            else
                std::cout << "Type missmatch, got " << type << std::endl;
                return "Error";
        }

        if (currentNode->children.empty() && (type == "FloatLiteral" || type == "IntLiteral"))
            return currentNode->value;
        else if (type != "Add" && type != "Sub" && type != "Mul" && type != "Div" && type != "Lt" && type != "Gt" && type != "Pow"){
            std::cout << to_string(currentNode->lineno) << ": Type missmatch, got " << currentNode->type << std::endl;
            return "Error";
        }

        auto i = currentNode->children.begin();
        std::string lhs = expressionEvaluate(*i, st, currentScope);
        if (lhs == "Error")
            return "Error";
        i++;
        std::string rhs = expressionEvaluate(*i, st, currentScope);
        if (rhs == "Error")
            return "Error";

        float res;
        if (currentNode->type == "Add") {
            res = std::stof(lhs) + std::stof(rhs);
            return to_string(res);
        }
        if (currentNode->type == "Sub") {
            res = std::stof(lhs) - std::stof(rhs);
            return to_string(res);
        }
        if (currentNode->type == "Mul") {
            res = std::stof(lhs) * std::stof(rhs);
            return to_string(res);
        }
        if (currentNode->type == "Div") {
            res = std::stof(lhs) / std::stof(rhs);
            return to_string(res);
        }
        if (currentNode->type == "Pow") {
            res = pow(std::stof(lhs), std::stof(rhs));
            return to_string(res);
        }
        return to_string(res);
    }
};

#endif