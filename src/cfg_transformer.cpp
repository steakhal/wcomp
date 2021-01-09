#include "cfg_transformer.h"
#include "utility.h"

#include <cassert>
#include <set>
#include <string_view>
#include <variant>

namespace {

std::string generate_unique_identifier(const symbols &syms,
                                       std::string_view prefix) {
  std::string unique_name{prefix};
  while (syms.count(unique_name) != 0)
    unique_name.append("x");
  return unique_name;
}

template <typename T, typename... Ts>
std::unique_ptr<expression> create_expr(Ts &&... args) {
  return std::make_unique<expression>(std::in_place_type<T>,
                                      std::forward<Ts>(args)...);
}

} // namespace

void flatten(symbols &syms, cfg &graph) {
  std::vector targets = [&graph] {
    std::vector<std::pair<bb_idx, basicblock *>> res;
    res.reserve(graph.blocks.size());
    for (auto &pair : graph.blocks)
      res.push_back(std::make_pair(pair.first, &pair.second));
    return res;
  }();

  constexpr int invalid_lineno = -1;

  basicblock *new_entry = graph.create_bb();
  basicblock *switch_dispatcher = graph.create_bb();
  const bb_idx original_entry = graph.entry->id;
  const bb_idx original_exit = graph.exit->id;

  // Create a unique identifier and declare it.
  symbol bb_selector{
      /*line=*/-1, /*name=*/generate_unique_identifier(syms, "__bb_selector_"),
      /*symbol_type=*/natural};
  syms.insert(std::make_pair(bb_selector.name, bb_selector));
  id_expression selector_var{invalid_lineno, /*name=*/bb_selector.name};

  // TODO: Remove debug dumps.
  std::cerr << "bb_selector: " << bb_selector.name << '\n';
  std::cerr << "targets: ";
  for (const auto [id, _] : targets)
    std::cerr << id << ' ';
  std::cerr << '\n';

  // NewEntry:
  // selector := original_entry
  // dispatch!
  new_entry->add_ir_instruction(assign_statement{
      invalid_lineno,
      /*left=*/bb_selector.name,
      /*right=*/create_expr<number_expression>(original_entry)});
  new_entry->add_ir_instruction(jump{*switch_dispatcher});

  // The CFG should start from the new entry.
  graph.entry = new_entry;

  // Replace the (last) cfg-instruction in each basic block.
  for (auto &pair : targets) {
    basicblock *target = pair.second;
    if (target->instructions.empty())
      continue;

    ir_instruction last = target->pop_last_ir_instruction();

    if (auto *x = std::get_if<selector>(&last)) {
      // selector := cond ? true_branch.id : false_branch.id
      // dispatch!
      target->add_ir_instruction(cassign{/*var=*/selector_var,
                                         /*condition=*/std::move(x->condition),
                                         /*true_value=*/x->true_branch.id,
                                         /*false_value=*/x->false_branch.id});
      target->add_ir_instruction(jump{*switch_dispatcher});
    } else if (auto *x = std::get_if<jump>(&last)) {
      // selector := target.id
      // dispatch!
      target->add_ir_instruction(assign_statement{
          invalid_lineno,
          /*left=*/selector_var.name,
          /*right=*/
          create_expr<number_expression>(/*value=*/x->target.id)});
      target->add_ir_instruction(jump{*switch_dispatcher});
    } else {
      target->add_ir_instruction(std::move(last));
    }
  }

  // Add the IR instruction for the switch to the dispatcher basic block.
  switch_dispatcher->add_ir_instruction(
      switcher{selector_var, std::move(targets)});
}
