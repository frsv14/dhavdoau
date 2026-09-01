#ifndef VARIABLE_H
#define VARIABLE_H

#include "Record.h"
#include <string>

class Variable : public Record {
private:
public:
    Variable(std::string id, std::string type) : Record(id, type) {}
    ~Variable() override {}
    std::string getKind() const override {return "Variable";}
    void printRecord() const override {
        std::cout << "id: " << this->getId() << " | Record: " << getKind() << " | type: " << this->getType() << std::endl;
    }
};

#endif