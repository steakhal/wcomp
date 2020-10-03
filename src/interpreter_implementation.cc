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

unsigned boolean_expression::get_value() const { return (unsigned)value; }

bool id_expression::is_constant_expression() const {
  return value_table.count(
      name); // If we have a constant value associated with it, true.
}

unsigned id_expression::get_value() const {
  if (value_table.count(name) == 0) {
    error(line, std::string("Variable has not been initialized: ") + name);
  }
  return value_table[name];
}

bool binop_expression::is_constant_expression() const {
  return left->is_constant_expression() && right->is_constant_expression();
}

unsigned binop_expression::get_value() const {
  int left_value = left->get_value();
  int right_value = right->get_value();
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
  } else {
    error(line, std::string("Unknown operator: ") + op);
  }
}

bool triop_expression::is_constant_expression() const {
  return cond->is_constant_expression() && left->is_constant_expression() &&
         right->is_constant_expression();
}

unsigned triop_expression::get_value() const {
  return cond->get_value() ? left->get_value() : right->get_value();
}

bool not_expression::is_constant_expression() const {
  return operand->is_constant_expression();
}

unsigned not_expression::get_value() const {
  return !(bool)(operand->get_value());
}

void assign_instruction::execute() { value_table[left] = right->get_value(); }

void simultan_assign_instruction::execute() {
  std::vector<unsigned> tmps;
  tmps.reserve(right->size());
  for (auto *expr : *right) {
    tmps.push_back(expr->get_value());
  }

  int i = 0;
  for (const auto &vname : *left) {
    value_table[vname] = tmps[i++];
  }
}

void read_instruction::execute() {
  std::string input_line;
  getline(std::cin, input_line);
  if (symbol_table[id].symbol_type == natural) {
    std::stringstream ss(input_line);
    unsigned input;
    ss >> input;
    value_table[id] = input;
  } else if (symbol_table[id].symbol_type == boolean) {
    if (input_line == "true") {
      value_table[id] = 1;
    } else {
      value_table[id] = 0;
    }
  }
}

void write_instruction::execute() {
  if (exp_type == natural) {
    std::cout << exp->get_value() << std::endl;
  } else if (exp_type == boolean) {
    if (exp->get_value()) {
      std::cout << "true" << std::endl;
    } else {
      std::cout << "false" << std::endl;
    }
  }
}

void if_instruction::execute() {
  if (condition->get_value()) {
    execute_commands(true_branch);
  } else {
    execute_commands(false_branch);
  }
}

void while_instruction::execute() {
  std::list<instruction *>::iterator it;
  while (condition->get_value()) {
    execute_commands(body);
  }
}

void for_instruction::execute() {
  unsigned &var = value_table[loopvar];
  for (var = first->get_value(); var < last->get_value(); ++var) {
    execute_commands(body);
  }
}

void execute_commands(std::list<instruction *> *commands) {
  if (!commands) {
    return;
  }

  std::list<instruction *>::iterator it;
  for (it = commands->begin(); it != commands->end(); ++it) {
    (*it)->execute();
  }
}
