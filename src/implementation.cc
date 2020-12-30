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

expression::~expression() noexcept = default;
number_expression::~number_expression() noexcept = default;
boolean_expression::~boolean_expression() noexcept = default;
id_expression::~id_expression() noexcept = default;
binop_expression::~binop_expression() noexcept = default;
not_expression::~not_expression() noexcept = default;

instruction::~instruction() noexcept = default;
assign_instruction::~assign_instruction() noexcept = default;
read_instruction::~read_instruction() noexcept = default;
write_instruction::~write_instruction() noexcept = default;
if_instruction::~if_instruction() noexcept = default;
while_instruction::~while_instruction() noexcept = default;
for_instruction::~for_instruction() noexcept = default;
