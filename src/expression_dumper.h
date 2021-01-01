#ifndef EXPRESSION_DUMPER_H
#define EXPRESSION_DUMPER_H

#include "expressions.h"

#include <iostream>

class expression_dumper {
  std::ostream &os;
  const unsigned indent;

public:
  explicit expression_dumper(std::ostream &os, unsigned initial_indent = 0)
      : os{os}, indent{initial_indent} {}

  std::ostream &operator()(const expression &x) const noexcept;
  std::ostream &operator()(const number_expression &x) const noexcept;
  std::ostream &operator()(const boolean_expression &x) const noexcept;
  std::ostream &operator()(const id_expression &x) const noexcept;
  std::ostream &operator()(const binop_expression &x) const noexcept;
  std::ostream &operator()(const not_expression &x) const noexcept;
};

#endif // EXPRESSION_DUMPER_H
