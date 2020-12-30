#include "implementation.hh"
#include <cstdlib>
#include <iostream>
#include <string_view>

mode current_mode;
long id = 0;

void error(int line, const std::string_view &msg) {
  std::cerr << "Line " << line << ": Error: " << msg << '\n';
  std::exit(1);
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

instruction::~instruction() noexcept = default;
assign_instruction::~assign_instruction() noexcept = default;
read_instruction::~read_instruction() noexcept = default;
write_instruction::~write_instruction() noexcept = default;
if_instruction::~if_instruction() noexcept = default;
while_instruction::~while_instruction() noexcept = default;
for_instruction::~for_instruction() noexcept = default;
