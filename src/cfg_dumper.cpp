#include "cfg_dumper.h"
#include "utility.h"

#include <iostream>
#include <variant>

std::ostream &cfg_dumper::operator()(const cfg &x) const noexcept {
  os << "Control flow graph:\n";
  os << "Entry: " << x.entry.id << '\n';
  os << "Exit:  " << x.exit.id << '\n';

  // Start dumping from  the entry block.
  (*this)(x.entry);
  return os;
}

std::ostream &cfg_dumper::operator()(const basicblock &x) const noexcept {
  os << "Basic block: " << x.id << '\n';
  cfg_dumper sub_dumper{os, indent + 2};
  for (const auto &inst : x.instructions)
    sub_dumper(inst);
  for (const basicblock *child : x.children)
    (*this)(*child);
  return os;
}

std::ostream &cfg_dumper::operator()(const ir_instruction &x) const noexcept {
  std::visit(*this, x);
  return os;
}

std::ostream &cfg_dumper::operator()(const assign_statement &x) const noexcept {
  repeat(os, ' ', indent) << x.left << " := ";
  std::visit(*this, *x.right);
  return os << '\n';
}

std::ostream &cfg_dumper::operator()(const read_statement &x) const noexcept {
  return repeat(os, ' ', indent) << "read(" << x.id << ")\n";
}

std::ostream &cfg_dumper::operator()(const write_statement &x) const noexcept {
  repeat(os, ' ', indent) << "write(";
  std::visit(*this, *x.value);
  return os << ")\n";
}

std::ostream &cfg_dumper::operator()(const selector &x) const noexcept {
  repeat(os, ' ', indent) << "select basicblock " << x.true_branch.id << " or "
                          << x.false_branch.id << " depending on ";
  std::visit(*this, *x.condition);
  return os << '\n';
}

std::ostream &cfg_dumper::operator()(const jump &x) const noexcept {
  return repeat(os, ' ', indent)
         << "jump to basicblock " << x.target.id << '\n';
}
