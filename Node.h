#ifndef NODE_H
#define	NODE_H

#include "symbol_table/Class.h"
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

#include "./symbol_table/SymbolTable.h"
#include "./intermediate_representation/BBlock.h"
#include "./intermediate_representation/Tac.h"
#include "./intermediate_representation/Expression.h"
#include "./intermediate_representation/AssignmentStmt.h"
#include "./intermediate_representation/CondJump.h"
#include "./intermediate_representation/Jump.h"
#include "./intermediate_representation/PrintStmt.h"
#include "./intermediate_representation/ReturnStmt.h"
#include "./intermediate_representation/ReadStmt.h"
#include "./intermediate_representation/CallStmt.h"


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

		if (type == "Class") {
            // Class
			std::cout << "Type: " << type << std::endl;
			std::cout << "value: " << value << std::endl;
            AClass* aClass = new AClass(value, type);
            st.put(value, aClass);

            st.enterScope();
            for (auto child : children) 
                child->createTable(st, aClass);
            st.exitScope();
            return;
        }

        if (type == "Method" || type == "Main") {
            std::string methodName = value;
            std::string returnType = (type == "Main") ? "IntType" : "";

            if (type != "Main" && children.size() > 1) {
                auto i = children.begin();
                i++;
                if (i != children.end()) {
                    returnType = (*i)->type;
                }
            }

            Method* method = new Method(methodName, returnType);
            st.put(method->getId(), method);
            
            if (currentClass != nullptr) {
                currentClass->addMethod(method->getId(), method);
            }

            st.enterScope();

            if (!children.empty() && children.front()->type == "Params") {
                Node* paramsNode = children.front();
				
                for (auto child : paramsNode->children) {
                    if (child->type != "Param") continue;

					Node* nodeType = child->children.empty() ? nullptr : child->children.front();
					std::string paramType;

					if (nodeType != nullptr && nodeType->type == "ArrayType" && nodeType->children.empty() == false)
						paramType = nodeType->children.front()->type + "[]";
					else if (nodeType != nullptr && (nodeType->type == "TypeID" || nodeType->type == "Identifier"))
						paramType = nodeType->value;
					else if (nodeType != nullptr)
						paramType = nodeType->type;
					else
						paramType = "Error: non param type!";

					Variable* param = new Variable(child->value, paramType);
					method->addParameter(param);
					st.put(param->getId(), param);
                }
            }

            for (auto child : children) {
                if (child->type == "Params") continue;

				if (child->type == "Block")
						for (auto i : child->children)
							i->createTable(st, currentClass); 
				else
					child->createTable(st, currentClass);
            }
            st.exitScope();
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

			if (memberType.substr(memberType.size() - 2) == "[]" && memberName == "length") return "IntLiteral"; //Returns type - [] ex. IntType instead of IntType[]
			else {
				std::cerr << "@error at line " << lineno << " - semantic (member ." << memberName << " used on non-array)" << std::endl;
				errCode = 4;
				return "error";
			}
		}
			//
			// TODO: FIX THIS 
			//

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
		std::string lhsTypeCut = lhsType;
		std::string rhsTypeCut = rhsType;

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
		/*
		else if (lhsType != rhsType) {
			std::cerr << "@error at line " << lineno << " - semantic (type mismatch: got " << rhsType << ", expected " << lhsType << ")" << std::endl;
			errCode = 4;
			return "error";
		} 
		*/
		return lhsType;
	}

	void setLoopTargets(BBlock* breakTarget, BBlock* continueTarget) {
		loopBreakTarget = breakTarget;
		loopContinueTarget = continueTarget;
		for (auto child : children) {
			child->setLoopTargets(breakTarget, continueTarget);
		}
	}

	//logic for generating IR for each node type
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



class SubExpression : public Node {
private:
public:
	SubExpression(string t, string v, int l) : Node(t, v, l) {}
	~SubExpression() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		std::string ret = mathSemanticCheck(st, "-");
		return ret;
	}

	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "-");
	}
};

class AddExpression : public Node {
private:
public:
	AddExpression(string t, string v, int l) : Node(t, v, l) {}
	~AddExpression() {}
	std::string semanticAnalysis(SymbolTable& st) override {
		std::string ret = mathSemanticCheck(st, "+");
		return ret;
	}
	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "+");
	}
};

class DivExpression : public Node {
private:
public:
	DivExpression(string t, string v, int l) : Node(t, v, l) {}
	~DivExpression() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		std::string ret = mathSemanticCheck(st, "/");
		return ret;
	}

	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "/");
	}
};

class MulExpression : public Node {
private:
public:
	MulExpression(string t, string v, int l) : Node(t, v, l) {}
	~MulExpression() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		std::string ret = mathSemanticCheck(st, "*");
		return ret;
	}

	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "*");
	}
};

class PowExpression : public Node {
private:
public:
	PowExpression(string t, string v, int l) : Node(t, v, l) {}
	~PowExpression() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		std::string ret = mathSemanticCheck(st, "^");
		return ret;
	}

	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "^");
	}
};

class ComparisonExpression : public Node {
public:
	ComparisonExpression(string t, int l) : Node(t, "", l) {}
	~ComparisonExpression() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		std::string ret = mathSemanticCheck(st, type);
		return ret;
	}

	IRResult genIR(BBlock *currentBlock) override {
		if (type == "Not" || type == "And" || type == "Or") {
			return Node::genIR(currentBlock);
		}

		std::string op;
		if (type == "Eq") op = "==";
		else if (type == "Neq") op = "!=";
		else if (type == "Lt") op = "<";
		else if (type == "Gt") op = ">";
		else if (type == "Lte") op = "<=";
		else if (type == "Gte") op = ">=";
		else throw std::logic_error("Unsupported comparison operator: " + type);
		return emitBinary(currentBlock, op);
	}
};

class Identifier : public Node {
private:
public:
	Identifier(string t, string v, int l) : Node(t, v, l) {}
	~Identifier() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		Record* record = st.lookup(value);
		
		if (record == nullptr) {
			std::cerr << "@error at line " << lineno << " - semantic (undefined identifier: " << value << ")" << std::endl;
			errCode = 4;
			return "error"; 
		}
		return st.lookup(value)->getType(); 
	}

	IRResult genIR(BBlock *currentBlock) override {
		return {this->getValue(), currentBlock}; // return the name of the identifier
	}
};

class Integer : public Node {
private:
public:
	Integer(string t, string v, int l) : Node(t, v, l) {}
	~Integer() {}
	std::string semanticAnalysis(SymbolTable& st) override { return type; }
	IRResult genIR(BBlock *currentBlock) override {
		return {this->getValue(), currentBlock}; // return the value of the integer
	}
};

class Float : public Node {
private:
public:
	Float(string t, string v, int l) : Node(t, v, l) {}
	~Float() {}
	std::string semanticAnalysis(SymbolTable& st) override { return type; }
	IRResult genIR(BBlock *currentBlock) override {
		return {this->getValue(), currentBlock}; // return the value of the float
	}
};

class Boolean : public Node {
private:
public:
	Boolean(string t, string v, int l) : Node(t, v, l) {}
	~Boolean() {}
	std::string semanticAnalysis(SymbolTable& st) override { return type; }
	IRResult genIR(BBlock *currentBlock) override {
		return {this->getValue(), currentBlock}; // return the value of the boolean
	}
};

class VariableDeclaration : public Node {
public:
	VariableDeclaration(string t, string v, int l) : Node(t, v, l) {}
	~VariableDeclaration() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		duplicateIdentifier(st); 
		if (type == "VarDeclAssign") {
			std::string ret = assignmentSemanticCheck(st);
			return ret;
		}
		return "error";
	}
	
	IRResult genIR(BBlock *currentBlock) override {
		if (type != "VarDeclAssign") {
			return {"", currentBlock};
		}
		if (children.size() != 2) {
			throw std::logic_error("Initialized variable declaration must have a type and value");
		}

		auto initializer = children.begin();
		++initializer;
		IRResult value = (*initializer)->genIR(currentBlock);
		value.block->addTacInstructions(new AssignmentStmt(this->value, value.value));
		return {"", value.block, value.terminated};
	}
};

class Assignment : public Node {
public:
	Assignment(string t, string v, int l) : Node(t, v, l) {}
	~Assignment() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		duplicateIdentifier(st); 
		if (type == "Assign") {
			std::string ret = assignmentSemanticCheck(st);
			return ret;
		}
		return "error";
	}
	IRResult genIR(BBlock *currentBlock) override {
		if (children.size() != 2) {
			throw std::logic_error("Assignment must have a target and a value");
		}

		auto i = children.begin();
		IRResult target = (*i)->genIR(currentBlock);
		++i;
		IRResult value = (*i)->genIR(target.block);
		value.block->addTacInstructions(new AssignmentStmt(target.value, value.value));
		return {"", value.block, value.terminated};
	}
};

class Return : public Node {
public:
	Return(string t, string v, int l) : Node(t, v, l) {}
	~Return() {}
	
	IRResult genIR(BBlock *currentBlock) override {
		if (children.size() != 1) {
			throw std::logic_error("Return statement is missing its expression");
		}
		auto i = children.begin();
		IRResult returnValue = (*i)->genIR(currentBlock);
		returnValue.block->addTacInstructions(new ReturnStmt("return", returnValue.value));
		return {"", returnValue.block, true};
	}
};

class Print : public Node {
public:
	Print(string t, string v, int l) : Node(t, v, l) {}
	~Print() {}

	IRResult genIR(BBlock *currentBlock) override {
		if (children.size() != 1) {
			throw std::logic_error("Print statement is missing its expression");
		}
		auto i = children.begin();
		IRResult printValue = (*i)->genIR(currentBlock);
		printValue.block->addTacInstructions(new PrintStmt("print", printValue.value));
		return {"", printValue.block, printValue.terminated};
	}
};

class BreakStatement : public Node {
public:
	BreakStatement(string t, string v, int l) : Node(t, v, l) {}
	~BreakStatement() {}

	IRResult genIR(BBlock *currentBlock) override {
		if (loopBreakTarget == nullptr) {
			throw std::logic_error("Break statement is outside a loop");
		}
		currentBlock->addTacInstructions(new Jump(loopBreakTarget->getName()));
		currentBlock->setTrueExit(loopBreakTarget);
		return {"", currentBlock, true};
	}
};

class ContinueStatement : public Node {
public:
	ContinueStatement(string t, string v, int l) : Node(t, v, l) {}
	~ContinueStatement() {}

	IRResult genIR(BBlock *currentBlock) override {
		if (loopContinueTarget == nullptr) {
			throw std::logic_error("Continue statement is outside a loop");
		}
		currentBlock->addTacInstructions(new Jump(loopContinueTarget->getName()));
		currentBlock->setTrueExit(loopContinueTarget);
		return {"", currentBlock, true};
	}
};

class ForStatement : public Node {
public:
	ForStatement(string t, string v, int l) : Node(t, v, l) {}
	~ForStatement() {}

	IRResult genIR(BBlock *currentBlock) override {
		if (children.size() != 2) {
			throw std::logic_error("For statement must have a header and a body");
		}

		auto child = children.begin();
		Node* header = *child++;
		Node* body = *child;
		if (header->children.size() != 3) {
			throw std::logic_error("For header must contain init, condition, and update");
		}

		auto part = header->children.begin();
		Node* init = *part++;
		Node* condition = *part++;
		Node* update = *part;

		if (init->children.size() == 1 &&
			(init->children.front()->type == "VarDecl" ||
			 init->children.front()->type == "VarDeclAssign")) {
			IRResult initResult = init->children.front()->genIR(currentBlock);
			currentBlock = initResult.block;
		} else if (init->children.size() == 2) {
			auto expression = init->children.begin();
			IRResult target = (*expression)->genIR(currentBlock);
			++expression;
			IRResult value = (*expression)->genIR(target.block);
			value.block->addTacInstructions(new AssignmentStmt(target.value, value.value));
			currentBlock = value.block;
		}

		std::string conditionName = generateBlockName();
		std::string bodyName = generateBlockName();
		std::string updateName = generateBlockName();
		std::string exitName = generateBlockName();

		BBlock* conditionBlock = new BBlock();
		conditionBlock->setBBlockName(conditionName);
		BBlock* bodyBlock = new BBlock();
		bodyBlock->setBBlockName(bodyName);
		BBlock* updateBlock = new BBlock();
		updateBlock->setBBlockName(updateName);
		BBlock* exitBlock = new BBlock();
		exitBlock->setBBlockName(exitName);

		currentBlock->setTrueExit(conditionBlock);
		currentBlock->addTacInstructions(new Jump(conditionName));

		if (condition->children.empty()) {
			conditionBlock->addTacInstructions(new Jump(bodyName));
			conditionBlock->setTrueExit(bodyBlock);
		} else {
			IRResult conditionResult = condition->children.front()->genIR(conditionBlock);
			conditionResult.block->addTacInstructions(
				new CondJump("ifFalse", conditionResult.value, exitName));
			conditionResult.block->addTacInstructions(new Jump(bodyName));
			conditionResult.block->setTrueExit(bodyBlock);
			conditionResult.block->setFalseExit(exitBlock);
		}

		body->setLoopTargets(exitBlock, updateBlock);
		IRResult bodyResult = body->genIR(bodyBlock);
		if (!bodyResult.terminated) {
			bodyResult.block->addTacInstructions(new Jump(updateName));
			bodyResult.block->setTrueExit(updateBlock);
		}

		BBlock* updateResultBlock = updateBlock;
		if (update->children.size() == 2) {
			auto expression = update->children.begin();
			IRResult target = (*expression)->genIR(updateResultBlock);
			++expression;
			IRResult value = (*expression)->genIR(target.block);
			value.block->addTacInstructions(new AssignmentStmt(target.value, value.value));
			updateResultBlock = value.block;
		} else if (update->children.size() == 1) {
			updateResultBlock = update->children.front()->genIR(updateResultBlock).block;
		}

		updateResultBlock->addTacInstructions(new Jump(conditionName));
		updateResultBlock->setTrueExit(conditionBlock);
		return {"", exitBlock};
	}
};

class IfStmt : public Node {
private:
public:
	IfStmt(string t, string v, int l) : Node(t, v, l) {}
	~IfStmt() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		if (children.empty()) {
			std::cerr << "@error at line " << lineno << " - semantic (missing condition)" << std::endl;
			errCode = 4;
			return "error";
		}
		auto i = children.begin();
		std::string conditionType = (*i)->semanticAnalysis(st);

		if (conditionType == "error")
			return "error";
		
		if (conditionType != "BoolType" && conditionType != "True" && conditionType != "False") {
			std::cerr << "@error at line " << lineno << " - semantic (invalid operand type: got " << conditionType << ", expected boolean)" << std::endl;
			errCode = 4;
			return "error";
		}
		return conditionType;
	}

	IRResult genIR(BBlock *currentBlock) override {
		if (children.size() != 2) {
			throw std::logic_error("If statement must have a condition and body");
		}
		auto i = children.begin();
		IRResult condition = (*i)->genIR(currentBlock);
		i++;

		std::string trueName = generateBlockName();
		std::string joinName = generateBlockName();
		BBlock* trueBlock = new BBlock();
		trueBlock->setBBlockName(trueName);
		IRResult trueResult = (*i)->genIR(trueBlock);

		BBlock* joiningBlock = new BBlock();
		joiningBlock->setBBlockName(joinName);

		condition.block->addTacInstructions(new CondJump("ifFalse", condition.value, joinName));
		condition.block->addTacInstructions(new Jump(trueName));

		if (!trueResult.terminated) {
			trueResult.block->addTacInstructions(new Jump(joinName));
			trueResult.block->setTrueExit(joiningBlock);
		}

		condition.block->setTrueExit(trueBlock);
		condition.block->setFalseExit(joiningBlock);

		return {"", joiningBlock};
	}
};

class IfElseStmt : public Node {
private:
public:
	IfElseStmt(string t, string v, int l) : Node(t, v, l) {}
	~IfElseStmt() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		if (children.empty()) {
			std::cerr << "@error at line " << lineno << " - semantic (missing condition)" << std::endl;
			errCode = 4;
			return "error";
		}
		auto i = children.begin();
		std::string conditionType = (*i)->semanticAnalysis(st);

		if (conditionType == "error")
			return "error";
		
		if (conditionType != "BoolType" && conditionType != "True" && conditionType != "False") {
			std::cerr << "@error at line " << lineno << " - semantic (invalid operand type: got " << conditionType << ", expected boolean)" << std::endl;
			errCode = 4;
			return "error";
		}
		return conditionType;
	}

	IRResult genIR(BBlock *currentBlock) override {
		if (children.size() != 3) {
			throw std::logic_error("If-else statement must have a condition and two branches");
		}
		auto i = children.begin();
		IRResult condition = (*i)->genIR(currentBlock);
		++i;

		std::string trueName = generateBlockName();
		std::string falseName = generateBlockName();
		std::string joinName = generateBlockName();

		BBlock* trueBlock = new BBlock();
		trueBlock->setBBlockName(trueName);
		IRResult trueResult = (*i)->genIR(trueBlock);
		++i;

		BBlock* falseBlock = new BBlock();
		falseBlock->setBBlockName(falseName);
		IRResult falseResult = (*i)->genIR(falseBlock);

		BBlock* joiningBlock = new BBlock();
		joiningBlock->setBBlockName(joinName);

		condition.block->addTacInstructions(new CondJump("ifFalse", condition.value, falseName));
		condition.block->addTacInstructions(new Jump(trueName));

		if (!trueResult.terminated) {
			trueResult.block->addTacInstructions(new Jump(joinName));
			trueResult.block->setTrueExit(joiningBlock);
		}
		if (!falseResult.terminated) {
			falseResult.block->addTacInstructions(new Jump(joinName));
			falseResult.block->setTrueExit(joiningBlock);
		}

		condition.block->setTrueExit(trueBlock);
		condition.block->setFalseExit(falseBlock);

		return {"", joiningBlock, trueResult.terminated && falseResult.terminated};
	}
};	

class Read : public Node {
private:
public:
	Read(string t, string v, int l) : Node(t, v, l) {}
	~Read() {}
	IRResult genIR(BBlock *currentBlock) override {
		if (children.size() != 1) {
				throw std::logic_error("Read statement must have one target");
			}
			IRResult target = children.front()->genIR(currentBlock);
			target.block->addTacInstructions(new ReadStmt(target.value));
			return {"", target.block};
	}
};	

class MethodNode : public Node {
private:
public:
	MethodNode(string t, string v, int l) : Node(t, v, l) {}
	~MethodNode() {}
	std::string semanticAnalysis(SymbolTable& st) override {
		duplicateIdentifier(st);
		st.enterScope();

		Node* params = nullptr;
		for (auto child : children) {
			if (child->type == "Params") {
				params = child;
				break;
			}
		}

		if (params != nullptr) {
			for (auto currentNode = params->children.begin(); currentNode != params->children.end(); currentNode++) {
				auto param = *currentNode;
				if (param->type != "Param") continue;

				for (auto prevNode = params->children.begin(); prevNode != currentNode; prevNode++) {
					auto prev = *prevNode;

					if (prev->type == "Param" && prev->value == param->value) {
						std::cerr << "@error at line " << lineno << " - semantic (already declared parameter: " << param->value << ")" << std::endl;
						errCode = 4;
						break;
					}
				} 
			} 
		} 

		for (auto child : children) {
			if (child->type == "Params") continue;

			if (child->type == "Block")
				for (auto blockChild : child->children)
					blockChild->semanticAnalysis(st);
			else
				child->semanticAnalysis(st);
		}

		st.exitScope();
		return type;
	}
};

class MethodCallNode : public Node {
private:
public:
	MethodCallNode(string t, string v, int l) : Node(t, v, l) {}
	~MethodCallNode() {}
	std::string semanticAnalysis(SymbolTable& st) override {
		auto i = children.begin();
		
		Node* first = *i;
		i++;
		Node* second = *i;
		
		std::string firstType = first->semanticAnalysis(st);
		if (firstType == "error") return "error";
		Record* record = first->type == "FuncCall" ? st.lookup(first->value) : st.lookup(firstType);

		AClass* methodClass = dynamic_cast<AClass*>(record);
		if (methodClass == nullptr) {
			std::cerr << "@error at line " << lineno << " - semantic (recevier is not a class: " << first << ")" << std::endl;
			errCode = 4;
			return "error";
		}

		Method* method = methodClass->lookupMethod(second->value);
		if (method == nullptr) {
			std::cerr << "@error at line " << lineno << " - semantic (method does not exist in class: " << first << ")" << std::endl;
			errCode = 4;
			return "error";
		}

		// Checking arguments
		if (i != children.end())
			for (auto arg : (*i)->children)
				if (arg->semanticAnalysis(st) == "error") return "error";
		return method->getType();
	}
};

class AClassNode : public Node {
private:
public:
	AClassNode(string t, string v, int l) : Node(t, v, l) {}
	~AClassNode() {}	
	std::string semanticAnalysis(SymbolTable& st) override {
		duplicateIdentifier(st); 

		st.enterScope();
		for (auto child : children)
			child->semanticAnalysis(st);
		st.exitScope();
		return type;
	}
};

class ArrayAccess : public Node {
private:
public:
	ArrayAccess(string t, string v, int l) : Node(t, v, l) {}
	~ArrayAccess() {}
	std::string semanticAnalysis(SymbolTable& st) override {
		if (children.size() != 2) {
			std::cerr << "@error at line " << lineno << " - semantic (expected two operands)" << std::endl;
			errCode = 4;
			return "error";
		}
		auto i = children.begin();
		std::string arrayType = (*i)->semanticAnalysis(st);
		i++;
		std::string indexType = (*i)->semanticAnalysis(st);

		if (indexType != "IntType" && indexType != "IntLiteral") {
			std::cerr << "@error at line " << lineno << " - semantic (indexing must be an integer)" << std::endl;
			errCode = 4;
			return "error";
		}

		if (arrayType == "IntType[]")
			return "IntType";
		else if (arrayType == "FloatType[]")
			return "FloatType";
		else
			return "error";
	}
};

#endif