#ifndef AST_DUMPER_H
#define AST_DUMPER_H

#include "expression_dumper.h"
#include "expressions.h"
#include "statements.h"

#include <iostream>

class ast_dumper : private expression_dumper {
  std::ostream &os;
  const unsigned indent;

public:
  explicit ast_dumper(std::ostream &os, unsigned initial_indent = 0)
      : expression_dumper{os, initial_indent}, os{os}, indent{initial_indent} {}

  using expression_dumper::operator();

  std::ostream &operator()(const ast &x) const noexcept;
  std::ostream &operator()(const symbols &xs) const noexcept;
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
