#include "implementation.hh"
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

std::map<std::string, unsigned> value_table;

bool do_constant_propagation = true;

bool number_expression::is_constant_expression() const { return true; }

unsigned number_expression::get_value() const { return value; }

bool boolean_expression::is_constant_expression() const { return true; }

unsigned boolean_expression::get_value() const {
  return static_cast<unsigned>(value);
}

bool id_expression::is_constant_expression() const {
  // If we have a constant value associated with it, true.
  return value_table.count(name);
}

unsigned id_expression::get_value() const {
  if (value_table.count(name) == 0)
    error(line, std::string("Variable has not been initialized: ") + name);
  return value_table[name];
}

bool binop_expression::is_constant_expression() const {
  return ::is_constant_expression(*left) && ::is_constant_expression(*right);
}

unsigned binop_expression::get_value() const {
  const unsigned left_value = ::get_value(*left);
  const unsigned right_value = ::get_value(*right);
  if (op == "+") {
    return left_value + right_value;
  } else if (op == "-") {
    return left_value - right_value;
  } else if (op == "*") {
    return left_value * right_value;
  } else if (op == "/") {
    return left_value / right_value;
  } else if (op == "%") {
    return left_value % right_value;
  } else if (op == "<") {
    return left_value < right_value;
  } else if (op == ">") {
    return left_value > right_value;
  } else if (op == "<=") {
    return left_value <= right_value;
  } else if (op == ">=") {
    return left_value >= right_value;
  } else if (op == "and") {
    return left_value && right_value;
  } else if (op == "or") {
    return left_value || right_value;
  } else if (op == "=") {
    return left_value == right_value;
  }
  error(line, std::string("Unknown operator: ") + op);
}

bool not_expression::is_constant_expression() const {
  return ::is_constant_expression(*operand);
}

unsigned not_expression::get_value() const {
  return !static_cast<bool>(::get_value(*operand));
}

void assign_statement::execute() const {
  value_table[left] = ::get_value(*right);
}

void read_statement::execute() const {
  std::string input_line;
  std::getline(std::cin, input_line);
  if (symbol_table[id].symbol_type == natural) {
    std::stringstream ss(input_line);
    unsigned input;
    ss >> input;
    value_table[id] = input;
  } else if (symbol_table[id].symbol_type == boolean) {
    value_table[id] = input_line == "true";
  }
}

void write_statement::execute() const {
  const auto val = ::get_value(*value);
  if (::get_type(*value) == natural)
    std::cout << val << '\n';
  else
    std::cout << (val ? "true\n" : "false\n");
}

void if_statement::execute() const {
  ::execute(::get_value(*condition) ? true_branch : false_branch);
}

void while_statement::execute() const {
  while (::get_value(*condition))
    ::execute(body);
}
