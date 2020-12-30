%option c++ noyywrap

%{
  #include "utility.h" // error
  #include "grammar.hpp"
  using tok = yy::parser::token;
%}

identifier [a-zA-Z_][a-zA-Z0-9_]*
decimal    [0-9]+
whitespace [ \t\n]+

%%

program      return tok::PROGRAM;
begin        return tok::BEGIN_;
end          return tok::END;
boolean      return tok::BOOLEAN;
natural      return tok::NATURAL;
read         return tok::READ;
write        return tok::WRITE;
if           return tok::IF;
then         return tok::THEN;
else         return tok::ELSE;
endif        return tok::ENDIF;
while        return tok::WHILE;
for          return tok::FOR;
do           return tok::DO;
done         return tok::DONE;
true         return tok::TRUE;
false        return tok::FALSE;
","          return tok::COMMA;
":="         return tok::ASSIGN;
".."         return tok::DOTDOT;
"+"          return tok::ADD;
"-"          return tok::SUB;
"*"          return tok::MUL;
"/"          return tok::DIV;
"%"          return tok::MOD;
"<"          return tok::LT;
">"          return tok::GT;
"<="         return tok::LE;
">="         return tok::GE;
"="          return tok::EQ;
and          return tok::AND;
or           return tok::OR;
not          return tok::NOT;
"("          return tok::LPAREN;
")"          return tok::RPAREN;
{identifier} return tok::ID;
{decimal}    return tok::NUM;
{whitespace} // Whitespaces skipped.
#.*          // Commens skipped.
. {
  std::string msg = ": Unexpected character: '";
  msg += YYText();
  msg += "'.\n";
  error(lineno(), msg);
}
