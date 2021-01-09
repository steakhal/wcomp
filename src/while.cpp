#include "FlexLexer.h"
#include "grammar.hpp"

#include "ast_dumper.h"
#include "ast_to_cfg.h"
#include "cfg.h"
#include "cfg_dumper.h"
#include "cfg_transformer.h"
#include "codegen.h"
#include "expressions.h"
#include "statements.h"
#include "utility.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>

#include <CLI/CLI.hpp>

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

int main(int argc, char **argv) {
  CLI::App app("Obfuscicating While compiler");
  std::string src;
  app.add_option("source", src, "while source code")
      ->required()
      ->check(CLI::ExistingFile);

  // Compilation and interpretation are mutually exclusive.
  CLI::Option *compile =
      app.add_flag("-c,--compile")
          ->multi_option_policy(CLI::MultiOptionPolicy::Throw);
  CLI::Option *interpret =
      app.add_flag("-i,--interpret")
          ->multi_option_policy(CLI::MultiOptionPolicy::Throw);
  compile->excludes(interpret);
  interpret->excludes(compile);

  bool flatten_cfg{false};
  bool encode_constants{false};
  std::optional<std::size_t> remap_bb_ids_seed;
  std::optional<std::size_t> serialization_seed;

  bool dump_ast{false};
  bool dump_cfg_text{false};
  bool dump_cfg_dot{false};
  app.add_flag("--dump-ast", dump_ast, "Dumps the AST for the source code.")
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw);
  app.add_flag("--dump-cfg-text", dump_cfg_text,
               "Dumps the Control-flow graph in a human readable form.")
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw);
  app.add_flag("--dump-cfg-dot", dump_cfg_dot,
               "Dumps the Control-flow graph in Graphwiz dot format.")
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw);

  app.add_flag("--flatten-cfg", flatten_cfg, "Flatten the control-flow graph.")
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw);

  app.add_flag("--remap-basic-block-ids", remap_bb_ids_seed,
               "Remap basic block ids.")
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw);

  app.add_flag("--xor-encode-constants", encode_constants,
               "Xor encode constants.")
      ->multi_option_policy(CLI::MultiOptionPolicy::Throw);

  app.add_option(
      "--random-remap-basic-blocks-seed", remap_bb_ids_seed,
      "Randomize the identifier of the basic blocks in the control-flow graph. "
      "Specify the seed for the pseudo-random sequence. -1 means random seed.");

  app.add_option(
      "--random-basic-block-serialization-seed", serialization_seed,
      "Randomize the order of the basic blocks when emitting assembly."
      "Specify the seed for the pseudo-random sequence. -1 means random seed.");

  CLI11_PARSE(app, argc, argv);

  const auto build_ast_from = [](std::istream &is) {
    ast ast;
    std::unique_ptr lexer = std::make_unique<yyFlexLexer>(&is);
    ::lexer = lexer.get();
    yy::parser parser{ast};
    parser.parse();
    ::lexer = nullptr;
    return ast;
  };

  std::ifstream input(src.c_str());
  ast code = build_ast_from(input);

  if (dump_ast)
    ast_dumper{std::cerr}(code);

  cfg graph = ast_to_cfg(std::move(code.stmts));

  if (flatten_cfg)
    flatten(code.syms, graph);

  if (remap_bb_ids_seed.has_value()) {
    if (remap_bb_ids_seed.value() == -1) {
      std::random_device rd;
      std::mt19937 gen(rd());
      remap_block_ids(graph, gen);
    } else {
      std::mt19937 gen(remap_bb_ids_seed.value());
      remap_block_ids(graph, gen);
    }
  }

  if (dump_cfg_dot)
    dot_cfg_dumper{std::cerr}(graph);
  if (dump_cfg_text)
    text_cfg_dumper{std::cerr}(graph);

  if (compile->count() == 1) {
    std::cout << codegen(graph, code.syms, serialization_seed,
                         encode_constants);
  } else {
    //::execute(ast);
  }
}
