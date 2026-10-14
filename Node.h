#ifndef NODE_H
#define	NODE_H

#include <list>
#include <iostream>
#include <fstream>
#include <vector>
#ifdef __unix__
#include <unistd.h>
#include <sys/wait.h>
#endif

#include <random>
#include <string>
#include <stdexcept>

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
		if (type == "Read") {
			if (children.size() != 1) {
				throw std::logic_error("Read statement must have one target");
			}
			IRResult target = children.front()->genIR(currentBlock);
			target.block->addTacInstructions(new ReadStmt(target.value));
			return {"", target.block};
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
	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "-");
	}
};

class AddExpression : public Node {
private:
public:
	AddExpression(string t, string v, int l) : Node(t, v, l) {}
	~AddExpression() {}
	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "+");
	}
};

class DivExpression : public Node {
private:
public:
	DivExpression(string t, string v, int l) : Node(t, v, l) {}
	~DivExpression() {}
	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "/");
	}
};

class MulExpression : public Node {
private:
public:
	MulExpression(string t, string v, int l) : Node(t, v, l) {}
	~MulExpression() {}
	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "*");
	}
};

class PowExpression : public Node {
private:
public:
	PowExpression(string t, string v, int l) : Node(t, v, l) {}
	~PowExpression() {}
	IRResult genIR(BBlock *currentBlock) override {
		return emitBinary(currentBlock, "^");
	}
};

class ComparisonExpression : public Node {
public:
	ComparisonExpression(string t, int l) : Node(t, "", l) {}

	IRResult genIR(BBlock *currentBlock) override {
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
	IRResult genIR(BBlock *currentBlock) override {
		return {this->getValue(), currentBlock}; // return the name of the identifier
	}
};

class Integer : public Node {
private:
public:
	Integer(string t, string v, int l) : Node(t, v, l) {}
	~Integer() {}
	IRResult genIR(BBlock *currentBlock) override {
		return {this->getValue(), currentBlock}; // return the value of the integer
	}
};

class Float : public Node {
private:
public:
	Float(string t, string v, int l) : Node(t, v, l) {}
	~Float() {}
	IRResult genIR(BBlock *currentBlock) override {
		return {this->getValue(), currentBlock}; // return the value of the float
	}
};

class Boolean : public Node {
private:
public:
	Boolean(string t, string v, int l) : Node(t, v, l) {}
	~Boolean() {}
	IRResult genIR(BBlock *currentBlock) override {
		return {this->getValue(), currentBlock}; // return the value of the boolean
	}
};

class VariableDeclaration : public Node {
public:
	VariableDeclaration(string t, string v, int l) : Node(t, v, l) {}

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

#endif