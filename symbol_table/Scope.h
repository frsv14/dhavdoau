#ifndef SCOPE_H
#define SCOPE_H

#include "Record.h"

#include <map>
#include <string>
#include <vector>
#include <iostream>

class Scope {
private:
    int next = 0;
    Scope* parentScope;
    std::vector<Scope*> childrenScopes;
    std::multimap<std::string, Record*> records;
public:
    Scope(Scope* parentScope) : parentScope(parentScope) {}

    Scope* nextChild() {
        Scope* nextChild;

        if(next == childrenScopes.size()) {
            nextChild = new Scope(this);
            childrenScopes.push_back(nextChild);
        } 
        else {
            nextChild = childrenScopes[next];
        }
        next++;
        return nextChild;
    }

    Record* lookup(std::string key) {
        auto it = records.find(key);
        if(it != records.end()) {
            return it->second;
        }
        else {
            if(parentScope == nullptr) {
                return nullptr;
            }
            else {
                return parentScope->lookup(key);
            }
        }
    }

    void resetScope() {
        next = 0;
        for(auto child : childrenScopes) {
            child->resetScope();
        }
    }

    void printTable() {
        std::cout << "Scope" << std::endl;
        for(auto& entry : records) {
            Record* record = entry.second;
            record->printRecord();
        }
        
        for(auto child : childrenScopes) {
            child->printTable();
        }
    }

    /////////////////////////////////////////////////
    const std::multimap<std::string, Record*>& getRecords() const { return records; }
    const std::vector<Scope*>& getChildrenScopes() const { return childrenScopes; }
    ////////////////////////////////////////////////

    Scope* getParentScope() const {return this->parentScope;}
    void setRecords(std::string key, Record* item) {records.insert({key, item});}
};

#endif