#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "Tac.h"
#include <string>
#include <iostream>

class Expression : public Tac {
private:
public:
    Expression(std::string _op, std::string _y, std::string _z, std::string _result) : Tac(_op, _y, _z, _result) {}
    Expression(std::string _op, std::string _operand, std::string _result)
        : Tac(_op, _operand, _result) {}
    ~Expression() override {}
    void dump() override { std::cout << getTacString() << std::endl; }
    std::string getTacString() override {
        if (getOp() == "newarray") {
            return getResult() + " := newarray " + getLhs() + " " + getRhs();
        }
        if (getRhs().empty()) {
            return getResult() + " := " + getOp() + " " + getLhs();
        }
        return getResult() + " := " + getLhs() + " " + getOp() + " " + getRhs();
    }
};

#endif