#ifndef CALLSTMT_H
#define CALLSTMT_H

#include "Tac.h"
#include <vector>

class CallStmt : public Tac {
public:
    CallStmt(const std::string& callee, const std::vector<std::string>& arguments,
             const std::string& result)
        : Tac("call", callee, joinArguments(arguments), result) {}

    void dump() override { std::cout << getTacString() << std::endl; }
    std::string getTacString() override {
        return getResult() + " := call " + getLhs() + "(" + getRhs() + ")";
    }

private:
    static std::string joinArguments(const std::vector<std::string>& arguments) {
        std::string joined;
        for (std::size_t i = 0; i < arguments.size(); ++i) {
            if (i != 0) joined += ", ";
            joined += arguments[i];
        }
        return joined;
    }
};

#endif
