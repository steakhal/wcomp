#ifndef CFG_TRANSFORMER_H
#define CFG_TRANSFORMER_H

#include "cfg.h"

void flatten(symbols &syms, cfg &graph);

template <typename Generator> void remap_block_ids(cfg &graph, Generator &gen) {
  constexpr auto largest_random = 1 << 30;
  std::uniform_int_distribution<bb_idx> distr(0, largest_random);

  // Create the mapping.
  std::map<bb_idx, bb_idx> remap;
  std::set<bb_idx> used;

  for (auto &block : graph.blocks) {
    bb_idx newid;
    bool success;
    do {
      newid = distr(gen);
      std::tie(std::ignore, success) = used.emplace(newid);
    } while (!success);
    remap.emplace(std::make_pair(block->id, newid));
  }

  // Apply the mapping.
  for (auto &block : graph.blocks) {
    const auto old_id = block->id;
    const auto new_id = remap.at(old_id);
    block->id = new_id;
  }
  graph.next_bb_idx = largest_random + 1;
}

#endif // CFG_TRANSFORMER_H
