#include <functional>
#include "expr.h"

void symbol::make_mutable()
{
    error(_mutable == 0 && _expr != nullptr, "attempted to make symbol <{}> mutable after assignment", name.data());
    _mutable = 1;
}

void symbol::assign(expr& expr)
{
    error(_expr && _expr != &expr && _mutable == 0, "attempted to redefine immutable symbol <{}>", name.data());
    error(_used && expr.type().type != A_m0, "not m0-symbol <{}> references before assignment", name.data());
    _expr = &expr;
    _used = 0;
}

void symbol::update()
{
    error(_expr == nullptr, "symbol <{}> used, but never defined", name.data());
    value = _expr->eval();
    type = _expr->type();
}
