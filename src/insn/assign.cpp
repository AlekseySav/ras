#include "insn.h"

struct assign : insn
{
    assign(symbol& s, expr&& e) : s(s), e(std::move(e))
    {}

    bool update() override
    {
        word v = s.value;
        typeinfo ti = s.type;
        s.assign(e);
        s.update();
        return (ti != s.type || v != s.value) && !s.is_mutable();
    }

    symbol& s;
    expr e;
};

ref<insn> assign_insn(expr_iterator it, byte n, byte d, bool f)
{
    lexer& lex = it.lex;
    token t = lex.get();
    error(t != L_sym, "bad assignment left operand");
    panic(lex.get() != '=', "assignment expr");
    expr e = *it;
    error(!lex.tryget(';') && !lex.tryget(','), "bad assignment separator");
    return make_insn<assign>(symbol::lookup(lval<string>(t), true), std::move(e));
}

ref<insn> label_insn(expr_iterator it, byte n, byte d, bool f)
{
    lexer& lex = it.lex;
    token t = lex.get();
    error(t != L_sym && t != L_num, "bad label");
    panic(lex.get() != ':', "label expr");
    cexpr c;
    c.compile({token(L_sym, string("."))});
    symbol& s = t == L_sym ? symbol::lookup(lval<string>(t), true) : symbol::lookup(lval<word>(t));
    return make_insn<assign>(s, expr(std::move(c)));
}
