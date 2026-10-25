#ifndef EXPRESSIONNODES_H
#define EXPRESSIONNODES_H

#include "Node.h"

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

		if (record->getKind() == "Variable" && lineno < record->getLineNr()) {
			std::cerr << "@error at line " << lineno << " - semantic (" << value << " is not defined yet)" << std::endl;
			errCode = 4;
			return "error"; 
		}

		return record->getType(); 
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

#endif
