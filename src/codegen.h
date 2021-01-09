#ifndef CODEGEN_H
#define CODEGEN_H

#include "cfg.h"
#include "expressions.h"
#include "statements.h"

#include <optional>
#include <string>

std::string codegen(const cfg &cfg, const symbols &syms,
                    std::optional<std::size_t> serialization_seed,
                    bool encode_constants);

#endif // CODEGEN_H
