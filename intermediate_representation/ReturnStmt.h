#ifndef RETURNSTMT_H
#define RETURNSTMT_H

#include "Tac.h"
#include <string>

class ReturnStmt : public Tac {
private:
public:
    ReturnStmt(std::string _op, std::string _x) : Tac(_op, _x, "") {}
    ~ReturnStmt() override {}
    void dump() override {std::cout << this->getOp() << " " << this->getLhs() << std::endl;}
    std::string getTacString() override {return this->getOp() + " " + this->getLhs();}
};

#endif