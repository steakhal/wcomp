#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>

#include "utility.h"

enum type { boolean, natural };
using expression = std::variant<class number_expression,
                                class boolean_expression, class id_expression,
                                class binop_expression, class not_expression>;

class number_expression {
public:
  explicit number_expression(unsigned value) : value(value) {}

  unsigned value;
};
static_assert(std::is_copy_constructible_v<number_expression>);

class boolean_expression {
public:
  explicit boolean_expression(bool value) : value(value) {}

  bool value;
};
static_assert(std::is_copy_constructible_v<boolean_expression>);

class id_expression {
public:
  id_expression(int line, std::string name)
      : line(line), name(std::move(name)) {}

  int line;
  std::string name;
};
static_assert(std::is_copy_constructible_v<id_expression>);

class binop_expression {
public:
  binop_expression(int line, std::string op, std::unique_ptr<expression> left,
                   std::unique_ptr<expression> right)
      : line(line), op(std::move(op)), left(std::move(left)),
        right(std::move(right)) {}
  binop_expression(const binop_expression &other);
  binop_expression(binop_expression &&other) = default;
  binop_expression &operator=(const binop_expression &other);
  binop_expression &operator=(binop_expression &&other) = default;

  int line;
  std::string op;
  std::unique_ptr<expression> left;
  std::unique_ptr<expression> right;
};
static_assert(std::is_copy_constructible_v<binop_expression>);

class not_expression {
public:
  not_expression(int line, std::string op, std::unique_ptr<expression> operand)
      : line(line), op(std::move(op)), operand(std::move(operand)) {}
  not_expression(const not_expression &other);
  not_expression(not_expression &&other) = default;
  not_expression &operator=(const not_expression &other);
  not_expression &operator=(not_expression &&other) = default;

  int line;
  std::string op;
  std::unique_ptr<expression> operand;
};
static_assert(std::is_copy_constructible_v<not_expression>);

struct symbol {
  symbol() = default;
  symbol(int line, std::string name, type type)
      : line(line), name(std::move(name)), symbol_type(type) {}

  int line;
  std::string name;
  type symbol_type;
};

using symbols = std::map<std::string, symbol>;

#endif // EXPRESSIONS_H
