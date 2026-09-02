#ifndef JUMP_H
#define JUMP_H

#include "Tac.h"
#include <string>

class Jump : public Tac {
private:
public:
    Jump(std::string _label) : Tac("goto", _label) {}
    ~Jump() override {}
    void dump() override {std::cout << this->getOp() << " " << this->getResult() << std::endl;}
    std::string getTacString() override {return this->getOp() + " " + this->getResult();}
};

#endif