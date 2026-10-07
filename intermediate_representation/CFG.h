#ifndef CFG_H
#define CFG_H

#include "../Node.h"
#include "./BBlock.h"
#include "./Jump.h"
#include "./CondJump.h"


class Cfg {
private:
    int currentBlockNum = 1;
public:
    void createCFG(Node* node, BBlock* currentBlock) {
        if (!node) return;

        if (node->type == "VarDecl" || node->type == "VarDeclAssign") {
            if (node->type == "VarDeclAssign") {
                for (auto i = std::next(node->children.begin()); i != node->children.end(); i++) {
                    createCFG(*i, currentBlock);
                }
            }
        }

        if (node->type == "If") {
            Node* nodeType = node->children.empty() ? nullptr : node->children.front();
            
            BBlock* bblockTrue = new BBlock();
            std::string blockTrueName = "Block_" + std::to_string(currentBlockNum + 1);
            bblockTrue->setBBlockName(blockTrueName);
            currentBlockNum++;

            CondJump* condJump = new CondJump("iffalse", nodeType->value, blockTrueName);
            currentBlock->addTacInstructions(condJump);
            currentBlock->setTrueExit(bblockTrue);

            Jump* jumpToEnd = new Jump("Block_" + std::to_string(currentBlockNum));
            bblockTrue->addTacInstructions(jumpToEnd);
        }

        if (node->type == "IfElse") {
            Node* nodeType = node->children.empty() ? nullptr : node->children.front();
            
            BBlock* bblockTrue = new BBlock();
            std::string blockTrueName = "Block_" + std::to_string(currentBlockNum + 1);
            bblockTrue->setBBlockName(blockTrueName);
            currentBlockNum++;

            BBlock* bblockFalse = new BBlock();
            std::string blockFalseName = "Block_" + std::to_string(currentBlockNum + 1);
            bblockFalse->setBBlockName(blockFalseName);
            currentBlockNum++;

            CondJump* condJump = new CondJump("iffalse", nodeType->value, blockFalseName);
            currentBlock->addTacInstructions(condJump);
            currentBlock->setTrueExit(bblockTrue);
            currentBlock->setFalseExit(bblockFalse);
            
            Jump* jumpToEnd = new Jump("Block_" + std::to_string(currentBlockNum));
            bblockTrue->addTacInstructions(jumpToEnd);
            bblockFalse->addTacInstructions(jumpToEnd);
        }
        

        for (auto child : node->children) 
            createCFG(child, currentBlock);
    }
};

#endif