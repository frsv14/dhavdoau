#ifndef BYTECODEGEN_H
#define BYTECODEGEN_H

#include "../intermediate_representation/Tac.h"
#include "../intermediate_representation/BBlock.h"
#include "../intermediate_representation/CFG.h"
#include "../Node.h"
#include <list>
#include <fstream>
#include <string>

class ByteCodeGen : public Node {
    private:

    static bool isNumber(const std::string& value) {
        //dena delen är ai för jag viste inte hur jag skulle göra
        char* end = nullptr;
        std::strtod(value.c_str(), &end);
        return end != value.c_str() && *end == '\0';
        //
    }

    static void writeValue(const std::string& value, std::ofstream& out) {
        if (!isNumber(value)) {
            out << "LOAD " << value << '\n';
        } else if (value.find_first_of(".eE") != std::string::npos) {
            out << "PUSH_FLOAT " << value << '\n';
        } else {
            out << "PUSH_INT " << value << '\n';
        }
    }

    static void writeTac(Tac* tac, std::ofstream& out) {
        const std::string op = tac->getOp();

        if (op == "assign") {
            writeValue(tac->getRhs(), out);
            out << "STORE " << tac->getLhs() << '\n';
        } else if (op == "print") {
            writeValue(tac->getLhs(), out);
            out << "PRINT\n";
        } else if (op == "return") {
            writeValue(tac->getLhs(), out);
            out << "RETURN\n";
        } else if (op == "+" || op == "-" || op == "*" || op == "/" || op == "^") {
            writeValue(tac->getLhs(), out);
            writeValue(tac->getRhs(), out);

            if (op == "+") out << "ADD\n";
            else if (op == "-") out << "SUB\n";
            else if (op == "*") out << "MUL\n";
            else if (op == "/") out << "DIV\n";
            else out << "POW\n";

            out << "STORE " << tac->getResult() << '\n';
        }
    }

    static void writeBlock(
        const BBlock* block,
        std::ofstream& out,
        std::unordered_set<const BBlock*>& visited
    ) {
        if (block == nullptr || !visited.insert(block).second) {
            return;
        }

        out << block->getName() << ":\n";

        for (Tac* tac : block->getInstructions()) {
            writeTac(tac, out);
        }

        writeBlock(block->getTrueExit(), out, visited);
        writeBlock(block->getFalseExit(), out, visited);
    }


    public:

    void generate(const Cfg& cfg, const std::string& filename) const {
        std::ofstream out(filename);
        if (!out) {
            throw std::runtime_error("Could not open " + filename);
        }

        for (const Cfg::Function& function : cfg.getFunctions()) {
            out << ".func " << function.name << '\n';

            std::unordered_set<const BBlock*> visited;
            writeBlock(function.entry, out, visited);

            out << ".end\n";
        }

        if (!out) {
            throw std::runtime_error("Could not write " + filename);
        }
    }


};


#endif