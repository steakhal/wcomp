#include "ast_to_cfg.h"
#include "expressions.h"
#include "statements.h"
#include "utility.h"

#include <variant>

namespace {
struct ast_to_cfg_visitor {
  cfg graph;
  basicblock *current_bb;

  ast_to_cfg_visitor() : current_bb{graph.entry} {}

  basicblock *operator()(statements &&xs);
  basicblock *operator()(invalid_statement &&x);
  basicblock *operator()(assign_statement &&x);
  basicblock *operator()(read_statement &&x);
  basicblock *operator()(write_statement &&x);
  basicblock *operator()(if_statement &&x);
  basicblock *operator()(while_statement &&x);
};
} // namespace

cfg ast_to_cfg(statements &&ast) {
  ast_to_cfg_visitor converter;
  converter.graph.exit = converter(std::move(ast));
  return std::move(converter.graph);
}

basicblock *ast_to_cfg_visitor::operator()(statements &&xs) {
  for (statement &x : xs)
    current_bb = std::visit(*this, std::move(x));
  return current_bb;
}

basicblock *ast_to_cfg_visitor::operator()(invalid_statement &&x) {
  unreachable();
}

basicblock *ast_to_cfg_visitor::operator()(assign_statement &&x) {
  current_bb->add_ir_instruction(std::move(x));
  return current_bb;
}

basicblock *ast_to_cfg_visitor::operator()(read_statement &&x) {
  current_bb->add_ir_instruction(std::move(x));
  return current_bb;
}

basicblock *ast_to_cfg_visitor::operator()(write_statement &&x) {
  current_bb->add_ir_instruction(std::move(x));
  return current_bb;
}

basicblock *ast_to_cfg_visitor::operator()(if_statement &&x) {
  basicblock *true_bb = graph.create_bb();
  basicblock *false_bb = graph.create_bb();
  basicblock *pseudo_exit_bb = graph.create_bb();

  current_bb->add_ir_instruction(
      selector{std::move(x.condition), *true_bb, *false_bb});

  current_bb = true_bb;
  basicblock *true_exit = operator()(std::move(x.true_branch));
  current_bb = false_bb;
  basicblock *false_exit = operator()(std::move(x.false_branch));

  // Add jumps targeting the pseudo node.
  true_exit->add_ir_instruction(jump{*pseudo_exit_bb});
  false_exit->add_ir_instruction(jump{*pseudo_exit_bb});

  return pseudo_exit_bb;
}

basicblock *ast_to_cfg_visitor::operator()(while_statement &&x) {
  basicblock *body_bb = graph.create_bb();
  basicblock *pseudo_exit_bb = graph.create_bb();
  auto cond_clone = std::make_unique<expression>(*x.condition);

  // Conditionally enter into the loop body or just jump to the exit.
  current_bb->add_ir_instruction(
      selector{std::move(x.condition), *body_bb, *pseudo_exit_bb});

  current_bb = body_bb;
  basicblock *body_exit = operator()(std::move(x.body));

  // Conditionally jump back to the body.
  body_exit->add_ir_instruction(
      selector{std::move(cond_clone), *body_bb, *pseudo_exit_bb});

  return pseudo_exit_bb;
}
