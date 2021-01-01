#include "cfg.h"

void basicblock::add_child(basicblock *child) { children.push_back(child); }

void basicblock::add_ir_instruction(ir_instruction inst) {
  instructions.emplace_back(std::move(inst));
}

bool basicblock::operator<(const basicblock &other) const noexcept {
  return id < other.id;
}

basicblock &cfg::create_bb() {
  const auto idx = next_bb_idx++;
  auto [it, _] = blocks.emplace(std::make_pair(idx, basicblock{idx}));
  return it->second;
}

void cfg::delete_bb(const basicblock &bb) { blocks.erase(bb.id); }
