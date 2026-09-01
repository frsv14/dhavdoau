#include <iostream>
#include "parser.tab.hh"

#include "symbol_table/SymbolTable.h"
//#include "symbol_table/Record.h"
//#include "symbol_table/Variable.h"
//#include "symbol_table/Method.h"
//#include "symbol_table/Class.h"
#include "semantic_analysis/SemanticAnalysis.h"
#include "intermediate_representation/Tac.h"
#include "intermediate_representation/Expression.h"
#include "intermediate_representation/Jump.h"
#include "intermediate_representation/MethodCall.h"
#include "intermediate_representation/CondJump.h"
#include "intermediate_representation/BBlock.h"

extern Node *root;
extern FILE *yyin;
extern int yylineno;
extern int lexical_errors;
extern yy::parser::symbol_type yylex();

enum errCodes
{
    SUCCESS = 0,
    LEXICAL_ERROR = 1,
    SYNTAX_ERROR = 2,
    AST_ERROR = 3,
    SEMANTIC_ERROR = 4,
    SEGMENTATION_FAULT = 139
};

int errCode = errCodes::SUCCESS;

// Handling Syntax Errors
void yy::parser::error(std::string const &err)
{
    if (!lexical_errors)
    {
        std::cerr << "Syntax errors found! See the logs below:" << std::endl;
        std::cerr << "\t@error at line " << yylineno << ". Cannot generate a syntax for this input:" << err.c_str() << std::endl;
        std::cerr << "End of syntax errors!" << std::endl;
        errCode = errCodes::SYNTAX_ERROR;
    }
}

int main(int argc, char **argv)
{
    // Reads from file if a file name is passed as an argument. Otherwise, reads from stdin.
	if (argc > 1)
    {
        if (!(yyin = fopen(argv[1], "r")))
        {
            perror(argv[1]);
            return 1;
        }
    }
    //
    if (USE_LEX_ONLY)
        yylex();
    else
    {
        yy::parser parser;

        bool parseSuccess = !parser.parse();

        if (lexical_errors)
            errCode = errCodes::LEXICAL_ERROR;

        if (parseSuccess && !lexical_errors)
        {
            printf("\nThe compiler successfuly generated a syntax tree for the given input! \n");

            printf("\nPrint Tree:  \n");
            try
            {
                root->print_tree();
                root->generate_tree();

                //test
                std::cout << "\n" << "----- SYMBOLTABLE START -----" << "\n" << std::endl;
                SymbolTable st;
                st.createTable(root);
                /*
                st.put("test1", new Variable("test", "string"));
                st.put("test2", new Variable("num", "int"));
                st.put("testMethod", new Method("testMethod", "void"));
                st.put("testClass", new AClass("TestClass", "TestClass"));
                */
                st.exportToDot();
                st.printTable();
                //test end

                std::cout << "\n" << "----- SEMANTIC START -----" << "\n" << std::endl;
                SemanticAnalysis semantic;
                semantic.traversal(st, root, st.getRootScope());

                std::cout << "\n" << "----- INTERMEDIATE START -----" << "\n" << std::endl;
                Expression* ex = new Expression("PLUS", "$1", "$2", "x");
                CondJump* cj = new CondJump("iffalse", "X", "L1" );
                Jump* jump = new Jump("L2");
                MethodCall* mc = new MethodCall("f", "N", "x");

                ex->dump();
                cj->dump();
                jump->dump();
                mc->dump();

                std::cout << "\n" << "----- BBLOCK START -----" << "\n" << std::endl;
                BBlock* bblock = new BBlock();
                bblock->addTacInstructions(ex);
                bblock->addTacInstructions(cj);
                bblock->addTacInstructions(jump);
                bblock->addTacInstructions(mc);
                bblock->printAllTacInstructions();

                std::cout << "\n" << "----- BBLOCK1 START -----" << "\n" << std::endl;
                BBlock* bblock1 = new BBlock();
                bblock1->setTrueExit(bblock1);
                bblock1->addTacInstructions(ex);
                bblock1->printAllTacInstructions();
            }
            catch (...)
            {
                errCode = errCodes::AST_ERROR;
            }
        }
    }

    return errCode;
}