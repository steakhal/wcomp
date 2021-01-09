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
  operator()(*x.entry);
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

  if (x.instructions.empty())
    return os;

  std::visit(overloaded{[&](const auto &) {},
                        [&](const selector &x) {
                          operator()(x.true_branch);
                          operator()(x.false_branch);
                        },
                        [&](const jump &x) { operator()(x.target); },
                        [&](const switcher &x) {
                          for (const basicblock *target : x.branches)
                            operator()(*target);
                        }},
             x.instructions.back());
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

std::ostream &text_cfg_dumper::operator()(const switcher &x) const noexcept {
  repeat(os, ' ', indent) << "switch on variable " << x.var.name << " {\n";
  for (const basicblock *target : x.branches) {
    repeat(os, ' ', indent + 2)
        << target->id << " -> bb_" << target->id << '\n';
  }
  repeat(os, ' ', indent) << "}\n";
  return os;
}

std::ostream &text_cfg_dumper::operator()(const cassign &x) const noexcept {
  repeat(os, ' ', indent) << "assign " << x.true_value << " to " << x.var.name
                          << " if ";
  std::visit(*this, *x.condition);
  os << " else " << x.false_value << '\n';
  return os;
}

// Dot

std::ostream &dot_cfg_dumper::operator()(const cfg &x) noexcept {
  os << "digraph CFG {\n";
  os << "  node [shape=\"box\",style=filled];\n";

  // Start dumping from  the entry block.
  operator()(*x.entry);
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

  if (x.instructions.empty())
    return os;

  std::visit(overloaded{[&](const auto &) {},
                        [&](const selector &y) {
                          os << "  bb_" << x.id << " -> "
                             << "bb_" << y.true_branch.id
                             << "[label=\"true\",color=darkgreen]\n";
                          operator()(y.true_branch);
                          os << "  bb_" << x.id << " -> "
                             << "bb_" << y.false_branch.id
                             << "[label=\"false\",color=red]\n";
                          operator()(y.false_branch);
                        },
                        [&](const jump &y) {
                          os << "  bb_" << x.id << " -> "
                             << "bb_" << y.target.id << "\n";
                          operator()(y.target);
                        },
                        [&](const switcher &y) {
                          for (const basicblock *target : y.branches)
                            os << "  bb_" << x.id << " -> "
                               << "bb_" << target->id << "\n";
                          for (const basicblock *target : y.branches)
                            operator()(*target);
                        }},
             x.instructions.back());
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

std::ostream &dot_cfg_dumper::operator()(const switcher &x) const noexcept {
  os << "switch on variable " << x.var.name << "{\\l";
  for (const basicblock *target : x.branches) {
    os << "  " << target->id << " -> bb_" << target->id << "\\l";
  }
  os << "}\\l";
  return os;
}

std::ostream &dot_cfg_dumper::operator()(const cassign &x) const noexcept {
  os << "assign " << x.true_value << " to " << x.var.name << " if ";
  std::visit(*this, *x.condition);
  os << ' ' << " else " << x.false_value << "\\l";
  return os;
}
