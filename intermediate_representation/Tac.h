#ifndef TAC_H
#define TAC_H

#include <iostream>

class Tac {
private:
    std::string op, lhs, rhs, result;
public:
    Tac(std::string _op, std::string _y, std::string _z, std::string _result) : op{_op}, lhs{_y}, rhs{_z}, result{_result} {}
    Tac(std::string _op, std::string _x, std::string _label) : op{_op}, lhs{_x}, result{_label} {}
    Tac(std::string _op, std::string _result) : op{"call"}, result{_result} {}
    virtual ~Tac() {}
    std::string getOp() {return this->op;}
    std::string getLhs() {return this->lhs;}
    std::string getRhs() {return this->rhs;}
    std::string getResult() {return this->result;}
    virtual void dump() = 0;
    virtual std::string getTacString() = 0;
};

#endif