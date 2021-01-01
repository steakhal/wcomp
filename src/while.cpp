#include "FlexLexer.h"
#include "grammar.hpp"

#include "expressions.h"
#include "statements.h"
#include "utility.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

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

static std::string get_bootstrapped_code(const statements &stmts) {
  std::stringstream ss;
  ss << "global main\n"
        "extern write_natural\n"
        "extern read_natural\n"
        "extern write_boolean\n"
        "extern read_boolean\n\n"
        "section .bss\n";
  std::map<std::string, symbol>::iterator it;
  for (it = symbol_table.begin(); it != symbol_table.end(); ++it) {
    ss << it->second.get_code();
  }
  ss << "\nsection .text\nmain:\n";
  ss << ::get_code(stmts);
  ss << "xor eax,eax\n";
  ss << "ret\n";
  return ss.str();
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " (-c|-i) inputfile" << std::endl;
    exit(1);
  }

  bool should_compile = false;
  if (std::string(argv[1]) == "-c") {
    should_compile = true;
  } else if (std::string(argv[1]) == "-i") {
    should_compile = false;
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
    statements ast;
    std::unique_ptr lexer = std::make_unique<yyFlexLexer>(&is);
    ::lexer = lexer.get();
    yy::parser parser{ast};
    parser.parse();
    ::lexer = nullptr;
    return ast;
  };

  statements ast = build_ast_from(input);

  if (should_compile) {
    std::cout << ::get_bootstrapped_code(ast);
  } else {
    ::execute(ast);
  }
  return 0;
}
