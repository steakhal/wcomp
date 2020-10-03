flex_target(lexer tokens.l "${GENERATED_SRC}/tokens.cpp")
add_library(lexer STATIC ${FLEX_lexer_OUTPUTS})
target_include_directories(lexer PRIVATE ".")
target_link_libraries(lexer PUBLIC CONAN_PKG::flex)


bison_target(parser grammer.y "${GENERATED_SRC}/grammer.cpp")
add_library(parser STATIC ${BISON_parser_OUTPUTS})
target_include_directories(parser PRIVATE ".")
target_include_directories(parser INTERFACE "${GENERATED_SRC}")
target_link_libraries(parser PUBLIC CONAN_PKG::bison)


add_executable(wcomp
  while.cc
  code_generator_implementation.cc
  implementation.cc
  interpreter_implementation.cc
  type_checker_implementation.cc
)
target_link_libraries(wcomp PRIVATE lexer parser)
target_include_directories(wcomp PRIVATE ".")
