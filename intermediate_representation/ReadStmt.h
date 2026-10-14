#ifndef READSTMT_H
#define READSTMT_H

#include "Tac.h"

class ReadStmt : public Tac {
public:
    explicit ReadStmt(const std::string& target) : Tac("read", target, "") {}

    void dump() override { std::cout << getTacString() << std::endl; }
    std::string getTacString() override { return "read " + getLhs(); }
};

#endif
