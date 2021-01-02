#include "cfg_dumper.h"
#include "utility.h"

#include <cassert>
#include <iostream>
#include <variant>

std::ostream &text_cfg_dumper::operator()(const cfg &x) noexcept {
  os << "Control flow graph:\n";
  os << "Entry: " << x.entry->id << '\n';
  os << "Exit:  " << x.exit->id << '\n';

  // Start dumping from  the entry block.
  (*this)(*x.entry);
  return os;
}

std::ostream &text_cfg_dumper::operator()(const basicblock &x) noexcept {
  auto [_, succeeded] = processed.insert(x.id);
  if (!succeeded)
    return os;

  os << "Basic block: " << x.id << '\n';
  text_cfg_dumper sub_dumper{os, indent + 2};
  for (const auto &inst : x.instructions)
    sub_dumper(inst);
  for (const basicblock *child : x.children)
    (*this)(*child);
  return os;
}

std::ostream &
text_cfg_dumper::operator()(const ir_instruction &x) const noexcept {
  std::visit(*this, x);
  return os;
}

std::ostream &
text_cfg_dumper::operator()(const assign_statement &x) const noexcept {
  repeat(os, ' ', indent) << x.left << " := ";
  std::visit(*this, *x.right);
  return os << '\n';
}

std::ostream &
text_cfg_dumper::operator()(const read_statement &x) const noexcept {
  return repeat(os, ' ', indent) << "read(" << x.id << ")\n";
}

std::ostream &
text_cfg_dumper::operator()(const write_statement &x) const noexcept {
  repeat(os, ' ', indent) << "write(";
  std::visit(*this, *x.value);
  return os << ")\n";
}

std::ostream &text_cfg_dumper::operator()(const selector &x) const noexcept {
  repeat(os, ' ', indent) << "select basicblock " << x.true_branch.id << " or "
                          << x.false_branch.id << " depending on ";
  std::visit(*this, *x.condition);
  return os << '\n';
}

std::ostream &text_cfg_dumper::operator()(const jump &x) const noexcept {
  return repeat(os, ' ', indent)
         << "jump to basicblock " << x.target.id << '\n';
}

// Dot

std::ostream &dot_cfg_dumper::operator()(const cfg &x) noexcept {
  os << "digraph CFG {\n";
  os << "  node [shape=\"box\",style=filled];\n";

  // Start dumping from  the entry block.
  (*this)(*x.entry);
  os << "}\n";
  return os;
}

std::ostream &dot_cfg_dumper::operator()(const basicblock &x) noexcept {
  auto [_, succeeded] = processed.insert(x.id);
  if (!succeeded)
    return os;

  os << "  bb_" << x.id << " [label=\"";
  os << "---  bb_" << x.id << "  ---\\n\\n";
  dot_cfg_dumper sub_dumper{os};
  for (const auto &inst : x.instructions)
    sub_dumper(inst);
  os << "\"]\n";

  assert(x.children.size() <= 2);
  if (x.children.size() == 2) {
    os << "  bb_" << x.id << " -> "
       << "bb_" << x.children[0]->id << "[label=\"true\",color=darkgreen]\n";
    (*this)(*x.children[0]);
    os << "  bb_" << x.id << " -> "
       << "bb_" << x.children[1]->id << "[label=\"false\",color=red]\n";
    (*this)(*x.children[1]);
  } else if (x.children.size() == 1) {
    os << "  bb_" << x.id << " -> "
       << "bb_" << x.children[0]->id << "\n";
    (*this)(*x.children[0]);
  }
  return os;
}

std::ostream &
dot_cfg_dumper::operator()(const ir_instruction &x) const noexcept {
  std::visit(*this, x);
  return os;
}

std::ostream &
dot_cfg_dumper::operator()(const assign_statement &x) const noexcept {
  os << x.left << " := ";
  std::visit(*this, *x.right);
  return os << "\\l";
}

std::ostream &
dot_cfg_dumper::operator()(const read_statement &x) const noexcept {
  return os << "read(" << x.id << ")\\l";
}

std::ostream &
dot_cfg_dumper::operator()(const write_statement &x) const noexcept {
  os << "write(";
  std::visit(*this, *x.value);
  return os << ")\\l";
}

std::ostream &dot_cfg_dumper::operator()(const selector &x) const noexcept {
  os << "select bb_" << x.true_branch.id << " if ";
  std::visit(*this, *x.condition);
  os << " bb_" << x.false_branch.id << " otherwise\\l";
  return os;
}

std::ostream &dot_cfg_dumper::operator()(const jump &x) const noexcept {
  return os << "jump to bb_" << x.target.id << "\\l";
}
