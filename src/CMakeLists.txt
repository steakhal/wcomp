flex_target(lexer tokens.l "${GENERATED_SRC}/tokens.cpp")
add_library(lexer STATIC ${FLEX_lexer_OUTPUTS})
target_include_directories(lexer PRIVATE ".")
target_link_libraries(lexer PUBLIC CONAN_PKG::flex)


bison_target(parser grammar.y "${GENERATED_SRC}/grammar.cpp" COMPILE_FLAGS -Wcounterexamples)
add_library(parser STATIC ${BISON_parser_OUTPUTS})
target_include_directories(parser PRIVATE ".")
target_include_directories(parser INTERFACE "${GENERATED_SRC}")
target_link_libraries(parser PUBLIC CONAN_PKG::bison)


add_executable(wcomp
  while.cpp
  codegen.cpp
  misc.cpp
  interpreter.cpp
  typecheck.cpp
  cfg.cpp
  expression_dumper.cpp
  ast_dumper.cpp
  cfg_dumper.cpp
  ast_to_cfg.cpp
)
target_link_libraries(wcomp PRIVATE lexer parser)
target_include_directories(wcomp PRIVATE ".")
