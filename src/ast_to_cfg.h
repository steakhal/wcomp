#ifndef AST_TO_CFG_H
#define AST_TO_CFG_H

#include "cfg.h"
#include "statements.h"

cfg ast_to_cfg(statements &&xs);

#endif // AST_TO_CFG_H
