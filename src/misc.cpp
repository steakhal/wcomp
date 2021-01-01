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

void unreachable() { assert(false && "Unreachable!"); }

void error(int line, const std::string_view &msg) {
  std::cerr << "Line " << line << ": Error: " << msg << '\n';
  std::exit(1);
}

std::ostream &repeat(std::ostream &os, char input, size_t num) {
  std::fill_n(std::ostream_iterator<char>(os), num, input);
  return os;
}

std::string get_code(const expression &expr) {
  return std::visit([](auto &&arg) { return arg.get_code(); }, expr);
}
unsigned get_value(const expression &expr) {
  return std::visit([](auto &&arg) { return arg.get_value(); }, expr);
}
bool is_constant_expression(const expression &expr) {
  return std::visit([](auto &&arg) { return arg.is_constant_expression(); },
                    expr);
}
type get_type(const expression &expr) {
  return std::visit([](auto &&arg) { return arg.get_type(); }, expr);
}

void type_check(const statement &stmt) {
  std::visit([](auto &&arg) { return arg.type_check(); }, stmt);
}
std::string get_code(const statement &stmt) {
  return std::visit([](auto &&arg) { return arg.get_code(); }, stmt);
}

void execute(const statement &stmt) {
  std::visit([](auto &&arg) { return arg.execute(); }, stmt);
}

void type_check(const statements &stmts) {
  for (const statement &stmt : stmts) {
    std::visit([](auto &&arg) { arg.type_check(); }, stmt);
  }
}

std::string get_code(const statements &stmts) {
  std::stringstream ss;
  for (const statement &stmt : stmts) {
    std::visit([&ss](auto &&arg) { ss << arg.get_code(); }, stmt);
  }
  return ss.str();
}
void execute(const statements &stmts) {
  for (const statement &stmt : stmts) {
    std::visit([](auto &&arg) { return arg.execute(); }, stmt);
  }
}
