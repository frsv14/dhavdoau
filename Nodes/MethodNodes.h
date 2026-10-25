#ifndef METHODNODES_H
#define METHODNODES_H

#include "Node.h"

class MethodNode : public Node {
private:
public:
	MethodNode(string t, string v, int l) : Node(t, v, l) {}
	~MethodNode() {}

	void createTable(SymbolTable& st, AClass* currentClass = nullptr) override {
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

				Variable* param = new Variable(child->value, paramType, child->lineno);
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
	}

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
			std::cerr << "@error at line " << lineno << " - semantic (" << second->value << " does not exist)" << std::endl;
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

	void createTable(SymbolTable& st, AClass* currentClass = nullptr) override {
		AClass* aClass = new AClass(value, type);
		st.put(value, aClass);

		st.enterScope();
		for (auto child : children) 
			child->createTable(st, aClass);
		st.exitScope();
	}

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

class MainNode : public Node {
private:
public:
	MainNode(string t, string v, int l) : Node(t, v, l) {}
	~MainNode() {}

	void createTable(SymbolTable& st, AClass* currentClass = nullptr) override {
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

				Variable* param = new Variable(child->value, paramType, child->lineno);
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
};


#endif
