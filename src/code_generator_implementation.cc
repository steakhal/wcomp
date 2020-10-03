#include "implementation.hh"
#include <cassert>
#include <iostream>
#include <sstream>

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
  ss << "mov al," << (value ? 1 : 0) << std::endl;
  return ss.str();
}

std::string next_label() {
  std::stringstream ss;
  ss << "label" << id++;
  return ss.str();
}

std::string symbol::get_code() {
  std::stringstream ss;
  ss << label << ": resb " << get_size() << "\t; variable: " << name
     << std::endl;
  return ss.str();
}

int symbol::get_size() {
  if (symbol_type == boolean) {
    return 1;
  } else {
    return 4;
  }
}

std::string get_register(type t) {
  if (t == boolean) {
    return "al";
  } else {
    return "eax";
  }
}

std::string id_expression::get_code() const {
  if (symbol_table.count(name) == 0) {
    error(line, std::string("Undefined variable: ") + name);
  }

  if (is_constant_expression())
    return store_into_eax(get_value());
  return std::string("mov eax,[") + symbol_table[name].label + "]\n";
}

std::string operator_code(std::string op) {
  std::stringstream ss;
  if (op == "+") {
    ss << "add eax,ecx" << std::endl;
  } else if (op == "-") {
    ss << "sub eax,ecx" << std::endl;
  } else if (op == "*") {
    ss << "xor edx,edx" << std::endl;
    ss << "mul ecx" << std::endl;
  } else if (op == "/") {
    ss << "xor edx,edx" << std::endl;
    ss << "div ecx" << std::endl;
  } else if (op == "%") {
    ss << "xor edx,edx" << std::endl;
    ss << "div ecx" << std::endl;
    ss << "mov eax,edx" << std::endl;
  } else if (op == "<") {
    ss << "cmp eax,ecx" << std::endl;
    ss << "mov al,0" << std::endl;
    ss << "mov cx,1" << std::endl;
    ss << "cmovb ax,cx" << std::endl;
  } else if (op == "<=") {
    ss << "cmp eax,ecx" << std::endl;
    ss << "mov al,0" << std::endl;
    ss << "mov cx,1" << std::endl;
    ss << "cmovbe ax,cx" << std::endl;
  } else if (op == ">") {
    ss << "cmp eax,ecx" << std::endl;
    ss << "mov al,0" << std::endl;
    ss << "mov cx,1" << std::endl;
    ss << "cmova ax,cx" << std::endl;
  } else if (op == ">=") {
    ss << "cmp eax,ecx" << std::endl;
    ss << "mov al,0" << std::endl;
    ss << "mov cx,1" << std::endl;
    ss << "cmovae ax,cx" << std::endl;
  } else if (op == "and") {
    ss << "cmp al,1" << std::endl;
    ss << "cmove ax,cx" << std::endl;
  } else if (op == "or") {
    ss << "cmp al,0" << std::endl;
    ss << "cmove ax,cx" << std::endl;
  } else {
    error(-1, std::string("Bug: Unsupported binary operator: ") + op);
  }
  return ss.str();
}

std::string eq_code(type t) {
  std::stringstream ss;
  if (t == natural) {
    ss << "cmp eax,ecx" << std::endl;
  } else {
    ss << "cmp al,cl" << std::endl;
  }
  ss << "mov al,0" << std::endl;
  ss << "mov cx,1" << std::endl;
  ss << "cmove ax,cx" << std::endl;
  return ss.str();
}

std::string binop_expression::get_code() const {
  if (is_constant_expression())
    return store_into_eax(get_value());

  std::stringstream ss;
  ss << (left->is_constant_expression() ? store_into_eax(left->get_value())
                                        : left->get_code());
  ss << "push eax" << std::endl;
  ss << (right->is_constant_expression() ? store_into_eax(right->get_value())
                                         : right->get_code());
  ss << "mov ecx,eax" << std::endl;
  ss << "pop eax" << std::endl;
  ss << (op == "=" ? eq_code(left->get_type()) : operator_code(op));
  return ss.str();
}

std::string triop_expression::get_code() const {
  if (is_constant_expression())
    return store_into_eax(get_value());

  std::string else_label = next_label();
  std::string end_label = next_label();
  std::stringstream ss;

  if (cond->is_constant_expression()) {
    if (cond->get_value())
      return left->is_constant_expression() ? store_into_eax(left->get_value())
                                            : left->get_code();
    return right->is_constant_expression() ? store_into_eax(right->get_value())
                                           : right->get_code();
  }
  ss << cond->get_code();
  ss << "cmp al,1" << std::endl;
  ss << "jne near " << else_label << std::endl;
  ss << (left->is_constant_expression() ? store_into_eax(left->get_value())
                                        : left->get_code());
  ss << "jmp " << end_label << std::endl;
  ss << else_label << ":" << std::endl;
  ss << right->is_constant_expression() ? store_into_eax(right->get_value())
                                        : right->get_code();
  ss << end_label << ":" << std::endl;
  return ss.str();
}

std::string not_expression::get_code() const {
  if (is_constant_expression())
    return store_into_eax(get_value());

  std::stringstream ss;
  ss << operand->get_code();
  ss << "xor al,1" << std::endl;
  return ss.str();
}

std::string assign_instruction::get_code() {
  if (right->is_constant_expression() && do_constant_propagation) {
    execute(); // Store the evaluated value, so this variable becomes a constant
               // expression.
    return store_into_eax(right->get_value());
  }

  std::stringstream ss;
  ss << right->get_code();
  ss << "mov [" + symbol_table[left].label + "],"
     << get_register(symbol_table[left].symbol_type) << std::endl;
  return ss.str();
}

std::string simultan_assign_instruction::get_code() {
  // TODO: Implement constant folding.

  std::stringstream ss;
  for (auto *expr : *right) {
    ss << expr->get_code();
    ss << "push eax\n";
  }
  for (auto it = left->rbegin(); it != left->rend(); ++it) {
    auto &sym = symbol_table[*it];
    ss << "pop eax\n";
    ss << "mov [" + sym.label + "]," << get_register(sym.symbol_type)
       << std::endl;
  }
  return ss.str();
}

std::string get_type_name(type t) {
  if (t == boolean) {
    return "boolean";
  } else {
    return "natural";
  }
}

std::string read_instruction::get_code() {
  type t = symbol_table[id].symbol_type;
  std::stringstream ss;
  ss << "call read_" << get_type_name(t) << std::endl;
  ss << "mov [" << symbol_table[id].label << "]," << get_register(t)
     << std::endl;
  return ss.str();
}

std::string write_instruction::get_code() {
  std::stringstream ss;
  ss << exp->get_code();
  if (exp_type == boolean) {
    ss << "and eax,1" << std::endl;
  }
  ss << "push eax" << std::endl;
  ss << "call write_" << get_type_name(exp_type) << std::endl;
  ss << "add esp,4" << std::endl;
  return ss.str();
}

std::string if_instruction::get_code() {
  if (condition->is_constant_expression()) {
    std::stringstream ss;
    generate_code_of_commands(ss, condition->get_value() ? true_branch
                                                         : false_branch);
    return ss.str();
  }

  std::string else_label = next_label();
  std::string end_label = next_label();
  std::stringstream ss;
  ss << condition->get_code();
  ss << "cmp al,1" << std::endl;
  ss << "jne near " << else_label << std::endl;
  generate_code_of_commands(ss, true_branch);
  ss << "jmp " << end_label << std::endl;
  ss << else_label << ":" << std::endl;
  generate_code_of_commands(ss, false_branch);
  ss << end_label << ":" << std::endl;
  return ss.str();
}

std::string while_instruction::get_code() {
  // Elide the loop completely if possible.
  if (condition->is_constant_expression() && condition->get_value() == false) {
    return "";
  }
  save_and_restore<bool> Guard(do_constant_propagation, false);
  std::string begin_label = next_label();
  std::string end_label = next_label();
  std::stringstream ss;
  ss << begin_label << ":" << std::endl;
  ss << condition->get_code();
  ss << "cmp al,1" << std::endl;
  ss << "jne near " << end_label << std::endl;
  generate_code_of_commands(ss, body);
  ss << "jmp " << begin_label << std::endl;
  ss << end_label << ":" << std::endl;
  return ss.str();
}

std::string for_instruction::get_code() {
  save_and_restore<bool> Guard(do_constant_propagation, false);

  std::stringstream ss;
  std::string begin_label = next_label();
  std::string end_label = next_label();

  // calculate first and store to i
  ss << first->get_code();
  ss << "mov [" + symbol_table[loopvar].label + "],eax\n";
  ss << "dec eax\n";

  ss << begin_label << ":\n";
  ss << "inc eax\n";                                       // increment i
  ss << "mov [" + symbol_table[loopvar].label + "],eax\n"; // store i
  ss << "push eax\n";                                      // push i

  ss << last->get_code();
  ss << "mov ecx,eax\n"; // store end into ecx
  ss << "pop eax\n";     // pop i into eax

  ss << operator_code("<"); // eax < ecx

  ss << "cmp al,1\n";
  ss << "jne near " << end_label << '\n';
  generate_code_of_commands(ss, body);
  ss << "mov eax,[" + symbol_table[loopvar].label + "]\n"; // load i
  ss << "jmp " << begin_label << '\n';
  ss << end_label << ":\n";

  return ss.str();
}

void generate_code_of_commands(std::ostream &out,
                               std::list<instruction *> *commands) {
  if (!commands) {
    return;
  }

  std::list<instruction *>::iterator it;
  for (it = commands->begin(); it != commands->end(); ++it) {
    out << (*it)->get_code();
  }
}

void generate_code(std::list<instruction *> *commands) {
  std::cout << "global main" << std::endl;
  std::cout << "extern write_natural" << std::endl;
  std::cout << "extern read_natural" << std::endl;
  std::cout << "extern write_boolean" << std::endl;
  std::cout << "extern read_boolean" << std::endl;
  std::cout << std::endl;
  std::cout << "section .bss" << std::endl;
  std::map<std::string, symbol>::iterator it;
  for (it = symbol_table.begin(); it != symbol_table.end(); ++it) {
    std::cout << it->second.get_code();
  }
  std::cout << std::endl;
  std::cout << "section .text" << std::endl;
  std::cout << "main:" << std::endl;
  generate_code_of_commands(std::cout, commands);
  std::cout << "xor eax,eax" << std::endl;
  std::cout << "ret" << std::endl;
}
