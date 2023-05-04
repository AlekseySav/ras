#include "insn.h"
#include <vector>

ref<insn> pseudo_error(expr_iterator it, byte n, byte d, bool f)
{
    struct insn_error : insn
    {
        insn_error(std::string&& s) : s(std::move(s))
        {}

        bool update() override
        {
            error("{}", s);
            return false;
        }

        std::string s;
    };

    return make_insn<insn_error>(it.lex.getstring());
}

ref<insn> pseudo_mut(expr_iterator it, byte n, byte d, bool f)
{
    struct insn_if : insn
    {
        insn_if(lexer& lex)
        {
            token t;
            for (;;)
            {
                error((t = lex.get()) != L_sym, "bad .mut syntax");
                syms.push_back(&symbol::lookup(lval<string>(t)));
                if (lex.tryget(';'))
                {
                    break;
                }
                error(lex.get() != ',', "bad .mut syntax");
            }
        }

        bool update() override
        {
            for (symbol* s : syms)
            {
                s->make_mutable();
            }
            return false;
        }

        std::vector<symbol*> syms;
    };

    return make_insn<insn_if>(it.lex);
}

ref<insn> pseudo_if(expr_iterator it, byte n, byte d, bool f)
{
    struct insn_if : insn
    {
        insn_if(expr&& e) : b(false), e(std::move(e)), insn(true)
        {}

        bool update() override
        {
            error(e.type() != A_m0, "bad .if syntax");
            bool old = b;
            b = e.eval() && state::if_stack.top();
            state::if_stack.push(b);
            return old != b;
        }

        bool b;
        expr e;
    };

    return make_insn<insn_if>(it.exactly_one());
}

ref<insn> pseudo_else(expr_iterator it, byte n, byte d, bool f)
{
    struct insn_else : insn
    {
        insn_else() : b(false), insn(true) {}

        bool update() override
        {
            bool old = b;
            error(state::if_stack.size() == 1, ".else without corresponding .if");
            b = state::if_stack.top() = !state::if_stack.top();
            return old != b;
        }

        bool b;
    };

    return make_insn<insn_else>();
}

ref<insn> pseudo_endif(expr_iterator it, byte n, byte d, bool f)
{
    struct insn_endif : insn
    {
        insn_endif() : insn(true) {}

        bool update() override
        {
            error(state::if_stack.size() == 1, ".endif without corresponding .if");
            state::if_stack.pop();
            return false;
        }
    };

    return make_insn<insn_endif>();
}

template<size_t S>
struct insn_store : insn
{
    insn_store(expr_iterator it)
    {
        for (expr e : it)
        {
            data.emplace_back(std::move(e));
        }
        size = data.size() * S;
    }

    void flush(output& out) override
    {
        for (expr& e : data)
        {
            error(e.type().type != A_m0, "bad .byte/.word syntax");
            if constexpr (S == 1)
            {
                out.put_byte(e.eval());
                continue;
            }
            if constexpr (S == 2)
            {
                out.put_word(e.eval());
                continue;
            }
            out.put_long(e.eval());
        }
    }

    std::vector<expr> data;
};

ref<insn> pseudo_byte(expr_iterator it, byte n, byte d, bool f)
{
    return make_insn<insn_store<1>>(it);
}

ref<insn> pseudo_word(expr_iterator it, byte n, byte d, bool f)
{
    return make_insn<insn_store<2>>(it);
}

ref<insn> pseudo_long(expr_iterator it, byte n, byte d, bool f)
{
    return make_insn<insn_store<4>>(it);
}

ref<insn> pseudo_ascii(expr_iterator it, byte n, byte d, bool f)
{
    struct insn_ascii : insn
    {
        insn_ascii(std::string&& s) : s(std::move(s))
        {
            size = this->s.size();
        }

        void flush(output& out) override
        {
            for (char c : s)
            {
                out.put_byte(c);
            }
        }

        std::string s;
    };

    return make_insn<insn_ascii>(it.lex.getstring());
}

ref<insn> pseudo_fill(expr_iterator it, byte n, byte d, bool f)
{
    struct insn_fill : insn
    {
        insn_fill(expr_iterator it)
        {
            error(it == it.end(), "bad .fill syntax");
            n = *it;
            ++it;
            if (it != it.end())
            {
                v = *it;
                ++it;
            }
            else
            {
                cexpr c;
                c.compile({token{L_num, 0}});
                v = expr(std::move(c));
            }
            error(it != it.end(), "bad .fill syntax");
        }

        bool update() override
        {
            error(n.type() != A_m0, "bad .fill syntax");
            word old = size;
            size = n.eval();
            return old != size;
        }

        void flush(output& out) override
        {
            error(n.type() != A_m0, "bad .fill syntax");
            error(v.type() != A_m0, "bad .fill syntax");
            word count = n.eval(), value = v.eval();
            while (count--)
            {
                out.put_byte(value);
            }
        }

        expr n, v;
    };
    return make_insn<insn_fill>(it);
}

ref<insn> pseudo_align(expr_iterator it, byte n, byte d, bool f)
{
    struct insn_align : insn
    {
        insn_align(expr&& e) : align(std::move(e))
        {}

        bool update() override
        {
            error(align.type() != A_m0, "bad .fill syntax");
            word old = size;
            word a = align.eval();
            size = (a - state::dot().value % a) % a;
            return old != size;
        }

        void flush(output& out) override
        {
            word n = size;
            while (n--)
            {
                out.put_byte(0);
            }
        }

        expr align;
    };
    return make_insn<insn_align>(it.exactly_one());
}
