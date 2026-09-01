#ifndef BBLOCK_H
#define BBLOCK_H

#include "Tac.h"
#include <vector>

class BBlock {
private:
    std::string name; //unique name
    std::vector<Tac*> tacInstructions;
    //Tac condition;
    BBlock *trueExit, *falseExit;
public:
    BBlock() : trueExit(nullptr), falseExit(nullptr) {}
    void addTacInstructions(Tac* tacIns) {tacInstructions.push_back(tacIns);}
    void printAllTacInstructions() {for (int i = 0; i < tacInstructions.size(); i++) tacInstructions[i]->dump();}
    void setTrueExit(BBlock* dest) {this->trueExit = dest;}
    void setFalseExit(BBlock* dest) {this->falseExit = dest;}
    // Whenever a new block is made through a jump, "BBlock *trueExit, *falseExit;" needs to point to that new block
    
};

#endif