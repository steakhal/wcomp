#include "expression_dumper.h"
#include "utility.h"

#include <algorithm>
#include <iostream>
#include <variant>

std::ostream &
expression_dumper::operator()(const expression &x) const noexcept {
  std::visit(*this, x);
  return os;
}

std::ostream &
expression_dumper::operator()(const number_expression &x) const noexcept {
  return os << x.value;
}

std::ostream &
expression_dumper::operator()(const boolean_expression &x) const noexcept {
  return os << (x.value ? "true" : "false");
}

std::ostream &
expression_dumper::operator()(const id_expression &x) const noexcept {
  return os << x.name;
}

std::ostream &
expression_dumper::operator()(const binop_expression &x) const noexcept {
  os << '(';
  std::visit(*this, *x.left);
  os << ' ' << x.op << ' ';
  std::visit(*this, *x.right);
  os << ')';
  return os;
}

std::ostream &
expression_dumper::operator()(const not_expression &x) const noexcept {
  os << "not ";
  std::visit(*this, *x.operand);
  return os;
}
