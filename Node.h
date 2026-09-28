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
	virtual std::string genIR(BBlock *currentBlock) {
		for (auto i = children.begin(); i != children.end(); i++)
			(*i)->genIR(currentBlock);
		return "";
	}

// GenerateRandomString is temporary until a better solution is implemented.
string generateRandomString()
{		
	int length = 8; // Length of the random string
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

    return random_string;
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
	std::string genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock);
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock);
		Tac* in = new Expression("-", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);

		std::cout << "lhs_name: " << lhs_name << std::endl;
		std::cout << "rhs_name: " << rhs_name << std::endl;
		return name;
	}
};

class AddExpression : public Node {
private:
public:
	AddExpression(string t, string v, int l) : Node(t, v, l) {}
	~AddExpression() {}
	std::string genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock);
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock);
		Tac* in = new Expression("+", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);
		return name;
	}
};

class DivExpression : public Node {
private:
public:
	DivExpression(string t, string v, int l) : Node(t, v, l) {}
	~DivExpression() {}
	std::string genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock);
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock);
		Tac* in = new Expression("/", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);
		return name;
	}
};

class MulExpression : public Node {
private:
public:
	MulExpression(string t, string v, int l) : Node(t, v, l) {}
	~MulExpression() {}
	std::string genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock);
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock);
		Tac* in = new Expression("*", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);
		return name;
	}
};

class PowExpression : public Node {
private:
public:
	PowExpression(string t, string v, int l) : Node(t, v, l) {}
	~PowExpression() {}
	std::string genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		auto i = children.begin();
		std::string lhs_name = (*i)->genIR(currentBlock);
		i++;
		std::string rhs_name = (*i)->genIR(currentBlock);
		Tac* in = new Expression("^", lhs_name, rhs_name, name);
		currentBlock->addTacInstructions(in);
		return name;
	}
};

class Identifier : public Node {
private:
public:
	Identifier(string t, string v, int l) : Node(t, v, l) {}
	~Identifier() {}
	std::string genIR(BBlock *currentBlock) override {
		return this->getValue(); // return the name of the identifier
	}
};

class Integer : public Node {
private:
public:
	Integer(string t, string v, int l) : Node(t, v, l) {}
	~Integer() {}
	std::string genIR(BBlock *currentBlock) override {
		return this->getValue(); // return the value of the integer
	}
};

class Float : public Node {
private:
public:
	Float(string t, string v, int l) : Node(t, v, l) {}
	~Float() {}
	std::string genIR(BBlock *currentBlock) override {
		return this->getValue(); // return the value of the float
	}
};

class Boolean : public Node {
private:
public:
	Boolean(string t, string v, int l) : Node(t, v, l) {}
	~Boolean() {}
	std::string genIR(BBlock *currentBlock) override {
		return this->getValue(); // return the value of the boolean
	}
};

class IfStmt : public Node {
private:
public:
	IfStmt(string t, string v, int l) : Node(t, v, l) {}
	~IfStmt() {}
	std::string genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		
		// Code for IfStmt goes here

		return name;
	}
};

class IfElseStmt : public Node {
private:
public:
	IfElseStmt(string t, string v, int l) : Node(t, v, l) {}
	~IfElseStmt() {}
	std::string genIR(BBlock *currentBlock) override {
		std::string name = generateRandomString(); //generate a unique name
		
		// Code for IfElseStmt goes here
		
		return name;
	}
};

#endif