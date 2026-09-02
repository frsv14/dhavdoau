#ifndef CONDJUMP_H
#define CONDJUMP_H

#include "Tac.h"
#include <string>

class CondJump : public Tac {
private:
public:
    CondJump(std::string _op, std::string _x, std::string _label) : Tac(_op, _x, _label) {}
    ~CondJump() override {}
    void dump() override {std::cout << this->getOp() << " " << this->getLhs() << " goto " << this->getRhs() << std::endl;}
    std::string getTacString() override {return this->getOp() + " " + this->getLhs() + " goto " + this->getRhs();}
};

#endif