#ifndef NODE_H
#define NODE_H

#include "../symbol_table/Class.h"
#include <list>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#ifdef __unix__
#include <unistd.h>
#include <sys/wait.h>
#endif

#include <random>
#include <string>
#include <stdexcept>

#include "../symbol_table/SymbolTable.h"
#include "../intermediate_representation/BBlock.h"
#include "../intermediate_representation/Tac.h"
#include "../intermediate_representation/Expression.h"
#include "../intermediate_representation/AssignmentStmt.h"
#include "../intermediate_representation/CondJump.h"
#include "../intermediate_representation/Jump.h"
#include "../intermediate_representation/PrintStmt.h"
#include "../intermediate_representation/ReturnStmt.h"
#include "../intermediate_representation/ReadStmt.h"
#include "../intermediate_representation/CallStmt.h"


using namespace std;
extern int errCode;

struct IRResult {
	std::string value;
	BBlock* block;
	bool terminated;

	IRResult(std::string value, BBlock* block, bool terminated = false)
		: value(value), block(block), terminated(terminated) {}
};

class Node {
public:	
	int id, lineno;
	string type, value;
	list<Node*> children;
	BBlock* loopBreakTarget = nullptr;
	BBlock* loopContinueTarget = nullptr;
	Node(string t, string v, int l) : type(t), value(v), lineno(l){}
	Node()
	{
		type = "uninitialised";
		value = "uninitialised"; 
	}   // Bison needs this.
	std::string getValue() { return value; }

	virtual void createTable(SymbolTable& st, AClass* currentClass = nullptr) {
        if (type == "Program") {
            // Program
            for (auto child : children) 
            	child->createTable(st, currentClass);
            return;
        }

        if (type == "VarDecl" || type == "VarDeclAssign") {
            Node* nodeType = children.empty() ? nullptr : children.front();
            std::string varType;

            if (nodeType != nullptr && nodeType->type == "ArrayType" && nodeType->children.empty() == false)
				varType = nodeType->children.front()->type + "[]";
			else if (nodeType != nullptr && (nodeType->type == "TypeID" || nodeType->type == "Identifier"))
				varType = nodeType->value;
			else if (nodeType != nullptr)
				varType = nodeType->type;
			else
				varType = "Error!";

            Variable* variable = new Variable(value, varType);
            st.put(value, variable);

            if (type == "VarDeclAssign") {
                for (auto i = std::next(children.begin()); i != children.end(); i++) {
                    (*i)->createTable(st, currentClass);
                }
            }
        }

        if (type == "Block") {
            st.enterScope();

            for (auto child : children) 
                child->createTable(st, currentClass);
            st.exitScope();
            return;
        }

        for (auto child : children) 
            child->createTable(st, currentClass);
    }

	virtual std::string semanticAnalysis(SymbolTable& st) {
		if (type == "Class") {
			st.enterScope();
			for (auto child : children) {
				child->semanticAnalysis(st);
			}
			st.exitScope();
			return type;
		}

		if (type == "Method" || type == "Main") {
			if (type == "Main")
				duplicateIdentifier(st); 
			st.enterScope();

			for (auto child : children) {
				if (child->type == "Params") {
					continue;
				}

				if (child->type == "Block") {
					for (auto blockChild : child->children) {
						blockChild->semanticAnalysis(st);
					}
				} else {
					child->semanticAnalysis(st);
				}
			}

			st.exitScope();
			return type;
		}

		
		if (type == "Block") {
			st.enterScope();
			for (auto child : children) {
				child->semanticAnalysis(st);
			}
			st.exitScope();
			return type;
		}

		if (type == "FuncCall") {
			Record* record = st.lookup(value);

			if (record == nullptr || (record->getKind() != "Method" && record->getKind() != "Class")) {
				std::cerr << "@error at line " << lineno << " - semantic (undefined class: " << value << ")" << std::endl;
				errCode = 4;
				return "error";
			}

			if (!children.empty()) {
				for (Node* argument : children.front()->children) {
					if (argument->semanticAnalysis(st) == "error") return "error";
				}
			}

			return record->getType();
		}

		if (type == "MemberAccess") {
			if (children.size() != 2) {
				std::cerr << "@error at line " << lineno << " - semantic (invalid MemberAccess)" << std::endl;
				errCode = 4;
				return "error";
			}

			auto i = children.begin();
			std::string memberType = (*i)->semanticAnalysis(st);
			i++;
			std::string memberName = (*i)->value;

			if (memberType == "error") return "error";

			if (memberType.substr(memberType.size() - 2) == "[]" && memberName == "length") return "IntLiteral"; // Returns type - [] ex. IntType instead of IntType[]
			else {
				std::cerr << "@error at line " << lineno << " - semantic (member ." << memberName << " used on non-array)" << std::endl;
				errCode = 4;
				return "error";
			}
		}

		for (auto child : children) {
			child->semanticAnalysis(st);
		}
		return type;
	}

	void duplicateIdentifier(SymbolTable& st) {
        std::multimap<std::string, Record*> records = st.getCurrentScope()->getRecords();
        int count = 0;

        for (auto it : records) {
            if (it.first == value)
                count++;
        }
        if (count > 1) {
			std::cerr << "@error at line " << lineno << " - semantic (Duplicated identifier " << value << ")" << std::endl;
			errCode = 4;
		}
    }

	std::string mathSemanticCheck(SymbolTable& st, std::string operand) {
        auto i = children.begin();
		if (children.size() != 2 && operand != "Not") {
			std::cerr << "@error at line " << lineno << " - semantic (expected two operands)" << std::endl;
			errCode = 4;
			return "error";
		}
		std::string lhsType = (*i)->semanticAnalysis(st);
		std::string rhsType;
		if (operand != "Not") {
			i++;
			rhsType = (*i)->semanticAnalysis(st);
		}

		if (lhsType == "error" || rhsType == "error")
			return "error";
		
		std::set<std::string> allOperands = {"+", "-", "*", "/", "^", "Eq", "Neq", "Lt", "Gt", "Lte", "Gte", "And", "Or", "Not"};
		std::set<std::string> validNumericOperands = {"+", "-", "*", "/", "^", "Eq", "Neq", "Lt", "Gt", "Lte", "Gte"};
		std::set<std::string> validNumericCompareOperands = {"Eq", "Neq", "Lt", "Gt", "Lte", "Gte"};
		std::set<std::string> validBooleanOperands = {"Eq", "Neq", "And", "Or", "Not"};
		std::set<std::string> numericTypes = {"IntLiteral", "IntType", "FloatLiteral", "FloatType"};
		std::set<std::string> booleanTypes = {"BoolType", "True", "False"};

		if (numericTypes.find(lhsType) != numericTypes.end() && numericTypes.find(rhsType) != numericTypes.end() && validNumericOperands.find(operand) != validNumericOperands.end()) {
			std::string resType;
			if (validNumericCompareOperands.find(operand) != validNumericCompareOperands.end())
				resType = "BoolType";
			else if (lhsType.substr(0, 5) == "Float" || rhsType.substr(0,5) == "Float")
				resType = "FloatType";
			else if (lhsType.substr(0, 3) == "Int" || rhsType.substr(0,3) == "Int")
				resType = "IntType";
			else
				return "error";

			if (lhsType.substr(lhsType.size() - 2) == "[]" && rhsType.substr(rhsType.size() - 2) == "[]")
				resType += "[]";
			return resType;
		}
		else if (booleanTypes.find(lhsType) != booleanTypes.end() && booleanTypes.find(rhsType) != booleanTypes.end() && validBooleanOperands.find(operand) != validBooleanOperands.end())
			return "BoolType";
		else if (operand == "Not" && booleanTypes.find(lhsType) != booleanTypes.end())
			return "BoolType";
		else {
			if (allOperands.find(operand) != allOperands.end())
				std::cerr << "@error at line " << lineno << " - semantic (invalid operand type: " << lhsType << " " << operand << " " << rhsType << ")" << std::endl;
			else 
				std::cerr << "@error at line " << lineno << " - semantic (undefined math operation error)" << std::endl;
			errCode = 4;
			
			if (operand == "Or" || operand == "And")
				return "BoolType";
			return "error";
		}
	}

	std::string assignmentSemanticCheck(SymbolTable& st) {
        auto i = children.begin();
		if (children.size() != 2) {
			std::cerr << "@error at line " << lineno << " - semantic (expected type and value)" << std::endl;
			errCode = 4;
			return "error";
		}

		std::string lhsType = (*i)->semanticAnalysis(st);
		i++;
		std::string rhsType = (*i)->semanticAnalysis(st);

		if (lhsType == "error" || rhsType == "error")
			return "error";
		
		if (lhsType == "IntType" && (rhsType != "IntLiteral" && rhsType != "IntType")) {
			std::cerr << "@error at line " << lineno << " - semantic (type mismatch: got " << rhsType << ", expected " << lhsType << ")" << std::endl;
			errCode = 4;
			return "error";
		}
		else if (lhsType == "FloatType" && (rhsType != "FloatLiteral" && rhsType != "FloatType" && rhsType != "IntLiteral" && rhsType != "IntType")) {
			std::cerr << "@error at line " << lineno << " - semantic (type mismatch: got " << rhsType << ", expected " << lhsType << ")" << std::endl;
			errCode = 4;
			return "error";
		}
		else if (lhsType == "BoolType" && (rhsType != "True" && rhsType != "False" && rhsType != "BoolType")) {
			std::cerr << "@error at line " << lineno << " - semantic (type mismatch: got " << rhsType << ", expected " << lhsType << ")" << std::endl;
			errCode = 4;
			return "error";
		}
		return lhsType;
	}

	void setLoopTargets(BBlock* breakTarget, BBlock* continueTarget) {
		loopBreakTarget = breakTarget;
		loopContinueTarget = continueTarget;
		for (auto child : children) {
			child->setLoopTargets(breakTarget, continueTarget);
		}
	}

	virtual IRResult genIR(BBlock *currentBlock) {
		if (type == "And") return emitBinary(currentBlock, "&&");
		if (type == "Or") return emitBinary(currentBlock, "||");
		if (type == "Not") {
			if (children.size() != 1) {
				throw std::logic_error("Not expression must have one operand");
			}
			IRResult operand = children.front()->genIR(currentBlock);
			std::string result = generateString();
			operand.block->addTacInstructions(
				new Expression("!", operand.value, result));
			return {result, operand.block};
		}
		if (type == "FuncCall") {
			return emitCall(currentBlock, value,
				children.empty() ? nullptr : children.front());
		}
		if (type == "MethodCall") {
			if (children.size() < 2 || children.size() > 3) {
				throw std::logic_error("Method call must have a receiver and method name");
			}
			auto child = children.begin();
			IRResult receiver = (*child)->genIR(currentBlock);
			++child;
			std::string callee = receiver.value + "." + (*child)->value;
			++child;
			return emitCall(receiver.block, callee,
				child == children.end() ? nullptr : *child);
		}
		if (type == "MemberAccess") {
			if (children.size() != 2) {
				throw std::logic_error("Member access must have a receiver and member");
			}
			auto child = children.begin();
			IRResult receiver = (*child)->genIR(currentBlock);
			++child;
			return {receiver.value + "." + (*child)->value, receiver.block};
		}
		if (type == "ArrayAccess") {
			if (children.size() != 2) {
				throw std::logic_error("Array access must have an array and index");
			}
			auto child = children.begin();
			IRResult array = (*child)->genIR(currentBlock);
			++child;
			IRResult index = (*child)->genIR(array.block);
			return {array.value + "[" + index.value + "]", index.block};
		}
		if (type == "Length") {
			if (children.size() != 1) {
				throw std::logic_error("Length expression must have one operand");
			}
			IRResult array = children.front()->genIR(currentBlock);
			std::string result = generateString();
			array.block->addTacInstructions(
				new Expression("length", array.value, result));
			return {result, array.block};
		}
		if (type == "ArrayLiteral" || type == "EmptyArrayLiteral") {
			std::string elementType = "unknown";
			Node *elements = nullptr;
			if (type == "ArrayLiteral") {
				if (children.size() != 2) {
					throw std::logic_error("Array literal must have a type and elements");
				}
				auto child = children.begin();
				elementType = (*child)->type;
				elements = *++child;
			} else if (!children.empty()) {
				throw std::logic_error("Empty array literal cannot have children");
			}

			std::vector<std::string> values;
			if (elements != nullptr) {
				for (Node *element : elements->children) {
					IRResult result = element->genIR(currentBlock);
					currentBlock = result.block;
					values.push_back(result.value);
				}
			}
			std::string result = generateString();
			currentBlock->addTacInstructions(new Expression(
				"newarray", elementType, std::to_string(values.size()), result));
			for (std::size_t i = 0; i < values.size(); ++i) {
				currentBlock->addTacInstructions(new AssignmentStmt(
					result + "[" + std::to_string(i) + "]", values[i]));
			}
			return {result, currentBlock};
		}
		if (type != "Block" && type != "StmtList" && type != "ExprList") {
			throw std::logic_error("IR generation is not implemented for AST node: " + type);
		}
		for (auto i = children.begin(); i != children.end(); i++) {
			IRResult result = (*i)->genIR(currentBlock);
			if (result.terminated) return result;
			currentBlock = result.block;
		}
		return {"", currentBlock};
	}

	std::string generateString() {
		//former name, namegenertor one
		static std::size_t nextName = 0;
		return "__t" + std::to_string(nextName++);
	}

	std::string generateBlockName() {
		//former name, namegenertor two
		static std::size_t nextName = 0;
		return "B" + std::to_string(nextName++);
	}

	//Printing tree

	IRResult emitBinary(BBlock *currentBlock, const std::string& op) {
		if (children.size() != 2) {
			throw std::logic_error(type + " expression must have two operands");
		}

		auto operand = children.begin();
		IRResult lhs = (*operand)->genIR(currentBlock);
		++operand;
		IRResult rhs = (*operand)->genIR(lhs.block);
		std::string result = generateString();
		rhs.block->addTacInstructions(new Expression(op, lhs.value, rhs.value, result));
		return {result, rhs.block};
	}


	IRResult emitCall(BBlock *currentBlock, const std::string& callee,
	                  Node *arguments) {
		std::vector<std::string> values;
		if (arguments != nullptr) {
			for (Node *argument : arguments->children) {
				IRResult result = argument->genIR(currentBlock);
				currentBlock = result.block;
				values.push_back(result.value);
			}
		}

		std::string result = generateString();
		currentBlock->addTacInstructions(new CallStmt(callee, values, result));
		return {result, currentBlock};
	}

	void print_tree(int depth=0) {
		for(int i=0; i<depth; i++)
		cout << "  ";
		cout << type << ":" << value << endl; //<< " @line: "<< lineno << endl;
		for(auto i=children.begin(); i!=children.end(); i++)
		(*i)->print_tree(depth+1);
	}
  
	void generate_tree() {
		std::ofstream outStream;
		char* filename = "tree.dot";
	  	outStream.open(filename);

		int count = 0;
		outStream << "digraph {" << std::endl;
		generate_tree_content(count, &outStream);
		outStream << "}" << std::endl;
		outStream.close();

		printf("\nBuilt a parse-tree at %s. Use 'make tree' to generate the pdf version.\n", filename);
  	}

  	void generate_tree_content(int &count, ofstream *outStream) {
	  id = count++;
	  *outStream << "n" << id << " [label=\"" << type << ":" << value << "\"];" << endl;

	  for (auto i = children.begin(); i != children.end(); i++)
	  {
		  (*i)->generate_tree_content(count, outStream);
		  *outStream << "n" << id << " -> n" << (*i)->id << endl;
	  }
  }
};

#endif
