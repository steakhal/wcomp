#ifndef IMPLEMENTATION_HH
#define IMPLEMENTATION_HH

#include <cassert>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

enum mode { compiler, interpreter };
extern mode current_mode;

enum type { boolean, natural };

[[noreturn]] static void unreachable() { assert(false && "Unreachable!"); }
[[noreturn]] void error(int line, const std::string_view &msg);

using expression = std::variant<class number_expression,
                                class boolean_expression, class id_expression,
                                class binop_expression, class not_expression>;

using statement = std::variant<class invalid_statement, class assign_statement,
                               class read_statement, class write_statement,
                               class if_statement, class while_statement>;
using statements = std::vector<statement>;

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

class id_expression {
public:
  id_expression(int line, std::string name)
      : line(line), name(std::move(name)) {}
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  int line;
  std::string name;
};

class binop_expression {
public:
  binop_expression(int line, std::string op, std::unique_ptr<expression> left,
                   std::unique_ptr<expression> right)
      : line(line), op(std::move(op)), left(std::move(left)),
        right(std::move(right)) {}
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  int line;
  std::string op;
  std::unique_ptr<expression> left;
  std::unique_ptr<expression> right;
};

class not_expression {
public:
  not_expression(int line, std::string op, std::unique_ptr<expression> operand)
      : line(line), op(std::move(op)), operand(std::move(operand)) {}
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  int line;
  std::string op;
  std::unique_ptr<expression> operand;
};

class statement_base {
public:
  explicit statement_base(int line) : line(line) {}
  int get_line() const { return line; }

private:
  int line;
};

class invalid_statement {
public:
  void type_check() const { unreachable(); }
  std::string get_code() const { unreachable(); }
  void execute() const { unreachable(); }
};

class assign_statement : public statement_base {
public:
  assign_statement(int line, std::string left,
                   std::unique_ptr<expression> right)
      : statement_base(line), left(std::move(left)), right(std::move(right)) {}
  void type_check() const;
  std::string get_code() const;
  void execute() const;

private:
  std::string left;
  std::unique_ptr<expression> right;
};

class read_statement : public statement_base {
public:
  read_statement(int line, std::string id)
      : statement_base(line), id(std::move(id)) {}
  void type_check() const;
  std::string get_code() const;
  void execute() const;

private:
  std::string id;
};

class write_statement : public statement_base {
public:
  write_statement(int line, std::unique_ptr<expression> value)
      : statement_base(line), value(std::move(value)) {}
  void type_check() const;
  std::string get_code() const;
  void execute() const;

private:
  std::unique_ptr<expression> value;
};

class if_statement : public statement_base {
public:
  if_statement(int line, std::unique_ptr<expression> condition,
               statements true_branch, statements false_branch)
      : statement_base(line), condition(std::move(condition)),
        true_branch(std::move(true_branch)),
        false_branch(std::move(false_branch)) {}
  void type_check() const;
  std::string get_code() const;
  void execute() const;

private:
  std::unique_ptr<expression> condition;
  statements true_branch;
  statements false_branch;
};

class while_statement : public statement_base {
public:
  while_statement(int line, std::unique_ptr<expression> condition,
                  statements body)
      : statement_base(line), condition(std::move(condition)),
        body(std::move(body)) {}
  void type_check() const;
  std::string get_code() const;
  void execute() const;

private:
  std::unique_ptr<expression> condition;
  statements body;
};

std::string get_code(const expression &expr);
unsigned get_value(const expression &expr);
bool is_constant_expression(const expression &expr);
type get_type(const expression &expr);

void type_check(const statement &stmt);
std::string get_code(const statement &stmt);
void execute(const statement &stmt);

void type_check(const statements &stmts);
std::string get_code(const statements &stmts);
void execute(const statements &stmts);

extern bool do_constant_propagation;

template <typename T> class save_and_restore {
  T &place;
  T original;

public:
  save_and_restore(T &x, T new_value) : place(x), original(x) {
    place = new_value;
  }
  ~save_and_restore() { place = original; }
};

#endif // IMPLEMENTATION_HH
