#ifndef IMPLEMENTATION_HH
#define IMPLEMENTATION_HH

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

enum mode { compiler, interpreter };

extern mode current_mode;

enum type { boolean, natural };

[[noreturn]] void error(int line, const std::string_view &msg);

class expression {
public:
  virtual type get_type() const = 0;
  virtual ~expression() noexcept = 0;
  virtual bool is_constant_expression() const = 0;
  virtual std::string get_code() const = 0;
  virtual unsigned get_value() const = 0;
};

class number_expression : public expression {
public:
  number_expression(unsigned value) : value(value) {}
  ~number_expression() noexcept override;
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  unsigned value;
};

class boolean_expression : public expression {
public:
  boolean_expression(bool value) : value(value) {}
  ~boolean_expression() noexcept override;
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  bool value;
};

extern long id;

extern std::string next_label();

struct symbol {
  symbol() = default;
  symbol(int line, std::string name, type type)
      : line(line), name(std::move(name)), symbol_type(type),
        label(next_label()) {}
  void declare();
  std::string get_code();
  int get_size();
  int line;
  std::string name;
  type symbol_type;
  std::string label;
};

extern std::map<std::string, symbol> symbol_table;
extern std::map<std::string, unsigned> value_table;

class id_expression : public expression {
public:
  id_expression(int line, std::string name)
      : line(line), name(std::move(name)) {}
  ~id_expression() noexcept override;
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  int line;
  std::string name;
};

class binop_expression : public expression {
public:
  binop_expression(int line, std::string op, std::unique_ptr<expression> left,
                   std::unique_ptr<expression> right)
      : line(line), op(std::move(op)), left(std::move(left)),
        right(std::move(right)) {}
  ~binop_expression() noexcept override;
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

class triop_expression : public expression {
public:
  triop_expression(int line, std::string op, std::unique_ptr<expression> cond,
                   std::unique_ptr<expression> left,
                   std::unique_ptr<expression> right)
      : line(line), op(std::move(op)), cond(std::move(cond)),
        left(std::move(left)), right(std::move(right)) {}
  ~triop_expression() noexcept override;
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  int line;
  std::string op;
  std::unique_ptr<expression> cond;
  std::unique_ptr<expression> left;
  std::unique_ptr<expression> right;
};

class not_expression : public expression {
public:
  not_expression(int line, std::string op, std::unique_ptr<expression> operand)
      : line(line), op(std::move(op)), operand(std::move(operand)) {}
  ~not_expression() noexcept override;
  type get_type() const;
  bool is_constant_expression() const;
  std::string get_code() const;
  unsigned get_value() const;

private:
  int line;
  std::string op;
  std::unique_ptr<expression> operand;
};

class instruction {
public:
  explicit instruction(int line) : line(line) {}
  virtual ~instruction() = 0;
  virtual void type_check() = 0;
  virtual std::string get_code() = 0;
  virtual void execute() = 0;
  int get_line() const { return line; }

private:
  int line;
};

class assign_instruction : public instruction {
public:
  assign_instruction(int line, std::string left,
                     std::unique_ptr<expression> right)
      : instruction(line), left(std::move(left)), right(std::move(right)) {}
  ~assign_instruction() noexcept override;
  void type_check();
  std::string get_code();
  void execute();

private:
  std::string left;
  std::unique_ptr<expression> right;
};

class read_instruction : public instruction {
public:
  read_instruction(int line, std::string id)
      : instruction(line), id(std::move(id)) {}
  ~read_instruction() noexcept override;
  void type_check();
  std::string get_code();
  void execute();

private:
  std::string id;
};

class write_instruction : public instruction {
public:
  write_instruction(int line, std::unique_ptr<expression> value)
      : instruction(line), value(std::move(value)) {}
  ~write_instruction() noexcept override;
  void type_check();
  std::string get_code();
  void execute();

private:
  std::unique_ptr<expression> value;
};

class if_instruction : public instruction {
public:
  if_instruction(int line, std::unique_ptr<expression> condition,
                 std::vector<std::unique_ptr<instruction>> true_branch,
                 std::vector<std::unique_ptr<instruction>> false_branch)
      : instruction(line), condition(std::move(condition)),
        true_branch(std::move(true_branch)),
        false_branch(std::move(false_branch)) {}
  ~if_instruction() noexcept override;
  void type_check();
  std::string get_code();
  void execute();

private:
  std::unique_ptr<expression> condition;
  std::vector<std::unique_ptr<instruction>> true_branch;
  std::vector<std::unique_ptr<instruction>> false_branch;
};

class while_instruction : public instruction {
public:
  while_instruction(int line, std::unique_ptr<expression> condition,
                    std::vector<std::unique_ptr<instruction>> body)
      : instruction(line), condition(std::move(condition)),
        body(std::move(body)) {}
  ~while_instruction() noexcept override;
  void type_check();
  std::string get_code();
  void execute();

private:
  std::unique_ptr<expression> condition;
  std::vector<std::unique_ptr<instruction>> body;
};

class for_instruction : public instruction {
public:
  for_instruction(int line, std::string loopvar,
                  std::unique_ptr<expression> first,
                  std::unique_ptr<expression> last,
                  std::vector<std::unique_ptr<instruction>> body)
      : instruction(line), loopvar(std::move(loopvar)), first(std::move(first)),
        last(std::move(last)), body(std::move(body)) {}
  ~for_instruction() noexcept override;
  void type_check();
  std::string get_code();
  void execute();

private:
  std::string loopvar;
  std::unique_ptr<expression> first;
  std::unique_ptr<expression> last;
  std::vector<std::unique_ptr<instruction>> body;
};

void type_check_commands(
    const std::vector<std::unique_ptr<instruction>> &commands);

void generate_code_of_commands(
    std::ostream &out,
    const std::vector<std::unique_ptr<instruction>> &commands);

void execute_commands(
    const std::vector<std::unique_ptr<instruction>> &commands);

void delete_commands(const std::vector<std::unique_ptr<instruction>> &commands);

void generate_code(const std::vector<std::unique_ptr<instruction>> &commands);

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
