#include "expressions.h"
#include "statements.h"
#include "utility.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string_view>

#include <FlexLexer.h>

binop_expression::binop_expression(const binop_expression &other)
    : line{other.line}, op{other.op} {
  if (other.left.get() != nullptr) {
    auto subexpr_clone = std::make_unique<expression>(*other.left);
    left = std::move(subexpr_clone);
  }
  if (other.right.get() != nullptr) {
    auto subexpr_clone = std::make_unique<expression>(*other.right);
    right = std::move(subexpr_clone);
  }
}

binop_expression &binop_expression::operator=(const binop_expression &other) {
  line = other.line;
  op = other.op;
  left = std::make_unique<expression>(*other.left);
  right = std::make_unique<expression>(*other.right);
  return *this;
}

not_expression::not_expression(const not_expression &other)
    : line{other.line}, op{other.op} {
  if (other.operand.get() != nullptr) {
    auto subexpr_clone = std::make_unique<expression>(*other.operand);
    operand = std::move(subexpr_clone);
  }
}

not_expression &not_expression::operator=(const not_expression &other) {
  line = other.line;
  op = other.op;
  operand = std::make_unique<expression>(*other.operand);
  return *this;
}

void unreachable() { assert(false && "Unreachable!"); }

void error(int line, const std::string_view &msg) {
  std::cerr << "Line " << line << ": Error: " << msg << '\n';
  std::exit(1);
}

std::ostream &repeat(std::ostream &os, char input, size_t num) {
  std::fill_n(std::ostream_iterator<char>(os), num, input);
  return os;
}
