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
        if (ti.type & (A_rb | A_rw | A_sr | A_cr | A_dr | A_tr))
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
        _type = as::make_rm(lex);
        error(!lex.tryget(')'), "bad expr syntax: missed <)>");
    }

    error(!(_type.type & A_mm) && disp == false, "bad expr: <$>");
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
