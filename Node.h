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
#include "./intermediate_representation/CondJump.h"
#include "./intermediate_representation/Jump.h"


using namespace std;

struct IRResult {
	std::string value;
	BBlock* block;
};

class Node {
public:	
	int id, lineno;
	string type, value;
	list<Node*> children;
	Node(string t, string v, int l) : type(t), value(v), lineno(l){}
	Node()
	{
		type = "uninitialised";
		value = "uninitialised"; 
	}   // Bison needs this.
	std::string getValue() { return value; }
	virtual IRResult genIR(BBlock *currentBlock) {
		for (auto i = children.begin(); i != children.end(); i++) {
			IRResult result = (*i)->genIR(currentBlock);
			currentBlock = result.block;
		}
		return {"", currentBlock};
	}

// GenerateRandomString is temporary until a better solution is implemented.
string generateRandomString()
{		
	int length = 2; // Length of the random string
    // Define the list of possible characters
    const string CHARACTERS
        = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuv"
          "wxyz";

    // Create a random number generator
    random_device rd;
    mt19937 generator(rd());

    // Create a distribution to uniformly select from all
    // characters
    uniform_int_distribution<> distribution(
        0, CHARACTERS.size() - 1);

    // Generate the random string
    string random_string;
    for (int i = 0; i < length; ++i) {
        random_string
            += CHARACTERS[distribution(generator)];
    }

    return random_string;
}
	
string generateString(IRResult result){
	
	return string();
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
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock).value;
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock).value;
		Tac* in = new Expression("-", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);

		std::cout << "lhs_name: " << lhs_name << std::endl;
		std::cout << "rhs_name: " << rhs_name << std::endl;
		return {name, currentBlock};
	}
};

class AddExpression : public Node {
private:
public:
	AddExpression(string t, string v, int l) : Node(t, v, l) {}
	~AddExpression() {}
	IRResult genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock).value;
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock).value;
		Tac* in = new Expression("+", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);
		return {name, currentBlock};
	}
};

class DivExpression : public Node {
private:
public:
	DivExpression(string t, string v, int l) : Node(t, v, l) {}
	~DivExpression() {}
	IRResult genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock).value;
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock).value;
		Tac* in = new Expression("/", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);
		return {name, currentBlock};
	}
};

class MulExpression : public Node {
private:
public:
	MulExpression(string t, string v, int l) : Node(t, v, l) {}
	~MulExpression() {}
	IRResult genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock).value;
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock).value;
		Tac* in = new Expression("*", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);
		return {name, currentBlock};
	}
};

class PowExpression : public Node {
private:
public:
	PowExpression(string t, string v, int l) : Node(t, v, l) {}
	~PowExpression() {}
	IRResult genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock).value;
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock).value;
		Tac* in = new Expression("^", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);

		return {name, currentBlock};
	}
};

class ComparisonExpression : public Node {
public:
	ComparisonExpression(string t, int l) : Node(t, "", l) {}

	IRResult genIR(BBlock *currentBlock) override {
		if (children.size() != 2) {
			throw std::logic_error("Comparison expression must have two operands");
		}

		auto i = children.begin();
		IRResult lhs = (*i)->genIR(currentBlock);
		++i;
		IRResult rhs = (*i)->genIR(lhs.block);

		std::string op;
		if (type == "Eq") op = "==";
		else if (type == "Neq") op = "!=";
		else if (type == "Lt") op = "<";
		else if (type == "Gt") op = ">";
		else if (type == "Lte") op = "<=";
		else if (type == "Gte") op = ">=";
		else throw std::logic_error("Unsupported comparison operator: " + type);

		std::string result = generateRandomString();
		rhs.block->addTacInstructions(new Expression(op, lhs.value, rhs.value, result));
		return {result, rhs.block};
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

class IfStmt : public Node {
private:
public:
	IfStmt(string t, string v, int l) : Node(t, v, l) {}
	~IfStmt() {}
	IRResult genIR(BBlock *currentBlock) override {
		auto i = children.begin();
		IRResult condition = (*i)->genIR(currentBlock);
		i++;

		std::string trueName = generateRandomString();
		std::string joinName = generateRandomString();
		BBlock* trueBlock = new BBlock();
		trueBlock->setBBlockName(trueName);
		IRResult trueResult = (*i)->genIR(trueBlock);

		BBlock* joiningBlock = new BBlock();
		joiningBlock->setBBlockName(joinName);

		condition.block->addTacInstructions(new CondJump("ifFalse", condition.value, joinName));
		condition.block->addTacInstructions(new Jump(trueName));

		trueResult.block->addTacInstructions(new Jump(joinName));
		trueResult.block->setTrueExit(joiningBlock);

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
		auto i = children.begin();
		IRResult condition = (*i)->genIR(currentBlock);
		++i;

		std::string trueName = generateRandomString();
		std::string falseName = generateRandomString();
		std::string joinName = generateRandomString();

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

		trueResult.block->addTacInstructions(new Jump(joinName));
		trueResult.block->setTrueExit(joiningBlock);
		falseResult.block->addTacInstructions(new Jump(joinName));
		falseResult.block->setTrueExit(joiningBlock);

		condition.block->setTrueExit(trueBlock);
		condition.block->setFalseExit(falseBlock);

		return {"", joiningBlock};
	}
};	

#endif