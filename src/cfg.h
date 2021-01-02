#ifndef CFG_H
#define CFG_H

#include "expressions.h"
#include "statements.h"

#include <memory>
#include <set>
#include <variant>
#include <vector>

// any expression; any statement except while_statement
using ir_instruction =
    std::variant<number_expression, boolean_expression, id_expression,
                 binop_expression, not_expression, assign_statement,
                 read_statement, write_statement, class selector, class jump>;

class basicblock;

class selector {
public:
  std::unique_ptr<expression> condition;
  basicblock &true_branch;
  basicblock &false_branch;
};

class jump {
public:
  basicblock &target;
};

using bb_idx = std::size_t;

class basicblock {
  void add_child(basicblock *child);

public:
  const bb_idx id;
  std::vector<ir_instruction> instructions;
  std::vector<basicblock *> children;

  explicit basicblock(bb_idx id) : id{id} {}

  void add_ir_instruction(ir_instruction inst);

  bool operator<(const basicblock &other) const noexcept;
};

class cfg {
public:
  std::map<bb_idx, basicblock> blocks;
  bb_idx next_bb_idx = 0;
  basicblock *entry = create_bb();
  basicblock *exit = entry;

  basicblock *create_bb();
};

#endif // CFG_H
