#ifndef BBLOCK_H
#define BBLOCK_H

#include "Tac.h"
#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

class BBlock {
private:
    std::string name; //unique name
    std::vector<Tac*> tacInstructions;
    BBlock *trueExit;
    BBlock* falseExit;
public:
    BBlock() : trueExit(nullptr), falseExit(nullptr) {}
    void addTacInstructions(Tac* tacIns) {tacInstructions.push_back(tacIns);}
    void printAllTacInstructions() const {
        for (Tac* instruction : tacInstructions) instruction->dump();
    }
    void setTrueExit(BBlock* dest) {this->trueExit = dest;}
    void setFalseExit(BBlock* dest) {this->falseExit = dest;}
    void setBBlockName(std::string name) {this->name = name;}
    const std::string& getName() const {return this->name;}
    BBlock* getTrueExit() const {return trueExit;}
    BBlock* getFalseExit() const {return falseExit;}
    const std::vector<Tac*>& getInstructions() const {return tacInstructions;}
    
    void generate_tree() const {
        std::ofstream outStream("CFG.dot");
        if (!outStream) {
            throw std::runtime_error("Could not open CFG.dot for writing");
        }

        outStream << "digraph CFG {\n";
        std::unordered_set<const BBlock*> visited;
        writeDot(outStream, visited);
        outStream << "}\n";

    }

    void writeEdge(std::ostream& outStream, const BBlock* destination) const {
        if (destination != nullptr) { 
                outStream << '"' << escapeDot(name) << "\" -> \"" << escapeDot(destination->getName()) << "\";\n";
        }
    }

    void writeDot(std::ostream& outStream,
                  std::unordered_set<const BBlock*>& visited) const {

        if (!visited.insert(this).second) return; 
        
        outStream << '"' << escapeDot(name) << "\" [shape=box, label=\"" << escapeDot(getTacStrings()) << "\"];\n";

        writeEdge(outStream, trueExit);
        writeEdge(outStream, falseExit);

        if (trueExit != nullptr) trueExit->writeDot(outStream, visited);
        if (falseExit != nullptr) falseExit->writeDot(outStream, visited);
    }

    std::string getTacStrings() const {
        std::string label = name;
        for (Tac* instruction : tacInstructions) {
            label += "\n" + instruction->getTacString();
        }
        return label;
    }

    static std::string escapeDot(const std::string& value) {
        std::string escaped;
        for (char character : value) {
            if (character == '\\' || character == '"') escaped += '\\';
            
            if (character == '\n') {
                escaped += "\\n";
            } else if (character != '\r') {
                escaped += character;
            }
        }
        return escaped;
    }
};

#endif