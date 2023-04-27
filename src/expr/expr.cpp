#include "expr.h"
#include "../as.h"

expr::expr(typeinfo t) : _type(t), _const{true}
{}

expr::expr(cexpr&& c) : _type(A_m0), _expr(std::move(c)), _const{false}
{}

expr::expr(lexer& lex) : _type(A_m0), _const{false}
{
    token t = lex.get();
    if (t == L_sym)
    {
        typeinfo ti = symbol::lookup(lval<string>(t)).type;
        if (ti.type == A_rb || ti.type == A_rw || ti.type == A_sr)
        {
            _type = ti;
            _const = true;
            return;
        }
    }
    if (t == '$')
    {
        _type = A_im;
    }
    else
    {
        lex.unget(t);
    }

    bool disp = _expr.compile(lex);
    if (lex.tryget('('))
    {
        error(_type.type == A_im, "bad expr syntax: <Scexpr(mr)>");
        _type.type = A_mm;
        t = lex.get();
        error(t != L_sym, "bad mod r/m byte");
        _type.n = as::make_rm(symbol::lookup(lval<string>(t)).type);
        error(!lex.tryget(')'), "bad expr syntax: missed <)>");
    }

    error(_type.type != A_mm && disp == false, "bad expr: <$>");
}

typeinfo expr::type() const
{
    if (_const) return _type;
    typeinfo t = _expr.type();
    error(t.type != A_m0 && _type.type != A_m0, "bad expr");
    return t.type == A_m0 ? _type : t;
}

word expr::eval() const
{
    if (state::assert_defined)
    {
        _expr.assert_defined();
    }
    return _expr.eval();
}
