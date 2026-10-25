#ifndef RECORD_H
#define RECORD_H

#include <iostream>
#include <string>

class Record {
private:
    std::string id; // Variable name
    std::string type; // Data type ex. int, string
    int lineNr;
public:
    Record(std::string id, std::string type, int lineNr = 0) : id(id), type(type), lineNr(lineNr) {}
    virtual ~Record() {}
    std::string getId() const {return this->id;}
    std::string getType() const {return this->type;}
    int getLineNr() const {return this->lineNr;}
    virtual std::string getKind() const = 0;
    virtual void printRecord() const = 0;
};

#endif