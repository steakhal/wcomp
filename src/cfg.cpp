#include "cfg.h"

void basicblock::add_child(basicblock *child) { children.push_back(child); }

void basicblock::add_ir_instruction(ir_instruction inst) {
  // Register jump targets.
  std::visit(overloaded{
                 [&](auto &x) {},
                 [&](selector &x) {
                   add_child(&x.true_branch);
                   add_child(&x.false_branch);
                 },
                 [&](jump &x) { add_child(&x.target); },
             },
             inst);

  // Store the instruction.
  instructions.emplace_back(std::move(inst));
}

bool basicblock::operator<(const basicblock &other) const noexcept {
  return id < other.id;
}

basicblock *cfg::create_bb() {
  const auto idx = next_bb_idx++;
  auto [it, _] = blocks.emplace(std::make_pair(idx, basicblock{idx}));
  return &it->second;
}
