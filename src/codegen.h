#ifndef CODEGEN_H
#define CODEGEN_H

#include "cfg.h"
#include "expressions.h"
#include "statements.h"

#include <string>

std::string codegen(const cfg &cfg, const symbols &syms);

#endif // CODEGEN_H
