#include "expressions.h"
#include "statements.h"
#include "utility.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string_view>

static std::string store_into_eax(unsigned value) {
  std::stringstream ss;
  ss << "mov eax," << value << '\n';
  return ss.str();
}

std::string number_expression::get_code() const {
  return store_into_eax(value);
}

std::string boolean_expression::get_code() const {
  std::stringstream ss;
  ss << "mov al," << (value ? 1 : 0) << '\n';
  return ss.str();
}

std::string next_label() {
  std::stringstream ss;
  static long id;
  ss << "label" << id++;
  return ss.str();
}

std::string symbol::get_code() const {
  std::stringstream ss;
  ss << label << ": resb " << get_size() << "\t; variable: " << name << '\n';
  return ss.str();
}

int symbol::get_size() const { return symbol_type == boolean ? 1 : 4; }

static std::string_view get_register(type t) {
  return t == boolean ? "al" : "eax";
}

std::string id_expression::get_code() const {
  if (symbol_table.count(name) == 0) {
    error(line, std::string("Undefined variable: ") + name);
  }

  if (is_constant_expression())
    return store_into_eax(get_value());
  return std::string("mov eax,[") + symbol_table[name].label + "]\n";
}

static std::string operator_code(std::string_view op) {
  std::stringstream ss;
  if (op == "+") {
    ss << "add eax,ecx\n";
  } else if (op == "-") {
    ss << "sub eax,ecx\n";
  } else if (op == "*") {
    ss << "xor edx,edx\n";
    ss << "mul ecx\n";
  } else if (op == "/") {
    ss << "xor edx,edx\n";
    ss << "div ecx\n";
  } else if (op == "%") {
    ss << "xor edx,edx\n";
    ss << "div ecx\n";
    ss << "mov eax,edx\n";
  } else if (op == "<") {
    ss << "cmp eax,ecx\n";
    ss << "mov al,0\n";
    ss << "mov cx,1\n";
    ss << "cmovb ax,cx\n";
  } else if (op == "<=") {
    ss << "cmp eax,ecx\n";
    ss << "mov al,0\n";
    ss << "mov cx,1\n";
    ss << "cmovbe ax,cx\n";
  } else if (op == ">") {
    ss << "cmp eax,ecx\n";
    ss << "mov al,0\n";
    ss << "mov cx,1\n";
    ss << "cmova ax,cx\n";
  } else if (op == ">=") {
    ss << "cmp eax,ecx\n";
    ss << "mov al,0\n";
    ss << "mov cx,1\n";
    ss << "cmovae ax,cx\n";
  } else if (op == "and") {
    ss << "cmp al,1\n";
    ss << "cmove ax,cx\n";
  } else if (op == "or") {
    ss << "cmp al,0\n";
    ss << "cmove ax,cx\n";
  } else {
    error(-1,
          std::string("Bug: Unsupported binary operator: ") + std::string(op));
  }
  return ss.str();
}

static std::string eq_code(type t) {
  std::stringstream ss;
  if (t == natural) {
    ss << "cmp eax,ecx\n";
  } else {
    ss << "cmp al,cl\n";
  }
  ss << "mov al,0\n";
  ss << "mov cx,1\n";
  ss << "cmove ax,cx\n";
  return ss.str();
}

std::string binop_expression::get_code() const {
  if (is_constant_expression())
    return store_into_eax(get_value());

  std::stringstream ss;
  ss << (::is_constant_expression(*left) ? store_into_eax(::get_value(*left))
                                         : ::get_code(*left));
  ss << "push eax\n";
  ss << (::is_constant_expression(*right) ? store_into_eax(::get_value(*right))
                                          : ::get_code(*right));
  ss << "mov ecx,eax\n";
  ss << "pop eax\n";
  ss << (op == "=" ? eq_code(::get_type(*left)) : operator_code(op));
  return ss.str();
}

std::string not_expression::get_code() const {
  if (is_constant_expression())
    return store_into_eax(get_value());

  std::stringstream ss;
  ss << ::get_code(*operand);
  ss << "xor al,1\n";
  return ss.str();
}

std::string assign_statement::get_code() const {
  if (::is_constant_expression(*right) && do_constant_propagation) {
    execute(); // Store the evaluated value, so this variable becomes a constant
               // expression.
    return store_into_eax(::get_value(*right));
  }

  std::stringstream ss;
  ss << ::get_code(*right);
  ss << "mov [" + symbol_table[left].label + "],"
     << get_register(symbol_table[left].symbol_type) << '\n';
  return ss.str();
}

static std::string_view get_type_name(type t) {
  return t == boolean ? "boolean" : "natural";
}

std::string read_statement::get_code() const {
  const type ty = symbol_table[id].symbol_type;
  std::stringstream ss;
  ss << "call read_" << get_type_name(ty) << '\n';
  ss << "mov [" << symbol_table[id].label << "]," << get_register(ty) << '\n';
  return ss.str();
}

std::string write_statement::get_code() const {
  const type ty = ::get_type(*value);
  std::stringstream ss;
  ss << ::get_code(*value);
  if (ty == boolean) {
    ss << "and eax,1\n";
  }
  ss << "push eax\n";
  ss << "call write_" << get_type_name(ty) << '\n';
  ss << "add esp,4\n";
  return ss.str();
}

std::string if_statement::get_code() const {
  if (::is_constant_expression(*condition)) {
    return ::get_code(::get_value(*condition) ? true_branch : false_branch);
  }

  std::string else_label = next_label();
  std::string end_label = next_label();
  std::stringstream ss;
  ss << ::get_code(*condition);
  ss << "cmp al,1\n";
  ss << "jne near " << else_label << '\n';
  ss << ::get_code(true_branch);
  ss << "jmp " << end_label << '\n';
  ss << else_label << ":\n";
  ss << ::get_code(false_branch);
  ss << end_label << ":\n";
  return ss.str();
}

std::string while_statement::get_code() const {
  // Elide the loop completely if possible.
  if (::is_constant_expression(*condition) &&
      ::get_value(*condition) == false) {
    return "";
  }
  save_and_restore<bool> Guard(do_constant_propagation, false);
  std::string begin_label = next_label();
  std::string end_label = next_label();
  std::stringstream ss;
  ss << begin_label << ":\n";
  ss << ::get_code(*condition);
  ss << "cmp al,1\n";
  ss << "jne near " << end_label << '\n';
  ss << ::get_code(body);
  ss << "jmp " << begin_label << '\n';
  ss << end_label << ":\n";
  return ss.str();
}
