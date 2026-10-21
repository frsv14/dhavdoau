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
    enum class ValueType {
        INT,
        FLOAT
    };

    struct Value {
        Value() {NULL;};

        ValueType type;

        union {
            int i;
            float f;
            bool isFloat;
        };

        Value(int value) : type(ValueType::INT), i(value) {}
        Value(float value) : type(ValueType::FLOAT), f(value) {}

        Value operator-(const Value& other) const {
            if (type == ValueType::FLOAT ||
                other.type == ValueType::FLOAT) {
                float a = (type == ValueType::FLOAT) ? f : i;
                float b = (other.type == ValueType::FLOAT) ? other.f : other.i;
                return Value(a - b);
            }

            return Value(i - other.i);
        }

        Value operator+(const Value& other) const {
            if (type == ValueType::FLOAT ||
                other.type == ValueType::FLOAT) {
                float a = (type == ValueType::FLOAT) ? f : i;
                float b = (other.type == ValueType::FLOAT) ? other.f : other.i;
                return Value(a + b);
            }

            return Value(i + other.i);
        }

        Value operator*(const Value& other) const {
            if (type == ValueType::FLOAT ||
                other.type == ValueType::FLOAT) {
                float a = (type == ValueType::FLOAT) ? f : i;
                float b = (other.type == ValueType::FLOAT) ? other.f : other.i;
                return Value(a * b);
            }

            return Value(i * other.i);
        }

        Value operator/(const Value& other) const {
            if (type == ValueType::FLOAT ||
                other.type == ValueType::FLOAT) {
                float a = (type == ValueType::FLOAT) ? f : i;
                float b = (other.type == ValueType::FLOAT) ? other.f : other.i;
                return Value(a / b);
            }

            return Value(i / other.i);
        }

    };

    struct heapNode {
        std::string name = "";
        Value IFvalue;
    };

    std::list<Value> stack;
    std::list<heapNode*> heap;

    void push_int (int value){
        stack.push_front(Value(value));
    }

    void push_float (float value){
        stack.push_front(Value(value));
    }

    void store (std::string name){
        heap.push_front(new heapNode{name, stack.front()});
        stack.pop_front();
    }

    void load (std::string name){
        for (heapNode* node : heap) {
            if (node->name == name) {
                stack.push_front(node->IFvalue);
                return;
            }
        }
    }
    
    void mul (){
        Value a = stack.front();
        stack.pop_front();
        Value b = stack.front();
        stack.pop_front();
        stack.push_front(b*a);
    }

    void powto () {
        Value exponent = stack.front();
        stack.pop_front();

        Value base = stack.front();
        stack.pop_front();

        float a = (exponent.type == ValueType::FLOAT) ? exponent.f : exponent.i;

        float b = (base.type == ValueType::FLOAT) ? base.f : base.i;

        stack.push_front(Value(std::pow(b, a)));
    }

    void add (){
        Value a = stack.front();
        stack.pop_front();
        Value b = stack.front();
        stack.pop_front();
        stack.push_front(b + a);
    }

    void sub (){
        Value a = stack.front();
        stack.pop_front();
        Value b = stack.front();
        stack.pop_front();
        stack.push_front(b - a);
    }
        
    void div (){
        Value a = stack.front();
        stack.pop_front();
        Value b = stack.front();
        stack.pop_front();
        stack.push_front(b / a);
    }
    
    void print (){
        Value a = stack.front();
        if (a.type == ValueType::FLOAT) {
            std::cout << a.f << std::endl;
        } else {
            std::cout << a.i << std::endl;
        }
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
                float value;
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