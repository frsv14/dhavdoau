#ifndef PRINT_H
#define PRINT_H
#include "Tac.h"

#include <string>

class Print : public Tac {
public:
    Print(std::string value) : Tac("print", value, "", "") {}
    ~Print() override {}
    void dump() override { std::cout << "print " << this->getLhs() << std::endl; }
    std::string getTacString() override { return "print " + this->getLhs(); }
};

#endif