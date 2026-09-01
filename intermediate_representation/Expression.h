#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "Tac.h"
#include <iostream>

class Expression : public Tac {
private:
public:
    Expression(std::string _op, std::string _y, std::string _z, std::string _result) : Tac(_op, _y, _z, _result) {}
    ~Expression() override {}
    void dump() override {std::cout << this->getResult() << " := " << this->getLhs() << " " << this->getOp() << " " << this->getRhs() << std::endl;}
};

#endif