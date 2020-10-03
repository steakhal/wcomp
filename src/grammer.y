%language "c++"
%locations
%define api.value.type variant

%code top {
#include "implementation.hh"
#include <list>
}

%code provides {
int yylex(yy::parser::semantic_type* yylval, yy::parser::location_type* yylloc);
}

%token PROGRAM BEGIN_ END
%token BOOLEAN NATURAL
%token READ WRITE
%token IF THEN ELSE ENDIF
%token WHILE FOR DOTDOT DO DONE
%token TRUE FALSE
%token COMMA ASSIGN
%token LPAREN RPAREN
%token QMARK COLON
%token <std::string> ID
%token <std::string> NUM

%left OR
%left AND
%left EQ
%left LT GT LE GE
%left ADD SUB
%left MUL DIV MOD
%precedence NOT

%type <expression*> expression
%type <instruction*> command
%type <std::list<instruction*>* > commands
%type <std::list<std::string>* > idpack

%type <std::list<expression*>* > exprpack

%%

start:
  PROGRAM ID declarations BEGIN_ commands END {
    type_check_commands($5);
    if(current_mode == compiler) {
      generate_code($5);
    } else {
      execute_commands($5);
    }
    delete_commands($5);
  }
;

declarations:
  /* empty */
| declarations declaration
;

declaration:
  BOOLEAN ID {
    symbol(@1.begin.line, $2, boolean).declare();
  }
| NATURAL ID {
    symbol(@1.begin.line, $2, natural).declare();
  }
;

commands:
  /* empty */ {
    $$ = new std::list<instruction*>();
  }
| commands command {
    $1->push_back($2);
    $$ = $1;
  }
;

idpack:
  ID {
    auto *p = new std::list<std::string>();
    p->push_back($1);
    $$ = p;
  }
| idpack COMMA ID {
    $1->push_back($3);
    $$ = $1;
  }
;

exprpack:
  expression {
    auto *p = new std::list<expression*>();
    p->push_back($1);
    $$ = p;
  }
| exprpack COMMA expression {
    $1->push_back($3);
    $$ = $1;
  }
;

command:
  READ LPAREN ID RPAREN {
    $$ = new read_instruction(@1.begin.line, $3);
  }
| WRITE LPAREN expression RPAREN {
    $$ = new write_instruction(@1.begin.line, $3);
  }
| ID ASSIGN expression {
    $$ = new assign_instruction(@2.begin.line, $1, $3);
  }
| idpack ASSIGN exprpack {
    $$ = new simultan_assign_instruction(@2.begin.line, $1, $3);
  }
| IF expression THEN commands ENDIF {
    $$ = new if_instruction(@1.begin.line, $2, $4, 0);
  }
| IF expression THEN commands ELSE commands ENDIF {
    $$ = new if_instruction(@1.begin.line, $2, $4, $6);
  }
| WHILE expression DO commands DONE {
    $$ = new while_instruction(@1.begin.line, $2, $4);
  }
| FOR ID ASSIGN expression DOTDOT expression DO commands DONE {
    $$ = new for_instruction(@1.begin.line, $2, $4, $6, $8);
  }
;

expression:
  NUM {
    $$ = new number_expression($1);
  }
| TRUE {
    $$ = new boolean_expression(true);
  }
| FALSE {
    $$ = new boolean_expression(false);
  }
| ID {
    $$ = new id_expression(@1.begin.line, $1);
  }
| expression QMARK expression COLON expression {
    $$ = new triop_expression(@2.begin.line, "?:", $1, $3, $5);
  }
| expression ADD expression {
    $$ = new binop_expression(@2.begin.line, "+", $1, $3);
  }
| expression SUB expression {
    $$ = new binop_expression(@2.begin.line, "-", $1, $3);
  }
| expression MUL expression {
    $$ = new binop_expression(@2.begin.line, "*", $1, $3);
  }
| expression DIV expression {
    $$ = new binop_expression(@2.begin.line, "/", $1, $3);
  }
| expression MOD expression {
    $$ = new binop_expression(@2.begin.line, "%", $1, $3);
  }
| expression LT expression {
    $$ = new binop_expression(@2.begin.line, "<", $1, $3);
  }
| expression GT expression {
    $$ = new binop_expression(@2.begin.line, ">", $1, $3);
  }
| expression LE expression {
    $$ = new binop_expression(@2.begin.line, "<=", $1, $3);
  }
| expression GE expression {
    $$ = new binop_expression(@2.begin.line, ">=", $1, $3);
  }
| expression AND expression {
    $$ = new binop_expression(@2.begin.line, "and", $1, $3);
  }
| expression OR expression {
    $$ = new binop_expression(@2.begin.line, "or", $1, $3);
  }
| expression EQ expression {
    $$ = new binop_expression(@2.begin.line, "=", $1, $3);
  }
| NOT expression {
    $$ = new not_expression(@1.begin.line, "not", $2);
  }
| LPAREN expression RPAREN {
    $$ = $2;
  }
;

%%
