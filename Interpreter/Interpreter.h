#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

class Interpreter {
private: 
    struct heapNode {
        std::string name = "";
        int Ivalue;
    };

    std::list<int> stack;
    std::list<heapNode*> heap;

    void push_int (int value){
        stack.push_front(value);
    }

    void push_float (int value){
        stack.push_front(value);
    }

    void store (std::string name){
        heap.push_front(new heapNode{name, stack.front()});
        stack.pop_front();
    }

    void load (std::string name){
        for (heapNode* node : heap) {
            if (node->name == name) {
                stack.push_front(node->Ivalue);
                return;
            }
        }
    }
    
    void mul (){
        int a = stack.front();
        stack.pop_front();
        int b = stack.front();
        stack.pop_front();
        stack.push_front(b*a);
    }

    void powto () {
        int a = stack.front();
        stack.pop_front();
        int b = stack.front();
        stack.pop_front();
        int result = pow(b, a);
        stack.push_front(result);
    }

    void add (){
        int a = stack.front();
        stack.pop_front();
        int b = stack.front();
        stack.pop_front();
        stack.push_front(b + a);
    }

    void sub (){
        int a = stack.front();
        stack.pop_front();
        int b = stack.front();
        stack.pop_front();
        stack.push_front(b-a);
    }
        
    void div (){
        int a = stack.front();
        stack.pop_front();
        int b = stack.front();
        stack.pop_front();
        stack.push_front(b/a);
    }
    
    void print (){
        int a = stack.front();
        std::cout << a << std::endl;
    }

public:

    void interpret(const std::string& filename) {
        std::ifstream in(filename);
        if (!in) {
            throw std::runtime_error("Could not open " + filename);
        }

        std::string line;
        while (std::getline(in, line)) {
            std::istringstream iss(line);
            std::string instruction;
            iss >> instruction;

            if (instruction == "PUSH_INT") {
                int value;
                iss >> value;
                push_int(value);
            } else if (instruction == "PUSH_FLOAT") {
                int value;
                iss >> value;
                push_float(value);
            }
            else if (instruction == "STORE") {
                std::string name;
                iss >> name;
                store(name);
            } else if (instruction == "LOAD") {
                std::string name;
                iss >> name;
                load(name);
            } else if (instruction == "ADD") {
                add();
            } else if (instruction == "SUB") {
                sub();
            } else if (instruction == "MUL") {
                mul();
            } else if (instruction == "DIV") {
                div();
            } else if (instruction == "POW") {
                powto();
            } else if (instruction == "PRINT") {
                print();
            }
        }
    }

};

#endif