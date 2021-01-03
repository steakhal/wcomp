#include "ast_dumper.h"
#include "utility.h"

#include <cassert>
#include <iostream>
#include <variant>

std::ostream &ast_dumper::operator()(const ast &x) const noexcept {
  ast_dumper sub_dumper{os, indent + 2};
  os << "program " << x.prog_name << '\n';
  sub_dumper(x.syms);
  os << "begin\n";
  sub_dumper(x.stmts);
  os << "end\n";
  return os;
}

std::ostream &ast_dumper::operator()(const symbols &xs) const noexcept {
  for (const auto &pair : xs) {
    const symbol &sym = pair.second;
    repeat(os, ' ', indent)
        << (sym.symbol_type == boolean ? "boolean" : "natural") << ' '
        << sym.name << '\n';
  }
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
