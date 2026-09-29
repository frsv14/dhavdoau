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

#include "./intermediate_representation/BBlock.h"
#include "./intermediate_representation/Tac.h"
#include "./intermediate_representation/Expression.h"

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
          "wxyz0123456789";

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

    return "Block_" + random_string;
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
		std::string name = generateRandomString(); //generate a unique name

		// code goes here
		
		return {"", currentBlock};
	}
};

class IfElseStmt : public Node {
private:
public:
	IfElseStmt(string t, string v, int l) : Node(t, v, l) {}
	~IfElseStmt() {}
	IRResult genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		std::string nameTrue = generateRandomString(); //generate a unique name
		std::string nameFalse = generateRandomString(); //generate a unique name

		// genIR for the boolean condition
		auto i = children.begin();
		(*i)->genIR(currentBlock);
		i++;

		// genIR true branch
		BBlock* trueBlock = new BBlock();
		trueBlock->setBBlockName(nameTrue);
		(*i)->genIR(trueBlock);
		i++;

		// genIR false branch
		BBlock* falseBlock = new BBlock();
		falseBlock->setBBlockName(nameFalse);
		(*i)->genIR(falseBlock);
		i++;

		// Joining block for true and false exit
		BBlock* joiningBlock = new BBlock();
		joiningBlock->setBBlockName(name);

		trueBlock->setTrueExit(joiningBlock);
		falseBlock->setTrueExit(joiningBlock);
		currentBlock->setTrueExit(trueBlock);
		currentBlock->setFalseExit(falseBlock);

		return {"", joiningBlock};
	}
};

#endif