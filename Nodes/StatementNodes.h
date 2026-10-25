#ifndef STATEMENTNODES_H
#define STATEMENTNODES_H

#include "Node.h"

class VariableDeclaration : public Node {
public:
	VariableDeclaration(string t, string v, int l) : Node(t, v, l) {}
	~VariableDeclaration() {}

	std::string semanticAnalysis(SymbolTable& st) override {
		duplicateIdentifier(st); 

		Node* firstChild = children.front();
		if (firstChild->type == "TypeID" || firstChild->type == "Identifier") {
			Record* record = st.getCurrentScope()->lookup(firstChild->value);

			if (record == nullptr || record->getKind() != "Class") {
				std::cerr << "@error - semantic (" << firstChild->value << " is undefined)" << std::endl;
				errCode = 4;
				return "error";
			}
		}

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
			std::cerr << "@error - semantic (missing condition)" << std::endl;
			errCode = 4;
			return "error";
		}
		auto i = children.begin();
		std::string conditionType = (*i)->semanticAnalysis(st);

		if (conditionType == "error")
			return "error";
		
		if (conditionType != "BoolType" && conditionType != "True" && conditionType != "False") {
			std::cerr << "@error - semantic (invalid operand type: got " << conditionType << ", expected boolean)" << std::endl;
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
			std::cerr << "@error - semantic (missing condition)" << std::endl;
			errCode = 4;
			return "error";
		}
		auto i = children.begin();
		std::string conditionType = (*i)->semanticAnalysis(st);

		if (conditionType == "error")
			return "error";
		
		if (conditionType != "BoolType" && conditionType != "True" && conditionType != "False") {
			std::cerr << "@error - semantic (invalid operand type: got " << conditionType << ", expected boolean)" << std::endl;
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

#endif
