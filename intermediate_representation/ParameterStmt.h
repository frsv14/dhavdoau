#ifndef PARAMETERSTMT_H
#define PARAMETERSTMT_H

#include "Tac.h"

class ParameterStmt : public Tac {
public:
    ParameterStmt(const std::string& name) : Tac("param", name, "") {}

    void dump() override { std::cout << getTacString() << std::endl; }
    std::string getTacString() override { return "param " + getLhs(); }
};

#endif
