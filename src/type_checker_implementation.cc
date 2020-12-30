#include "implementation.hh"
#include <cassert>
#include <iostream>
#include <sstream>

std::map<std::string, symbol> symbol_table;

type number_expression::get_type() const { return natural; }

type boolean_expression::get_type() const { return boolean; }

void symbol::declare() {
  if (symbol_table.count(name) > 0) {
    error(line, std::string("Re-declared variable: ") + name);
  }
  symbol_table[name] = *this;
}

type id_expression::get_type() const {
  if (symbol_table.count(name) == 0) {
    error(line, std::string("Undefined variable: ") + name);
  }
  return symbol_table[name].symbol_type;
}

type operand_type(std::string_view op) {
  if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%" ||
      op == "<" || op == ">" || op == "<=" || op == ">=") {
    return natural;
  } else {
    return boolean;
  }
}

type return_type(std::string_view op) {
  if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
    return natural;
  } else {
    return boolean;
  }
}

type binop_expression::get_type() const {
  if (op == "=") {
    if (::get_type(*left) != ::get_type(*right)) {
      error(line, "Left and right operands of '=' have different types.");
    }
  } else {
    const type ty = operand_type(op);
    if (::get_type(*left) != ty) {
      error(line,
            std::string("Left operand of '") + op + "' has unexpected type.");
    }
    if (::get_type(*right) != ty) {
      error(line,
            std::string("Right operand of '") + op + "' has unexpected type.");
    }
  }
  return return_type(op);
}

type not_expression::get_type() const {
  if (::get_type(*operand) != boolean) {
    error(line, "Operand of 'not' is not boolean.");
  }
  return boolean;
}

void assign_instruction::type_check() {
  if (symbol_table.count(left) == 0) {
    error(get_line(), std::string("Undefined variable: ") + left);
  }
  if (symbol_table[left].symbol_type != ::get_type(*right)) {
    error(get_line(),
          "Left and right hand sides of assignment are of different types.");
  }
}

void read_instruction::type_check() {
  if (symbol_table.count(id) == 0) {
    error(get_line(), std::string("Undefined variable: ") + id);
  }
}

void write_instruction::type_check() {}

void if_instruction::type_check() {
  if (::get_type(*condition) != boolean) {
    error(get_line(), "Condition of 'if' instruction is not boolean.");
  }
  type_check_commands(true_branch);
  type_check_commands(false_branch);
}

void while_instruction::type_check() {
  if (::get_type(*condition) != boolean) {
    error(get_line(), "Condition of 'while' instruction is not boolean.");
  }
  type_check_commands(body);
}

void for_instruction::type_check() {
  if (symbol_table.count(loopvar) == 0) {
    error(get_line(), std::string("Undefined variable: ") + loopvar);
  }
  if (symbol_table[loopvar].symbol_type != natural) {
    error(get_line(), "The loop variable must have natural type.");
  }
  if (::get_type(*first) != natural) {
    error(get_line(),
          "Begin of the range of 'for' instruction is not natural.");
  }
  if (::get_type(*last) != natural) {
    error(get_line(), "End of the range of 'for' instruction is not natural.");
  }
  type_check_commands(body);
}

void type_check_commands(const commands_t &commands) {
  for (const auto &command : commands)
    command->type_check();
}
