#include "cfg.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <variant>

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

  // Store the instruction.
  instructions.emplace_back(std::move(inst));
}

ir_instruction basicblock::pop_last_ir_instruction() {
  assert(!instructions.empty());
  ir_instruction last = std::move(instructions.back());
  instructions.pop_back();
  return last;
}

bool basicblock::operator<(const basicblock &other) const noexcept {
  return id < other.id;
}

basicblock *cfg::create_bb() {
  const auto idx = next_bb_idx++;
  assert(std::find_if(blocks.begin(), blocks.end(), [idx](const auto &x) {
           return x->id == idx;
         }) == blocks.end());
  blocks.push_back(std::make_unique<basicblock>(idx));
  return blocks.back().get();
}
