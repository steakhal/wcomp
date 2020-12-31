#ifndef AST_DUMPER_H
#define AST_DUMPER_H

#include "expressions.h"
#include "statements.h"
#include "utility.h"

#include <iostream>

class ast_dumper {
  std::ostream &os;
  const unsigned indent;

public:
  explicit ast_dumper(std::ostream &os, unsigned initial_indent = 0)
      : os{os}, indent{initial_indent} {}

  std::ostream &operator()(const expression &x) const noexcept;
  std::ostream &operator()(const number_expression &x) const noexcept;
  std::ostream &operator()(const boolean_expression &x) const noexcept;
  std::ostream &operator()(const id_expression &x) const noexcept;
  std::ostream &operator()(const binop_expression &x) const noexcept;
  std::ostream &operator()(const not_expression &x) const noexcept;

  std::ostream &operator()(const statements &xs) const noexcept;
  std::ostream &operator()(const statement &x) const noexcept;
  std::ostream &operator()(const invalid_statement &x) const noexcept;
  std::ostream &operator()(const assign_statement &x) const noexcept;
  std::ostream &operator()(const read_statement &x) const noexcept;
  std::ostream &operator()(const write_statement &x) const noexcept;
  std::ostream &operator()(const if_statement &x) const noexcept;
  std::ostream &operator()(const while_statement &x) const noexcept;
};

#endif // AST_DUMPER_H
