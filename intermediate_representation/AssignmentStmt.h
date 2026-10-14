#ifndef ASSIGNMENTSTMT_H
#define ASSIGNMENTSTMT_H

#include "Tac.h"
#include <string>

class AssignmentStmt : public Tac {
public:
    AssignmentStmt(std::string target, std::string value)
        : Tac("assign", target, value, "") {}

    void dump() override {
        std::cout << getLhs() << " := " << getRhs() << std::endl;
    }

    std::string getTacString() override {
        return getLhs() + " := " + getRhs();
    }
};

#endif
