#include "cfg.h"
#include "expressions.h"
#include "statements.h"
#include "typecheck.h"
#include "utility.h"

#include <cassert>
#include <iostream>
#include <set>
#include <sstream>
#include <string_view>

namespace {
class symbols_to_asm {
  std::ostream &ss;

public:
  explicit symbols_to_asm(std::ostream &ss) : ss{ss} {}

  void operator()(const symbols &syms) const {
    for (const auto &pair : syms) {
      const auto &name = pair.first;
      const symbol &sym = pair.second;
      const int width = sym.symbol_type == boolean ? 1 : 4;
      ss << "var_" << sym.name << ": resb " << width << '\n';
    }
  }
};

std::string_view get_register(type ty) { return ty == boolean ? "al" : "eax"; }

void emit_eq_code(std::ostream &ss, type ty) {
  ss << (ty == natural ? "cmp eax,ecx\n" : "cmp al,cl\n");
  ss << "mov al,0\n";
  ss << "mov cx,1\n";
  ss << "cmove ax,cx\n";
}

void emit_operator_code(std::ostream &ss, std::string_view op) {
  if (op == "+") {
    ss << "add eax,ecx\n";
  } else if (op == "-") {
    ss << "sub eax,ecx\n";
  } else if (op == "*") {
    ss << "xor edx,edx\n";
    ss << "mul ecx\n";
  } else if (op == "/") {
    ss << "xor edx,edx\n";
    ss << "div ecx\n";
  } else if (op == "%") {
    ss << "xor edx,edx\n";
    ss << "div ecx\n";
    ss << "mov eax,edx\n";
  } else if (op == "<") {
    ss << "cmp eax,ecx\n";
    ss << "mov al,0\n";
    ss << "mov cx,1\n";
    ss << "cmovb ax,cx\n";
  } else if (op == "<=") {
    ss << "cmp eax,ecx\n";
    ss << "mov al,0\n";
    ss << "mov cx,1\n";
    ss << "cmovbe ax,cx\n";
  } else if (op == ">") {
    ss << "cmp eax,ecx\n";
    ss << "mov al,0\n";
    ss << "mov cx,1\n";
    ss << "cmova ax,cx\n";
  } else if (op == ">=") {
    ss << "cmp eax,ecx\n";
    ss << "mov al,0\n";
    ss << "mov cx,1\n";
    ss << "cmovae ax,cx\n";
  } else if (op == "and") {
    ss << "cmp al,1\n";
    ss << "cmove ax,cx\n";
  } else if (op == "or") {
    ss << "cmp al,0\n";
    ss << "cmove ax,cx\n";
  } else {
    error(-1,
          std::string("Bug: Unsupported binary operator: ") + std::string(op));
  }
}

std::string_view get_type_name(type ty) {
  return ty == boolean ? "boolean" : "natural";
}

class ir_to_asm {
  const symbols &syms;
  std::ostream &ss;

public:
  ir_to_asm(const symbols &syms, std::ostream &ss) : syms{syms}, ss{ss} {}

  // Expressions:
  void operator()(const number_expression &x) const {
    ss << "mov eax," << x.value << '\n';
  }
  void operator()(const boolean_expression &x) const {
    ss << "mov al," << (x.value ? 1 : 0) << '\n';
  }
  void operator()(const id_expression &x) const {
    const auto it = syms.find(x.name);
    assert(it != syms.end());
    ss << "mov eax,[var_" << it->second.name + "]\n";
  }
  void operator()(const binop_expression &x) const {
    std::visit(*this, *x.left);
    ss << "push eax\n";
    std::visit(*this, *x.right);
    ss << "mov ecx,eax\n";
    ss << "pop eax\n";
    if (x.op == "=")
      emit_eq_code(ss, infer_expression_type(syms, *x.left));
    else
      emit_operator_code(ss, x.op);
  }
  void operator()(const not_expression &x) const {
    std::visit(*this, *x.operand);
    ss << "xor al,1\n";
  }

  // Statements:
  void operator()(const assign_statement &x) const {
    std::visit(*this, *x.right);
    const auto it = syms.find(x.left);
    assert(it != syms.end());
    ss << "mov [var_" + it->second.name + "],"
       << get_register(it->second.symbol_type) << '\n';
  }
  void operator()(const read_statement &x) const {
    const auto it = syms.find(x.id);
    assert(it != syms.end());
    const type ty = it->second.symbol_type;
    ss << "call read_" << get_type_name(ty) << '\n';
    ss << "mov [var_" << it->second.name << "]," << get_register(ty) << '\n';
  }
  void operator()(const write_statement &x) const {
    const type ty = infer_expression_type(syms, *x.value);
    std::visit(*this, *x.value);
    if (ty == boolean) {
      ss << "and eax,1\n";
    }
    ss << "push eax\n";
    ss << "call write_" << get_type_name(ty) << '\n';
    ss << "add esp,4\n";
  }

  void operator()(const cassign &x) const {
    std::visit(*this, *x.condition);
    ss << "cmp al,1\n";
    ss << "mov eax," << x.false_value << '\n';
    ss << "mov ebx, " << x.true_value << '\n';
    ss << "cmove eax, ebx\n";
    ss << "mov [var_" << x.var.name << "], eax\n";
  }

  // Control-flow:
  void operator()(const selector &x) const {
    std::visit(*this, *x.condition);
    ss << "cmp al,1\n";
    ss << "je bb_" << x.true_branch.id << '\n';
    ss << "jmp bb_" << x.false_branch.id << '\n';
  }
  void operator()(const jump &x) const {
    ss << "jmp bb_" << x.target.id << '\n';
  }
  void operator()(const switcher &x) const {
    operator()(x.var); // Load var to eax.

    assert(!x.branches.empty());
    for (const auto &pair : x.branches) {
      ss << "mov ecx, " << pair.first << '\n';
      ss << "cmp eax,ecx\n";
      ss << "je bb_" << pair.second->id << '\n';
    }
  }
};

void emit_basicblock(std::ostream &ss, const cfg &cfg, const symbols &syms,
                     const basicblock &bb) {
  ir_to_asm emitter{syms, ss};

  if (&bb == cfg.entry)
    ss << "; entry\n";
  else if (&bb == cfg.exit)
    ss << "; exit\n";

  ss << "bb_" << bb.id << ":\n";

  for (const ir_instruction &inst : bb.instructions)
    std::visit(emitter, inst);

  if (&bb == cfg.exit) {
    ss << "xor eax,eax\n";
    ss << "ret\n";
  }
}

void recursively_emit_basicblock(std::ostream &ss, const cfg &cfg,
                                 const symbols &syms, const basicblock &bb,
                                 std::set<bb_idx> &processed) {
  auto [_, succeeded] = processed.insert(bb.id);
  if (!succeeded)
    return;

  emit_basicblock(ss, cfg, syms, bb);

  if (bb.instructions.empty())
    return;

  std::visit(overloaded{[&](const auto &x) {},
                        [&](const selector &x) {
                          recursively_emit_basicblock(ss, cfg, syms,
                                                      x.true_branch, processed);
                          recursively_emit_basicblock(
                              ss, cfg, syms, x.false_branch, processed);
                        },
                        [&](const jump &x) {
                          recursively_emit_basicblock(ss, cfg, syms, x.target,
                                                      processed);
                        },
                        [&](const switcher &x) {
                          for (const auto [_, child] : x.branches)
                            recursively_emit_basicblock(ss, cfg, syms, *child,
                                                        processed);
                        }},
             bb.instructions.back());
}

} // namespace

std::string codegen(const cfg &cfg, const symbols &syms) {
  std::stringstream ss;
  ss << "global main\n"
        "extern write_natural\n"
        "extern read_natural\n"
        "extern write_boolean\n"
        "extern read_boolean\n\n"
        "section .bss\n";
  symbols_to_asm{ss}(syms);

  ss << "\nsection .text\nmain:\n";
  std::set<bb_idx> processed;
  recursively_emit_basicblock(ss, cfg, syms, *cfg.entry, processed);
  return ss.str();
}
