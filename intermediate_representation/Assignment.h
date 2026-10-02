#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include "Tac.h"

#include <string>

class Assignment : public Tac {
public:
    Assignment(std::string target, std::string value) : Tac("=", value, "", target) {}
    ~Assignment() override {}
    void dump() override { std::cout << this->getResult() << " := " << this->getLhs() << std::endl; }
    std::string getTacString() override { return this->getResult() + " := " + this->getLhs(); }
};

#endif