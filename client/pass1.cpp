#include "client.h"

static section* s;

static ref<insn> _next_insn(lexer& lex)
{
    if (lex.touch() == '<') return pseudo_ascii(lex);
    token t1 = lex.get();
    token t2 = lex.get();
    if (t2 == ':' || t2 == '=')
    {
        lex.unget(t2);
        lex.unget(t1);
        return t2 == '=' ? assign_insn(lex) : label_insn(lex);
    }
    lex.unget(t2);
    if (t1 == L_sym)
    {
        if (lval<string>(t1) == string(".sect"))
        {
            s = &lookup(lex.getstring().c_str());
            return _next_insn(lex);
        }
        auto insn = opcode(lval<string>(t1).data(), lex);
        if (insn.defined()) return std::move(insn).extract();
    }
    lex.unget(t1);
    return opcode(".word", lex).extract();
}

static optional<ref<insn>> next_insn(lexer& lex)
{
    while (lex.tryget(';'));
    if (lex.tryget(L_eof)) return {};
    ref<insn> r = _next_insn(lex);
    r->line = state::line;
    r->filename = state::filename;
    return r;
}

void first_pass(lexer& lex)
{
    s = &program[0];
    for (;;)
    {
        try {
            auto r = next_insn(lex);
            if (!r) break;
            s->code.emplace_back(std::move(r).extract());
        } catch (Error) {
            char c;
            while ((c = lex.nextch(false)) != ';' && c != L_eof);
        }
    }
}
