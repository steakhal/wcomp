#ifndef CFG_DUMPER_H
#define CFG_DUMPER_H

#include "cfg.h"
#include "expression_dumper.h"
#include "expressions.h"
#include "statements.h"

#include <iostream>

/// Dump the basicblocks in preorder.
class cfg_dumper : private expression_dumper {
  std::ostream &os;
  const unsigned indent;

public:
  explicit cfg_dumper(std::ostream &os, unsigned initial_indent = 0)
      : expression_dumper{os, initial_indent}, os{os}, indent{initial_indent} {}

  using expression_dumper::operator();

  std::ostream &operator()(const cfg &x) const noexcept;
  std::ostream &operator()(const basicblock &x) const noexcept;
  std::ostream &operator()(const ir_instruction &x) const noexcept;

  std::ostream &operator()(const assign_statement &x) const noexcept;
  std::ostream &operator()(const read_statement &x) const noexcept;
  std::ostream &operator()(const write_statement &x) const noexcept;
  std::ostream &operator()(const selector &x) const noexcept;
  std::ostream &operator()(const jump &x) const noexcept;
};

#endif // CFG_DUMPER_H
