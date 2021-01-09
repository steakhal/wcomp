#ifndef CFG_H
#define CFG_H

#include "expressions.h"
#include "statements.h"

#include <memory>
#include <set>
#include <variant>
#include <vector>

using bb_idx = std::size_t;

// any expression; any statement except while_statement
using ir_instruction =
    std::variant<number_expression, boolean_expression, id_expression,
                 binop_expression, not_expression, assign_statement,
                 read_statement, write_statement, class selector, class jump,
                 class switcher, class cassign>;

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

class switcher {
public:
  id_expression var;
  std::vector<std::pair<bb_idx, basicblock *>> branches;
};

class cassign {
public:
  id_expression var;
  std::unique_ptr<expression> condition;
  bb_idx true_value;
  bb_idx false_value;
};

class basicblock {
public:
  const bb_idx id;
  std::vector<ir_instruction> instructions;

  explicit basicblock(bb_idx id) : id{id} {}

  void add_ir_instruction(ir_instruction inst);
  ir_instruction pop_last_ir_instruction();

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
