#ifndef CFG_H
#define CFG_H

#include "../Nodes/Node.h"
#include "./BBlock.h"
#include "./ParameterStmt.h"
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

class Cfg {
public:
    struct Function {
        std::string name;
        BBlock* entry;
    };

    const std::vector<Function>& getFunctions() const { return functions; }

    void generate(Node* ast) {
        if (ast == nullptr) {
            throw std::invalid_argument("Cannot generate IR from a null AST");
        }
        functions.clear();
        collectFunctions(ast, "");
        if (functions.empty()) {
            throw std::logic_error("The AST contains no main function or methods");
        }
    }

    void printAllTacInstructions() const {
        for (const Function& function : functions) {
            std::unordered_set<const BBlock*> visited;
            printBlock(function.entry, visited);
        }
    }

    void generateDot(const std::string& filename = "CFG.dot") const {
        std::ofstream outStream(filename);
        if (!outStream) {
            throw std::runtime_error("Could not open " + filename + " for writing");
        }

        outStream << "digraph CFG {\n";
        for (std::size_t i = 0; i < functions.size(); ++i) {
            const Function& function = functions[i];
            outStream << "  subgraph cluster_" << i << " {\n"
                      << "    label=\"" << BBlock::escapeDot(function.name) << "\";\n";
            std::unordered_set<const BBlock*> visited;
            function.entry->writeDot(outStream, visited);
            outStream << "  }\n";
        }        
        outStream << "}\n";

        std::cout << "Wrote control-flow graph to " << filename << '\n';
    }

private:
    std::vector<Function> functions;

    void collectFunctions(Node* node, const std::string& className) {
        if (node->type == "Main") {
            if (node->children.size() != 1) {
                throw std::logic_error("Main node must contain one body");
            }
            addFunction("main", node->children.front(), nullptr);
            return;
        }

        if (node->type == "Method") {
            if (node->children.size() != 3) {
                throw std::logic_error("Method node must contain parameters, return type, and body");
            }
            auto child = node->children.begin();
            Node* parameters = *child++;
            ++child;
            Node* body = *child;
            const std::string qualifiedName = className.empty()
                ? node->value : className + "." + node->value;
            addFunction(qualifiedName, body, parameters);
            return;
        }

        const std::string nestedClassName =
            node->type == "Class" ? node->value : className;
            
        for (Node* child : node->children) {
            collectFunctions(child, nestedClassName);
        }
    }

    void addFunction(const std::string& name, Node* body, Node* parameters) {
        BBlock* entry = new BBlock();

        std::string label;
        for (unsigned char character : name) {
            label += std::isalnum(character) || character == '_'
                ? static_cast<char>(character) : '_';
        }

        entry->setBBlockName((label.empty() ? "function" : label) + "_entry");
        if (parameters != nullptr) {
            for (Node* parameter : parameters->children) {
                entry->addTacInstructions(new ParameterStmt(parameter->value));
            }
        }
        body->genIR(entry);
        functions.push_back({name, entry});
    }

    static void printBlock(const BBlock* block,
                           std::unordered_set<const BBlock*>& visited) {
        if (block == nullptr || !visited.insert(block).second) return;
        std::cout << block->getName() << ":\n";
        block->printAllTacInstructions();
        printBlock(block->getTrueExit(), visited);
        printBlock(block->getFalseExit(), visited);
    }
};

#endif
