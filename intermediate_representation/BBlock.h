#ifndef BBLOCK_H
#define BBLOCK_H

#include "Tac.h"
#include <vector>
#include <fstream>

class BBlock {
private:
    std::string name; //unique name
    std::vector<Tac*> tacInstructions;
    //Tac condition;
    BBlock *trueExit;
    BBlock* falseExit;
public:
    BBlock() : trueExit(nullptr), falseExit(nullptr) {}
    void addTacInstructions(Tac* tacIns) {tacInstructions.push_back(tacIns);}
    void printAllTacInstructions() {for (int i = 0; i < tacInstructions.size(); i++) tacInstructions[i]->dump();}
    void setTrueExit(BBlock* dest) {this->trueExit = dest;}
    void setFalseExit(BBlock* dest) {this->falseExit = dest;}
    void setBBlockName(std::string name) {this->name = name;}
    // Whenever a new block is made through a jump, "BBlock *trueExit, *falseExit;" needs to point to that new block
    std::string getName() {return this->name;}
    
	void generate_tree() {
		std::ofstream outStream;
		char* filename = "CFG.dot";
	  	outStream.open(filename);

		outStream << "digraph {" << std::endl;
		generate_tree_content(&outStream);
		outStream << "}" << std::endl;
		outStream.close();

		printf("\nBuilt a tac-tree at %s. Use 'make tree' to generate the pdf version.\n", filename);
  	}

  	void generate_tree_content(std::ofstream *outStream) {
	  *outStream << name << getTacStrings() << std::endl;

      if (trueExit != nullptr) {
        trueExit->generate_tree_content(outStream);
        *outStream << name << " -> " << trueExit->getName() << std::endl;
      }
      if (falseExit != nullptr) {
        falseExit->generate_tree_content(outStream);
        *outStream << name << " -> " << falseExit->getName() << std::endl;
      }
    }

    std::string getTacStrings() {
        std::string label = " [label=\"";

        for (int i = 0; i < tacInstructions.size(); i++) {
            auto* x = tacInstructions[i];
            label += x->getTacString() + " \n ";
	    }
        label += "\", shape=box];";
        return label;
    }
};

#endif