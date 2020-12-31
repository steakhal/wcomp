#ifndef STATEMENTS_H
#define STATEMENTS_H

#include "expressions.h"
#include "utility.h"

#include <variant>
#include <vector>

using statement = std::variant<class invalid_statement, class assign_statement,
                               class read_statement, class write_statement,
                               class if_statement, class while_statement>;
using statements = std::vector<statement>;

void type_check(const statement &stmt);
std::string get_code(const statement &stmt);
void execute(const statement &stmt);

void type_check(const statements &stmts);
std::string get_code(const statements &stmts);
void execute(const statements &stmts);

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

  std::string id;
};

class write_statement : public statement_base {
public:
  write_statement(int line, std::unique_ptr<expression> value)
      : statement_base(line), value(std::move(value)) {}
  void type_check() const;
  std::string get_code() const;
  void execute() const;

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

  std::unique_ptr<expression> condition;
  statements body;
};

#endif // STATEMENTS_H
