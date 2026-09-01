#ifndef CLASS_H
#define CLASS_H

#include "Record.h"
#include "Variable.h"
#include "Method.h"

#include <map>
#include <string>

class AClass : public Record {
private:
    std::map<std::string, Variable*> variables;
    std::map<std::string, Method*> methods;
public:
    AClass(std::string id, std::string type) : Record(id, type) {}
    ~AClass() override {}
    void addVariable(std::string id, Variable* variable) {variables.insert({id, variable});}
    void addMethod(std::string id, Method* method) {methods.insert({id, method});}
    
    Variable* lookupVariable(std::string key) {
        auto it = variables.find(key);
        if(it != variables.end())
            return it->second;
        else
            return nullptr;
    }

    Method* lookupMethod(std::string key) {
        auto it = methods.find(key);
        if(it != methods.end())
            return it->second;
        else
            return nullptr;
    }
    std::string getKind() const override {return "Class";}
    void printRecord() const override {
        std::cout << "id: " << this->getId() << " | Record: " << getKind() << " | type: " << this->getType() << std::endl;
    }

    //FOR TESTING PURPOSES
    void printAllMethods() {
        printRecord();
        for (const auto pair : methods)
            std::cout << pair.first << std::endl;
    }
};

#endif