#ifndef PPRINTSTMT_H
#define PPRINTSTMT_H

#include "Tac.h"
#include <string>

class PrintStmt : public Tac {
private:
public:
    PrintStmt(std::string _op, std::string _x) : Tac(_op, _x, "") {}
    ~PrintStmt() override {}
    void dump() override {std::cout << this->getOp() << " " << this->getLhs() << std::endl;}
    std::string getTacString() override {return this->getOp() + " " + this->getLhs();}
};
#endif