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

std::string get_code(const expression &expr);
unsigned get_value(const expression &expr);
bool is_constant_expression(const expression &expr);
type get_type(const expression &expr);

class number_expression {
public:
  explicit number_expression(unsigned value) : value(value) {}
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  unsigned value;
};
static_assert(std::is_copy_constructible_v<number_expression>);

class boolean_expression {
public:
  explicit boolean_expression(bool value) : value(value) {}
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  bool value;
};
static_assert(std::is_copy_constructible_v<boolean_expression>);

class id_expression {
public:
  id_expression(int line, std::string name)
      : line(line), name(std::move(name)) {}
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

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

  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

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

  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

  int line;
  std::string op;
  std::unique_ptr<expression> operand;
};
static_assert(std::is_copy_constructible_v<not_expression>);

/////// symbol ///////////
extern std::string next_label();

struct symbol {
  symbol() = default;
  symbol(int line, std::string name, type type)
      : line(line), name(std::move(name)), symbol_type(type),
        label(next_label()) {}
  void declare() const;
  std::string get_code() const;
  int get_size() const;
  int line;
  std::string name;
  type symbol_type;
  std::string label;
};

extern std::map<std::string, symbol> symbol_table;
extern std::map<std::string, unsigned> value_table;

#endif // EXPRESSIONS_H
