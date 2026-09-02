#ifndef METHODCALL_H
#define METHODCALL_H

#include "Tac.h"
#include <string>

class MethodCall : public Tac {
private:
public:
    MethodCall(std::string _f, std::string _N, std::string _result) : Tac("call", _f, _N, _result) {}
    ~MethodCall() override {}
    void dump() override {std::cout << this->getResult() << " := " << this->getOp() << " " << this->getLhs() << " " << this->getRhs() << std::endl;}
    std::string getTacString() override {return this->getResult() + " := " + this->getOp() + " " + this->getLhs() + " " + this->getRhs();}
};

#endif