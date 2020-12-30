#include "grammer.hpp"
#include "implementation.hh"

#include <FlexLexer.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>

yyFlexLexer *lexer;

int yylex(yy::parser::semantic_type *yylval,
          yy::parser::location_type *yylloc) {
  yylloc->begin.line = lexer->lineno();
  int token = lexer->yylex();
  if (token == yy::parser::token::ID || token == yy::parser::token::NUM) {
    yylval->build(std::string(lexer->YYText()));
  }
  return token;
}

void yy::parser::error(const location_type &loc, const std::string &msg) {
  std::cerr << "Line " << loc.begin.line << ": " << msg << std::endl;
  exit(1);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " (-c|-i) inputfile" << std::endl;
    exit(1);
  }

  if (std::string(argv[1]) == "-c") {
    current_mode = compiler;
  } else if (std::string(argv[1]) == "-i") {
    current_mode = interpreter;
  } else {
    std::cerr << "Usage: " << argv[0] << "(-c|-i) inputfile" << std::endl;
    exit(1);
  }

  std::ifstream input(argv[2]);
  if (!input) {
    std::cerr << "Cannot open input file: " << argv[2] << std::endl;
    exit(1);
  }

  const auto build_ast_from = [](std::istream &is) {
    commands_t ast;
    std::unique_ptr lexer = std::make_unique<yyFlexLexer>(&is);
    ::lexer = lexer.get();
    yy::parser parser{ast};
    parser.parse();
    ::lexer = nullptr;
    return ast;
  };

  commands_t ast = build_ast_from(input);
  return 0;
}
