#ifndef METHOD_H
#define METHOD_H

#include "Record.h"
#include "Variable.h"
#include <map>
#include <vector>
#include <string>

class Method : public Record {
private:
    std::vector<Variable*> parameters;
    std::map<std::string, Variable*> variables;
public:
    Method(std::string id, std::string type) : Record(id, type) {}
    ~Method() override {}
    void addParameter(Variable* parameter) {parameters.push_back(parameter);}
    void addVariable(std::string id, Variable* variable) {variables.insert({id, variable});}
    std::string getKind() const override {return "Method";}
    void printRecord() const override {
        std::cout << "id: " << this->getId() << " | Record: " << getKind() << " | type: " << this->getType() << std::endl;
    }
};

#endif