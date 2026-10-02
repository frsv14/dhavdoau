#ifndef READ_H
#define READ_H

#include "Tac.h"

#include <string>

class Read : public Tac {
public:
    Read(std::string value) : Tac("read", value, "", "") {}
    ~Read() override {}
    void dump() override { std::cout << "read " << this->getLhs() << std::endl; }
    std::string getTacString() override { return "read " + this->getLhs(); }
};



#endif // READ_H