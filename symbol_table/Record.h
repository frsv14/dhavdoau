#ifndef RECORD_H
#define RECORD_H

#include <iostream>
#include <string>

class Record {
private:
    std::string id; // Variable name
    std::string type; // Data type ex. int, string
public:
    Record(std::string id, std::string type) : id(id), type(type) {}
    virtual ~Record() {}
    std::string getId() const {return this->id;}
    std::string getType() const {return this->type;}
    virtual std::string getKind() const = 0;
    virtual void printRecord() const = 0;
};

#endif