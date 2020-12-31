#include "ast_dumper.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <variant>

static std::ostream &repeat(std::ostream &os, char input, size_t num) {
  std::fill_n(std::ostream_iterator<char>(os), num, input);
  return os;
}

std::ostream &ast_dumper::operator()(const expression &x) const noexcept {
  std::visit(*this, x);
  return os;
}

std::ostream &
ast_dumper::operator()(const number_expression &x) const noexcept {
  return os << x.get_value();
}

std::ostream &
ast_dumper::operator()(const boolean_expression &x) const noexcept {
  return os << (x.get_value() ? "true" : "false");
}

std::ostream &ast_dumper::operator()(const id_expression &x) const noexcept {
  return os << x.name;
}

std::ostream &ast_dumper::operator()(const binop_expression &x) const noexcept {
  os << '(';
  std::visit(*this, *x.left);
  os << ' ' << x.op << ' ';
  std::visit(*this, *x.right);
  os << ')';
  return os;
}

std::ostream &ast_dumper::operator()(const not_expression &x) const noexcept {
  os << "not ";
  std::visit(*this, *x.operand);
  return os;
}

std::ostream &ast_dumper::operator()(const statements &xs) const noexcept {
  for (const statement &x : xs)
    std::visit(*this, x);
  return os;
}

std::ostream &ast_dumper::operator()(const statement &x) const noexcept {
  std::visit(*this, x);
  return os;
}

std::ostream &
ast_dumper::operator()(const invalid_statement &x) const noexcept {
  assert(false && "Unreachabel!");
  return os;
}

std::ostream &ast_dumper::operator()(const assign_statement &x) const noexcept {
  repeat(os, ' ', indent) << x.left << " := ";
  std::visit(*this, *x.right);
  return os << '\n';
}

std::ostream &ast_dumper::operator()(const read_statement &x) const noexcept {
  return repeat(os, ' ', indent) << "read(" << x.id << ")\n";
}

std::ostream &ast_dumper::operator()(const write_statement &x) const noexcept {
  repeat(os, ' ', indent) << "write(";
  std::visit(*this, *x.value);
  return os << ")\n";
}

std::ostream &ast_dumper::operator()(const if_statement &x) const noexcept {
  ast_dumper sub_dumper{os, indent + 2};
  repeat(os, ' ', indent) << "if ";
  std::visit(*this, *x.condition);
  os << " then\n";
  sub_dumper(x.true_branch);
  if (!x.false_branch.empty()) {
    repeat(os, ' ', indent) << "else\n";
    sub_dumper(x.false_branch);
  }
  repeat(os, ' ', indent) << "endif\n";
  return os;
}

std::ostream &ast_dumper::operator()(const while_statement &x) const noexcept {
  ast_dumper sub_dumper{os, indent + 2};
  repeat(os, ' ', indent) << "while ";
  std::visit(*this, *x.condition);
  os << " do\n";
  sub_dumper(x.body);
  repeat(os, ' ', indent) << "done\n";
  return os;
}
