#include "expr.h"

bool cexpr::compile(lexer& lex)
{
    bool flag = false;
    token t;
    while (L_iscexpr(t = lex.get()))
    {
        _impl.append(t);
        flag = true;
    }
    lex.unget(t);
    _impl.flush();
    return flag;
}

void cexpr::compile(std::initializer_list<token> list)
{
    for (token t : list)
    {
        _impl.append(t);
    }
    _impl.flush();
}
