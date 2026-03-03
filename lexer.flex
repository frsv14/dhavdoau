%top{
    #include "parser.tab.hh"
    #define YY_DECL yy::parser::symbol_type yylex()
    #include "Node.h"
    int lexical_errors = 0;
    int brace_level = 0;
    std::vector<int> indent_stack = {0}; 
    int pending_indents = 0;              
    int pending_dedents = 0;              
    bool at_line_start = true;
}
%option yylineno noyywrap nounput batch noinput stack 
%%

"main"                  {if(USE_LEX_ONLY) {printf("MAINKEY ");} else {return yy::parser::make_MAIN();}}
"class"                 {if(USE_LEX_ONLY) {printf("CLASSKEY ");} else {return yy::parser::make_CLASS();}}

"+"                     {if(USE_LEX_ONLY) {printf("PLUSOP ");} else {return yy::parser::make_PLUSOP(yytext);}}
"-"                     {if(USE_LEX_ONLY) {printf("SUBOP ");} else {return yy::parser::make_MINUSOP(yytext);}}
"*"                     {if(USE_LEX_ONLY) {printf("MULTOP ");} else {return yy::parser::make_MULTOP(yytext);}}
"/"                     {if(USE_LEX_ONLY) {printf("DIVOP ");} else {return yy::parser::make_DIVOP(yytext);}}
"^"                     {if(USE_LEX_ONLY) {printf("POWEROP ");} else {return yy::parser::make_POWEROP(yytext);}}
"("                     {if(USE_LEX_ONLY) {printf("LP ");} else {return yy::parser::make_LP(yytext);}}
")"                     {if(USE_LEX_ONLY) {printf("RP ");} else {return yy::parser::make_RP(yytext);}}
"["                     {if(USE_LEX_ONLY) {printf("LSB ");} else {return yy::parser::make_LSB(yytext);}}
"]"                     {if(USE_LEX_ONLY) {printf("RSB ");} else {return yy::parser::make_RSB(yytext);}}
"{"                     {if(USE_LEX_ONLY) {printf("LCB ");} else { brace_level++; return yy::parser::make_LCB(yytext);}}
"}"                     {if(USE_LEX_ONLY) {printf("RCB ");} else {if (brace_level > 0) brace_level--; return yy::parser::make_RCB(yytext);}}
"&"                     {if(USE_LEX_ONLY) {printf("AND ");} else {return yy::parser::make_AND();}}
"|"                     {if(USE_LEX_ONLY) {printf("OR ");} else {return yy::parser::make_OR();}}
"<"                     {if(USE_LEX_ONLY) {printf("LT ");} else {return yy::parser::make_LT();}}
">"                     {if(USE_LEX_ONLY) {printf("GT ");} else {return yy::parser::make_GT();}}
"<="                    {if(USE_LEX_ONLY) {printf("LTE ");} else {return yy::parser::make_LTE();}}
">="                    {if(USE_LEX_ONLY) {printf("GTE ");} else {return yy::parser::make_GTE();}}
"="                     {if(USE_LEX_ONLY) {printf("EQUAL ");} else {return yy::parser::make_EQUAL();}}
"!="                    {if(USE_LEX_ONLY) {printf("NOTEQUAL ");} else {return yy::parser::make_NOTEQUAL();}}
":="                    {if(USE_LEX_ONLY) {printf("ASSIGNMENTOP ");} else {return yy::parser::make_ASSIGNMENTOP(yytext);}}
"!"                     {if(USE_LEX_ONLY) {printf("NOTOP ");} else {return yy::parser::make_NOTOP();}}

"%%"                    {if(USE_LEX_ONLY) {printf("DELIMITER ");} else {return yy::parser::make_DELIMITER ();}}
","                     {if(USE_LEX_ONLY) {printf("COMMA ");} else {return yy::parser::make_COMMA(yytext);}}
":"                     {if(USE_LEX_ONLY) {printf("COLON ");} else {return yy::parser::make_COLON(yytext);}}
"."                     {if(USE_LEX_ONLY) {printf("DOT ");} else {return yy::parser::make_DOT(yytext);}}

"int"                   {if(USE_LEX_ONLY) {printf("INTEGERKEY ");} else {return yy::parser::make_INTKEY();}}
"float"                 {if(USE_LEX_ONLY) {printf("FLOATKEY ");} else {return yy::parser::make_FLOATKEY();}}
"boolean"               {if(USE_LEX_ONLY) {printf("BOOLEANKEY ");} else {return yy::parser::make_BOOLEANKEY();}}
"void"                  {if(USE_LEX_ONLY) {printf("VOIDKEY ");} else {return yy::parser::make_VOIDKEY();}}
"if"                    {if(USE_LEX_ONLY) {printf("IFKEY ");} else {return yy::parser::make_IF();}}
"else"                  {if(USE_LEX_ONLY) {printf("ELSEKEY ");} else {return yy::parser::make_ELSE();}}
"for"                   {if(USE_LEX_ONLY) {printf("FORKEY ");} else {return yy::parser::make_FOR();}}
"while"                 {if(USE_LEX_ONLY) {printf("WHILEKEY ");} else {return yy::parser::make_WHILE();}}
"print"                 {if(USE_LEX_ONLY) {printf("PRINTKEY ");} else {return yy::parser::make_PRINT();}}
"read"                  {if(USE_LEX_ONLY) {printf("READKEY ");} else {return yy::parser::make_READ();}}
"return"                {if(USE_LEX_ONLY) {printf("RETURNKEY ");} else {return yy::parser::make_RETURN();}}
"break"                 {if(USE_LEX_ONLY) {printf("BREAKKEY ");} else {return yy::parser::make_BREAK();}}
"continue"              {if(USE_LEX_ONLY) {printf("CONTINUEKEY ");} else {return yy::parser::make_CONTINUE();}}
"volatile"              {if(USE_LEX_ONLY) {printf("VOLATILEKEY ");} else {return yy::parser::make_VOLATILE();}}
"lenght"                {if(USE_LEX_ONLY) {printf("LENGHTKEY ");} else {return yy::parser::make_LENGTH ();}}
"true"                  {if(USE_LEX_ONLY) {printf("LENGHTKEY ");} else {return yy::parser::make_TRUE(yytext);}}
"false"                 {if(USE_LEX_ONLY) {printf("LENGHTKEY ");} else {return yy::parser::make_FALSE(yytext);}}

0|[1-9][0-9]*           {if(USE_LEX_ONLY) {printf("INT ");} else {return yy::parser::make_INT(yytext);}}
[0-9]+"."[0-9]+         {if (USE_LEX_ONLY) {printf("FLOAT ");} else {return yy::parser::make_FLOAT(yytext);}}

[a-zA-Z_][a-zA-Z0-9_]* { return yy::parser::make_ID(yytext); }


^[ \t]*$ {
    if (at_line_start && pending_dedents > 0) {
        pending_dedents--;
        return yy::parser::make_DEDENT();
    }
}

^[ \t]+ {
    if (!at_line_start) {
    
    } else {
        int tabs = yyleng;
        int current = indent_stack.back();

        if (brace_level > 0) {
        
            at_line_start = false;
        } else if (tabs > current) {
            indent_stack.push_back(tabs);
            at_line_start = false;
            return yy::parser::make_INDENT();
        } else if (tabs < current) {
            int count = 0;
            while (indent_stack.size() > 1 && indent_stack.back() > tabs) {
                indent_stack.pop_back();
                count++;
            }
            if (count > 0) {
                if (count > 1)
                    pending_dedents += (count - 1);
                at_line_start = false;
                return yy::parser::make_DEDENT();
            }
            at_line_start = false;
        } else {
          
            at_line_start = false;
        }
    }
}


[ \t]+ {  }


"//"[^\n]*              {}

.            { 
                if (!lexical_errors) 
                    fprintf(stderr, "Lexical errors found! See below:\n"); 
                lexical_errors++; 
                fprintf(stderr, "\t@line %d, unrecognized character '%s'\n", yylineno, yytext); 
            
             }
\r?\n                    {at_line_start = true; if(USE_LEX_ONLY) {printf("NEWLINE \n");} else {return yy::parser::make_NEWLINE();}}

<<EOF>>                  {
                            while (indent_stack.size() > 1) {
                                indent_stack.pop_back();
                                return yy::parser::make_DEDENT();
                            }
                            return yy::parser::make_END();
                        }


%%