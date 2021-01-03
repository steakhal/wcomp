#include "typecheck.h"

#include "expressions.h"
#include "statements.h"
#include "utility.h"

namespace {
type operand_type(std::string_view op) {
  if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%" ||
      op == "<" || op == ">" || op == "<=" || op == ">=") {
    return natural;
  }
  return boolean;
}

type return_type(std::string_view op) {
  if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
    return natural;
  }
  return boolean;
}

class expression_type_inferer {
  const symbols &syms;

public:
  explicit expression_type_inferer(const symbols &syms) : syms{syms} {}

  type operator()(const number_expression &x) const { return natural; }
  type operator()(const boolean_expression &x) const { return boolean; }
  type operator()(const id_expression &x) const {
    const auto it = syms.find(x.name);
    if (it == syms.end())
      error(x.line, std::string("Undefined variable: ") + x.name);
    return it->second.symbol_type;
  }
  type operator()(const binop_expression &x) const {
    const type left_ty = infer_expression_type(syms, *x.left);
    const type right_ty = infer_expression_type(syms, *x.right);

    if (x.op == "=") {
      if (left_ty != right_ty) {
        error(x.line, "Left and right operands of '=' have different types.");
      }
    } else {
      const type op_ty = operand_type(x.op);
      if (left_ty != op_ty) {
        error(x.line, std::string("Left operand of '") + x.op +
                          "' has unexpected type.");
      }
      if (right_ty != op_ty) {
        error(x.line, std::string("Right operand of '") + x.op +
                          "' has unexpected type.");
      }
    }
    return return_type(x.op);
  }
  type operator()(const not_expression &x) const {
    if (infer_expression_type(syms, *x.operand) != boolean)
      error(x.line, "Operand of 'not' is not boolean.");
    return boolean;
  }
};

class statement_type_checker {
  const symbols &syms;

public:
  explicit statement_type_checker(const symbols &syms) : syms{syms} {}

  void operator()(const invalid_statement &x) const { unreachable(); }
  void operator()(const assign_statement &x) const {
    const auto it = syms.find(x.left);
    if (it == syms.end())
      error(x.get_line(), std::string("Undefined variable: ") + x.left);

    if (it->second.symbol_type != infer_expression_type(syms, *x.right)) {
      error(x.get_line(),
            "Left and right hand sides of assignment are of different types.");
    }
  }
  void operator()(const read_statement &x) const {
    if (syms.count(x.id) == 0)
      error(x.get_line(), std::string("Undefined variable: ") + x.id);
  }
  void operator()(const write_statement &x) const {}
  void operator()(const if_statement &x) const {
    if (infer_expression_type(syms, *x.condition) != boolean)
      error(x.get_line(), "Condition of 'if' instruction is not boolean.");

    type_check(syms, x.true_branch);
    type_check(syms, x.false_branch);
  }
  void operator()(const while_statement &x) const {
    if (infer_expression_type(syms, *x.condition) != boolean)
      error(x.get_line(), "Condition of 'while' instruction is not boolean.");
    type_check(syms, x.body);
  }
};
} // namespace

type infer_expression_type(const symbols &syms, const expression &x) {
  return std::visit(expression_type_inferer{syms}, x);
}

void type_check(const ast &ast) { type_check(ast.syms, ast.stmts); }
void type_check(const symbols &syms, const statements &stmts) {
  statement_type_checker checker{syms};

  for (const statement &stmt : stmts)
    std::visit(checker, stmt);
}
