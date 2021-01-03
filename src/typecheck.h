#ifndef TYPECHECK_h
#define TYPECHECK_h

#include "expressions.h"
#include "statements.h"

type infer_expression_type(const symbols &syms, const expression &x);

void type_check(const ast &ast);
void type_check(const symbols &syms, const statements &stmts);

#endif // TYPECHECK_h
