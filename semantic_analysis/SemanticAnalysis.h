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
    bool first = true;
public:

    void traversal(SymbolTable& st, Node* currentNode) {
        
        if (currentNode->type == "Class" || currentNode->type == "Block" ||
            currentNode->type == "Method" || currentNode->type == "Main") {
            st.enterScope();
            if (currentNode->type == "Method" || currentNode->type == "Main") {
                currentMethod = currentNode->value;
            }
        }
        
        if (currentNode->type == "Method") {
            auto params = currentNode->children.begin();
            if (params != currentNode->children.end()) {
                for (auto child : (*params)->children) {
                    duplicateIdentifier(child, st);
                }
            }
        }

        if (currentNode->type == "MethodCall") {
            if (currentNode->children.size() < 2 ||
                currentNode->children.size() > 3) {
                std::cerr << "Invalid method call at line "
                          << currentNode->lineno << '\n';
                return;
            }

            auto child = currentNode->children.begin();
            Node* receiver = *child++;
            Node* methodName = *child++;
            Record* receiverRecord = nullptr;
            if (receiver->type == "Identifier") {
                receiverRecord = st.lookup(receiver->value);
            } else if (receiver->type == "FuncCall") {
                receiverRecord = st.lookup(receiver->value);
            }
            AClass* receiverClass = dynamic_cast<AClass*>(receiverRecord);
            if (receiverClass == nullptr && receiverRecord != nullptr) {
                receiverClass = dynamic_cast<AClass*>(st.lookup(receiverRecord->getType()));
            }
            if (receiverClass != nullptr &&
                receiverClass->lookupMethod(methodName->value) == nullptr) {
                std::cerr << "Undeclared method " << methodName->value
                          << " at line " << currentNode->lineno << '\n';
            }

            receiver->type == "FuncCall"
                ? traverseChildren(st, receiver)
                : traversal(st, receiver);
            if (child != currentNode->children.end()) {
                traversal(st, *child);
            }
            return;
        }

        if (currentNode->type == "MemberAccess") {
            if (!currentNode->children.empty()) {
                traversal(st, currentNode->children.front());
            }
            return;
        }

        if (currentNode->type == "Return") {
            if (!currentNode->children.empty()) {
                Node* expression = currentNode->children.front();
                Record* method = st.lookup(currentMethod);
                if (method != nullptr) {
                    std::string actualType = evaluate(expression, st);
                    const std::string expectedType = method->getType();
                    if (!typesCompatible(expectedType, actualType)) {
                        std::cerr << "Type mismatch, expecting " << expectedType
                                  << " but got " << actualType << '\n';
                    }
                }
            }
        }

        if (currentNode->type == "If" || currentNode->type == "IfElse") {
            if (!currentNode->children.empty()) {
                std::string type = evaluate(currentNode->children.front(), st);
                if (type != "Error" && type != "BoolType" &&
                    type != "True" && type != "False" &&
                    type != "IntType" && type != "FloatType" &&
                    type != "IntLiteral" && type != "FloatLiteral") {
                    std::cerr << "Invalid if-condition type at line "
                              << currentNode->lineno << ": " << type << '\n';
                }
            }
        }
        
        if (currentNode->type == "Identifier") {
            undeclaredIdentifier(st, currentNode);
        }

        if (currentNode->type == "VarDecl") {
            duplicateIdentifier(currentNode, st);
        }

        if (currentNode->type == "VarDeclAssign" &&
            currentNode->children.size() == 2) {
            auto i = currentNode->children.begin();
            std::string lhs = evaluate(*i, st);
            i++;
            std::string rhs = evaluate(*i, st);

            if (rhs != "Error" && !typesCompatible(lhs, rhs)) {
                std::cerr << "Type mismatch, expecting " << lhs
                          << " but got " << rhs << '\n';
            }
        }

        if (!currentNode->children.empty()) {
            for (auto child : currentNode->children)
                traversal(st, child);
        }

        if (currentNode->type == "Class" || currentNode->type == "Block" ||
            currentNode->type == "Method" || currentNode->type == "Main") {
            st.exitScope();
        }
    }

    void undeclaredIdentifier(SymbolTable& st, Node* currentNode) {
        if (st.lookup(currentNode->value) == nullptr) {
            std::cerr << "Undeclared identifier " << currentNode->value
                      << " at line " << currentNode->lineno << '\n';
        }
    }

    void traverseChildren(SymbolTable& st, Node* currentNode) {
        for (Node* child : currentNode->children) traversal(st, child);
    }

    static bool typesCompatible(const std::string& expected,
                                const std::string& actual) {
        if (expected == actual) return true;
        if (expected == "IntType") return actual == "IntLiteral";
        if (expected == "FloatType") {
            return actual == "FloatLiteral" || actual == "IntLiteral" ||
                   actual == "IntType";
        }
        if (expected == "BoolType") return actual == "True" || actual == "False";
        if (expected.size() > 2 &&
            expected.compare(expected.size() - 2, 2, "[]") == 0) {
            return actual == expected;
        }
        return false;
    }

    void duplicateIdentifier(Node* currentNode, SymbolTable& st) {
        std::multimap<std::string, Record*> records = st.getCurrentScope()->getRecords();
        int count = 0;

        for (auto it : records) {
            if (it.first == currentNode->value)
                count++;
        }
        if (count > 1) 
            std::cerr << "Duplicated identifier " << currentNode->value << std::endl;
    }

    std::string evaluate(Node* currentNode, SymbolTable& st) {
        if (currentNode == nullptr) return "Error";
        const std::string& nodeType = currentNode->type;
        if (nodeType == "IntLiteral" || nodeType == "FloatLiteral" ||
            nodeType == "True" || nodeType == "False") return nodeType;
        if (nodeType == "IntType" || nodeType == "FloatType" ||
            nodeType == "BoolType" || nodeType == "VoidType") return nodeType;
        if (nodeType == "TypeID") return currentNode->value;
        if (nodeType == "ArrayType") {
            if (currentNode->children.empty()) return "Unknown[]";
            return evaluate(currentNode->children.front(), st) + "[]";
        }
        if (nodeType == "Identifier") {
            Record* record = st.lookup(currentNode->value);
            return record == nullptr ? "Error" : record->getType();
        }
        if (nodeType == "MemberAccess") {
            if (currentNode->children.size() != 2) return "Error";
            auto child = currentNode->children.begin();
            const std::string receiverType = evaluate(*child, st);
            Node* member = *++child;
            if (member->value == "length") return "IntType";
            AClass* receiverClass = dynamic_cast<AClass*>(st.lookup(receiverType));
            if (receiverClass != nullptr) {
                Variable* field = receiverClass->lookupVariable(member->value);
                if (field != nullptr) return field->getType();
            }
            return "Error";
        }
        if (nodeType == "ArrayAccess") {
            if (currentNode->children.size() != 2) return "Error";
            std::string arrayType = evaluate(currentNode->children.front(), st);
            if (arrayType.size() > 2 &&
                arrayType.compare(arrayType.size() - 2, 2, "[]") == 0) {
                arrayType.resize(arrayType.size() - 2);
                return arrayType;
            }
            return "Error";
        }
        if (nodeType == "Length") return "IntType";
        if (nodeType == "Eq" || nodeType == "Neq" || nodeType == "Lt" ||
            nodeType == "Gt" || nodeType == "Lte" || nodeType == "Gte" ||
            nodeType == "And" || nodeType == "Or" || nodeType == "Not") {
            return "BoolType";
        }
        if (nodeType == "FuncCall") {
            Record* callee = st.lookup(currentNode->value);
            if (callee == nullptr) return "Error";
            return dynamic_cast<AClass*>(callee) == nullptr
                ? callee->getType() : currentNode->value;
        }
        if (nodeType == "MethodCall") {
            if (currentNode->children.size() < 2) return "Error";
            auto child = currentNode->children.begin();
            const std::string receiverType = evaluate(*child++, st);
            Node* methodName = *child;
            AClass* receiverClass = dynamic_cast<AClass*>(st.lookup(receiverType));
            if (receiverClass == nullptr) return "Error";
            Method* method = receiverClass->lookupMethod(methodName->value);
            return method == nullptr ? "Error" : method->getType();
        }
        if (nodeType == "ArrayLiteral") {
            if (currentNode->children.empty()) return "Error";
            return evaluate(currentNode->children.front(), st) + "[]";
        }
        if (nodeType == "EmptyArrayLiteral") return "Unknown[]";
        if (nodeType == "Add" || nodeType == "Sub" || nodeType == "Mul" ||
            nodeType == "Div" || nodeType == "Pow") {
            if (currentNode->children.size() != 2) return "Error";
            auto child = currentNode->children.begin();
            const std::string lhs = evaluate(*child++, st);
            const std::string rhs = evaluate(*child, st);
            if ((lhs == "IntType" || lhs == "IntLiteral") &&
                (rhs == "IntType" || rhs == "IntLiteral")) return "IntType";
            if ((lhs == "IntType" || lhs == "IntLiteral" ||
                 lhs == "FloatType" || lhs == "FloatLiteral") &&
                (rhs == "IntType" || rhs == "IntLiteral" ||
                 rhs == "FloatType" || rhs == "FloatLiteral")) return "FloatType";
            return "Error";
        }
        return nodeType;
    }

    std::string expressionEvaluate(Node* currentNode, SymbolTable& st) {
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
                std::cerr << "Type missmatch, got " << type << std::endl;
                return "Error";
        }

        if (currentNode->children.empty() && (type == "FloatLiteral" || type == "IntLiteral"))
            return currentNode->value;
        else if (type != "Add" && type != "Sub" && type != "Mul" && type != "Div" && type != "Lt" && type != "Gt" && type != "Pow" && type != "Eq"){
            std::cerr << to_string(currentNode->lineno) << ": Type missmatch, got " << currentNode->type << std::endl;
            return "Error";
        }

        auto i = currentNode->children.begin();
        std::string lhs = expressionEvaluate(*i, st);
        if (lhs == "Error")
            return "Error";
        i++;
        std::string rhs = expressionEvaluate(*i, st);
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
        if (currentNode->type == "Eq") {
            return "1";
        }
        return to_string(res);
    }
};

#endif