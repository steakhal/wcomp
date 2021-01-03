#include "cfg.h"

#include <algorithm>
#include <cassert>
#include <variant>

void basicblock::add_child(basicblock *child) { children.push_back(child); }

void basicblock::add_ir_instruction(ir_instruction inst) {
  if (!instructions.empty()) {
    const auto &last = instructions.back();
    const bool last_was_controlflow_instruction =
        std::holds_alternative<selector>(last) ||
        std::holds_alternative<jump>(last);
    assert(
        !last_was_controlflow_instruction &&
        "Can not add further instructions after a controlf-flow instruction.");
  }

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

void cfg::remove_bb(basicblock *bb) {
  assert(bb);
  assert(std::all_of(blocks.begin(), blocks.end(),
                     [bb](const auto &pair) {
                       const auto &range = pair.second.children;
                       return std::none_of(
                           range.begin(), range.end(),
                           [bb](const auto *x) { return x == bb; });
                     }) &&
         "No basic block should refer to a removed element.");
  const auto removed_count = blocks.erase(bb->id);
  assert(removed_count == 1);
}
