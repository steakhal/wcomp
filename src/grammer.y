%language "c++"
%locations
%define api.value.type variant

%code requires {
  #include "implementation.hh"
  #include <vector>
  #include <memory>
  #include <utility>
  #include <cstdlib>
}

%code provides {
int yylex(yy::parser::semantic_type* yylval, yy::parser::location_type* yylloc);
}

%parse-param {commands_t &ast}

%token PROGRAM BEGIN_ END
%token BOOLEAN NATURAL
%token READ WRITE
%token IF THEN ELSE ENDIF
%token WHILE FOR DOTDOT DO DONE
%token TRUE FALSE
%token COMMA ASSIGN
%token LPAREN RPAREN
%token <std::string> ID
%token <std::string> NUM

%left OR
%left AND
%left EQ
%left LT GT LE GE
%left ADD SUB
%left MUL DIV MOD
%precedence NOT

%type <std::unique_ptr<expression>> expression
%type <std::unique_ptr<instruction>> command
%type <std::vector<std::unique_ptr<instruction>>> commands

%%

start:
  PROGRAM ID declarations BEGIN_ commands END {
    type_check_commands($5);
    ast = std::move($5);
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
    $$ = std::vector<std::unique_ptr<instruction>>();
  }
| commands command {
    $1.push_back(std::move($2));
    $$ = std::move($1);
  }
;

command:
  READ LPAREN ID RPAREN {
    $$ = std::make_unique<read_instruction>(@1.begin.line, std::move($3));
  }
| WRITE LPAREN expression RPAREN {
    $$ = std::make_unique<write_instruction>(@1.begin.line, std::move($3));
  }
| ID ASSIGN expression {
    $$ = std::make_unique<assign_instruction>(@2.begin.line, std::move($1), std::move($3));
  }
| IF expression THEN commands ENDIF {
    $$ = std::make_unique<if_instruction>(@1.begin.line, std::move($2), std::move($4), std::vector<std::unique_ptr<instruction>>());
  }
| IF expression THEN commands ELSE commands ENDIF {
    $$ = std::make_unique<if_instruction>(@1.begin.line, std::move($2), std::move($4), std::move($6));
  }
| WHILE expression DO commands DONE {
    $$ = std::make_unique<while_instruction>(@1.begin.line, std::move($2), std::move($4));
  }
| FOR ID ASSIGN expression DOTDOT expression DO commands DONE {
    $$ = std::make_unique<for_instruction>(@1.begin.line, std::move($2), std::move($4), std::move($6), std::move($8));
  }
;

expression:
  NUM {
    $$ = std::make_unique<number_expression>(std::atoi($1.c_str()));
  }
| TRUE {
    $$ = std::make_unique<boolean_expression>(true);
  }
| FALSE {
    $$ = std::make_unique<boolean_expression>(false);
  }
| ID {
    $$ = std::make_unique<id_expression>(@1.begin.line, std::move($1));
  }
| expression ADD expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "+", std::move($1), std::move($3));
  }
| expression SUB expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "-", std::move($1), std::move($3));
  }
| expression MUL expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "*", std::move($1), std::move($3));
  }
| expression DIV expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "/", std::move($1), std::move($3));
  }
| expression MOD expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "%", std::move($1), std::move($3));
  }
| expression LT expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "<", std::move($1), std::move($3));
  }
| expression GT expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, ">", std::move($1), std::move($3));
  }
| expression LE expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "<=", std::move($1), std::move($3));
  }
| expression GE expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, ">=", std::move($1), std::move($3));
  }
| expression AND expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "and", std::move($1), std::move($3));
  }
| expression OR expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "or", std::move($1), std::move($3));
  }
| expression EQ expression {
    $$ = std::make_unique<binop_expression>(@2.begin.line, "=", std::move($1), std::move($3));
  }
| NOT expression {
    $$ = std::make_unique<not_expression>(@1.begin.line, "not", std::move($2));
  }
| LPAREN expression RPAREN {
    $$ = std::move($2);
  }
;

%%
