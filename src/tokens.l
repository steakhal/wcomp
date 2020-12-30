%option c++ noyywrap

%{
  #include "expressions.h"
  #include "statements.h"
  #include "utility.h"
  #include "grammar.hpp"

  #include <iostream>
  #include <cstdlib>
%}

%%

program                     return yy::parser::token::PROGRAM;
begin                       return yy::parser::token::BEGIN_;
end                         return yy::parser::token::END;
boolean                     return yy::parser::token::BOOLEAN;
natural                     return yy::parser::token::NATURAL;
read                        return yy::parser::token::READ;
write                       return yy::parser::token::WRITE;
if                          return yy::parser::token::IF;
then                        return yy::parser::token::THEN;
else                        return yy::parser::token::ELSE;
endif                       return yy::parser::token::ENDIF;
while                       return yy::parser::token::WHILE;
for                         return yy::parser::token::FOR;
do                          return yy::parser::token::DO;
done                        return yy::parser::token::DONE;
true                        return yy::parser::token::TRUE;
false                       return yy::parser::token::FALSE;
","                         return yy::parser::token::COMMA;
":="                        return yy::parser::token::ASSIGN;
".."                        return yy::parser::token::DOTDOT;
"+"                         return yy::parser::token::ADD;
"-"                         return yy::parser::token::SUB;
"*"                         return yy::parser::token::MUL;
"/"                         return yy::parser::token::DIV;
"%"                         return yy::parser::token::MOD;
"<"                         return yy::parser::token::LT;
">"                         return yy::parser::token::GT;
"<="                        return yy::parser::token::LE;
">="                        return yy::parser::token::GE;
"="                         return yy::parser::token::EQ;
and                         return yy::parser::token::AND;
or                          return yy::parser::token::OR;
not                         return yy::parser::token::NOT;
"("                         return yy::parser::token::LPAREN;
")"                         return yy::parser::token::RPAREN;
[a-zA-Z_][a-zA-Z0-9_]*      return yy::parser::token::ID;
[0-9]+                      return yy::parser::token::NUM;
[ \t\n]+                    // Whitespaces skipped.
#.*                         // Commens skipped.
.                           { std::cerr << "Line " << lineno() << ": Unexpected character: '" << YYText() << "'.\n"; std::exit(1); }
