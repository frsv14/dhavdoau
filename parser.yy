
%skeleton "lalr1.cc"
%defines
%define parse.error verbose
%define api.value.type variant
%define api.token.constructor

%code requires{
  #include <string>
  #include "Node.h"
  #define USE_LEX_ONLY false
}

%code{
  #define YY_DECL yy::parser::symbol_type yylex()
  YY_DECL;

  Node* root = nullptr;
  extern int yylineno;

  bool syntax_errors = false;
}

%token <std::string> ID INT FLOAT
%token DELIMITER
%token CLASS MAIN IF ELSE FOR WHILE PRINT READ RETURN BREAK CONTINUE
%token VOLATILE
%token INTKEY FLOATKEY BOOLEANKEY VOIDKEY
%token <std::string> TRUE FALSE 
%token LENGTH

%token <std::string> PLUSOP MINUSOP MULTOP DIVOP POWEROP
%token AND OR 
%token NOTOP
%token EQUAL NOTEQUAL LT GT LTE GTE
%token <std::string> ASSIGNMENTOP

%token <std::string> LP RP
%token <std::string> LSB RSB LCB RCB
%token <std::string> COMMA COLON DOT
%token NEWLINE
%token END 0 "end of file"

%left OR
%left AND
%nonassoc EQUAL NOTEQUAL LT GT LTE GTE
%left PLUSOP MINUSOP
%left MULTOP DIVOP
%right POWEROP
%right NOTOP

%type <Node*> program class class_body entry method var type baseType  
%type <Node*> stmt stmtBl stmt_list
%type <Node*> param_list param_list_opt
%type <Node*> root
%type <Node*> expr
%type <Node*> for_header
%type <Node*> expr_list

%%
root
    : program entry stmtEnd END
        {
          root = $1;
          root->children.push_back($2);
        }
    ;

program
    :  { $$ = new Node("Program", "", yylineno); }
    | program class stmtEnd
        { $$ = $1; $$->children.push_back($2); }
    | program entry stmtEnd
        { $$ = $1; $$->children.push_back($2); }
    | program stmt stmtEnd  
        { $$ = $1; $$->children.push_back($2); }
    | program var stmtEnd
        { $$ = $1; $$->children.push_back($2); }
    | program DELIMITER
        {
          Node* n = new Node("Delimiter", "%%", yylineno);
          $$ = $1;
          $$->children.push_back(n);
        }
    | program NEWLINE
        { $$ = $1; }  
    ;

class
    : CLASS ID LCB class_body RCB
        {
          $$ = new Node("Class", $2, yylineno);
          $$->children.push_back($4);
        }
    ;

class_body
    : 
        { $$ = new Node("ClassBody", "", yylineno); }
    | class_body var stmtEnd
        { $$ = $1; $$->children.push_back($2); }
    | class_body method stmtEnd
        { $$ = $1; $$->children.push_back($2); }
    | class_body NEWLINE  
        { $$ = $1; }
    ;

entry
    : MAIN LP RP COLON INTKEY stmtBl
        {
          $$ = new Node("Main", "", yylineno);
          $$->children.push_back($6);
        }
    ;

method
    : ID LP param_list_opt RP COLON type stmtBl
        {
          $$ = new Node("Method", $1, yylineno);
          $$ = new Node("Params", "", yylineno);
          $$->children.push_back($3);
          $$->children.push_back($6);
          $$->children.push_back($7);
        }
    | ID COLON type LP RP stmtBl
        {
          $$ = new Node("Method", $1, yylineno);
          $$ = new Node("Params", "", yylineno);
          $$->children.push_back($3);
          $$->children.push_back($3);
          $$->children.push_back($6);
        }
    ;

param_list_opt
    : { $$ = new Node("Params", "", yylineno); }
    | param_list  { $$ = $1; }
    ;

param_list
    : ID COLON type
        {
          $$ = new Node("Params", "", yylineno);
          Node* p = new Node("Param", $1, yylineno);
          p->children.push_back($3);
          $$->children.push_back(p);
        }
    | param_list COMMA ID COLON type
        {
          Node* p = new Node("Param", $3, yylineno);
          p->children.push_back($5);
          $1->children.push_back(p);
          $$ = $1;
        }
    ;
    
var
    : VOLATILE ID COLON type
        {
          $$ = new Node("VarDecl", $2, yylineno);
          $$->children.push_back($4);
        }
    | VOLATILE ID COLON type ASSIGNMENTOP expr
        {
          $$ = new Node("VarDeclAssign", $2, yylineno);
          $$->children.push_back($4); 
          $$->children.push_back($6); 
        }
    | ID COLON type
        {
          $$ = new Node("VarDecl", $1, yylineno);
          $$->children.push_back($3);
        }
    | ID COLON type ASSIGNMENTOP expr
        {
          $$ = new Node("VarDeclAssign", $1, yylineno);
          $$->children.push_back($3);
          $$->children.push_back($5);
        }
    ;

type
    : baseType           { $$ = $1; }
    | baseType LSB RSB   { $$ = new Node("ArrayType", "", yylineno); $$->children.push_back($1); }
    | ID                 { $$ = new Node("TypeID", $1, yylineno); }
    | VOIDKEY            { $$ = new Node("VoidType", "", yylineno); }
    ;

baseType
    : INTKEY     { $$ = new Node("IntType", "", yylineno); }
    | FLOATKEY   { $$ = new Node("FloatType", "", yylineno); }
    | BOOLEANKEY { $$ = new Node("BoolType", "", yylineno); }
    ;



stmtBl
    : LCB stmt_list RCB
        { $$ = new Node("Block", "", yylineno); $$->children.push_back($2); }
    ;

stmt_list
    : 
    { $$ = new Node("StmtList", "", yylineno); }
    | stmt_list stmt { $$ = $1; $$->children.push_back($2); }
    | stmt_list NEWLINE { $$ = $1; } 
    ;

stmt
    : NEWLINE stmt                   { $$ = $2; }
    | stmtBl                         { $$ = $1; }
    | var                  
      { $$ = $1; }
    | expr ASSIGNMENTOP expr stmtEnd
        {
          $$ = new Node("Assign", "", yylineno);
          $$->children.push_back($1);
          $$->children.push_back($3);
        }
    | FOR LP for_header RP stmtBl
        {
          $$ = new Node("For", "", yylineno);
          $$->children.push_back($3);
          $$->children.push_back($5); 
        }
    |WHILE LP expr RP stmtBl
        {
          $$ = new Node("While", "", yylineno);
          $$->children.push_back($3);
          $$->children.push_back($5); 
        }
    | IF LP expr RP stmt ELSE stmt
        {
          $$ = new Node("IfElse", "", yylineno);
          $$->children.push_back($3);
          $$->children.push_back($5);
          $$->children.push_back($7);
        }
    | IF LP expr RP stmt
        {
          $$ = new Node("If", "", yylineno);
          $$->children.push_back($3);
          $$->children.push_back($5);
        }
    | PRINT LP expr RP stmtEnd
        {
          $$ = new Node("Print", "", yylineno);
          $$->children.push_back($3);
        }
    | READ LP expr RP stmtEnd
        {
          $$ = new Node("Read", "", yylineno);
          $$->children.push_back($3);
        }
    | RETURN expr stmtEnd
        {
          $$ = new Node("Return", "", yylineno);
          $$->children.push_back($2);
        }
    | BREAK stmtEnd     { $$ = new Node("Break", "", yylineno); }
    | CONTINUE stmtEnd { $$ = new Node("Continue", "", yylineno); }
    | expr stmtEnd     { $$ = $1; }
    ;

for_header
    : expr ASSIGNMENTOP expr COMMA expr COMMA expr ASSIGNMENTOP expr
        {
        $$ = new Node("ForHeader", "", yylineno);

        Node* init = new Node("Init", "", yylineno);
        init->children.push_back($1); 
        init->children.push_back($3);

        Node* cond = new Node("Condition", "", yylineno);
        cond->children.push_back($5); 

        Node* update = new Node("Update", "", yylineno);
        update->children.push_back($7); 
        update->children.push_back($9); 

        $$->children.push_back(init);
        $$->children.push_back(cond);
        $$->children.push_back(update);
        }
    ;


stmtEnd
    : NEWLINE
    | stmtEnd NEWLINE
    |
    ;

expr
    : NOTOP expr
      {
        Node* n = new Node("Not", "", yylineno);
        n->children.push_back($2);
        $$ = n;
      }
    | expr LENGTH LP expr RP
      {
        Node* n = new Node("Length", "", yylineno);
        n->children.push_back($1);
        $$ = n;
      }
    | expr POWEROP expr
      {
        Node* n = new Node("Pow", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr MULTOP expr
      {
        Node* n = new Node("Mul", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr DIVOP expr
      {
        Node* n = new Node("Div", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr PLUSOP expr
      {
        Node* n = new Node("Add", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr MINUSOP expr
      {
        Node* n = new Node("Sub", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr AND expr
      {
        Node* n = new Node("And", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr OR expr
      {
        Node* n = new Node("Or", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr EQUAL expr
      {
        Node* n = new Node("Eq", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr NOTEQUAL expr
      {
        Node* n = new Node("Neq", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr LT expr
      {
        Node* n = new Node("Lt", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr GT expr
      {
        Node* n = new Node("Gt", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr LTE expr
      {
        Node* n = new Node("Lte", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | expr GTE expr
      {
        Node* n = new Node("Gte", "", yylineno);
        n->children.push_back($1);
        n->children.push_back($3);
        $$ = n;
      }
    | LP expr RP
      { $$ = $2; }
    | ID
      { $$ = new Node("Identifier", $1, yylineno); }
    | INT
      { $$ = new Node("IntLiteral", $1, yylineno); }
    | FLOAT
      { $$ = new Node("FloatLiteral", $1, yylineno); }
    | TRUE
      { $$ = new Node("True", "true", yylineno); }
    | FALSE
      { $$ = new Node("False", "false", yylineno); }
    | expr DOT ID
        {
          Node* n = new Node("MemberAccess", "", yylineno);
          n->children.push_back($1); 
          n->children.push_back(new Node("Identifier", $3, yylineno)); 
          $$ = n;
          }
    | expr LSB expr RSB
        {
          Node* n = new Node("ArrayAccess", "", yylineno);
          n->children.push_back($1);
          n->children.push_back($3); 
          $$ = n;
        }
    | type LSB expr_list RSB
        {
          Node* n = new Node("ArrayLiteral", "", yylineno);
          n->children.push_back($1);
          n->children.push_back($3); 
          $$ = n;
        }
    | INTKEY LSB expr_list RSB
        {
          Node* n = new Node("ArrayLiteral", "", yylineno);
          Node* t = new Node("IntType", "", yylineno);
          n->children.push_back(t);
          n->children.push_back($3);
          $$ = n;
        }
    | FLOATKEY LSB expr_list RSB
        {
          Node* n = new Node("ArrayLiteral", "", yylineno);
          Node* t = new Node("FloatType", "", yylineno);
          n->children.push_back(t);
          n->children.push_back($3);
          $$ = n;
        }
    | LSB RSB
        {
          Node* n = new Node("EmptyArrayLiteral", "", yylineno);
          $$ = n;
        }
    | ID LP RP
        {
          Node* n = new Node("FuncCall", $1, yylineno);
          $$ = n;
        }
    | ID LP expr_list RP
        {
          Node* n = new Node("FuncCall", $1, yylineno);
          n->children.push_back($3);
          $$ = n;
        }
    | expr DOT ID LP RP
        {
          Node* n = new Node("MethodCall", "", yylineno);
          n->children.push_back($1); 
          n->children.push_back(new Node("Identifier", $3, yylineno)); 
          $$ = n;
        }
    | expr DOT ID LP expr_list RP
        {
          Node* n = new Node("MethodCall", "", yylineno);
          n->children.push_back($1);
          n->children.push_back(new Node("Identifier", $3, yylineno));
          n->children.push_back($5);
          $$ = n;
        }
    ;

expr_list
    : expr
        {
          $$ = new Node("ExprList", "", yylineno);
          $$->children.push_back($1); 
        }
    | expr_list COMMA expr
        {
          $$ = $1;  
          $$->children.push_back($3); 
        }
    ;
%%


