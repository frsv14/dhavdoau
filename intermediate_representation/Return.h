#ifndef RETURN_H
#define RETURN_H
#include "Tac.h"

#include <string>

class Return : public Tac {
public:
    Return(std::string value) : Tac("return", value, "", "") {}
    ~Return() override {}
    void dump() override { std::cout << "return " << this->getLhs() << std::endl; }
    std::string getTacString() override { return "return " + this->getLhs(); }
};

#endif