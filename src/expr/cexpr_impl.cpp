#include "expr.h"

static byte _prec[] = {
/*  eo nu sy << >> == != <= >= -                   */
    9, 0, 0, 3, 3, 5, 5, 4, 4, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*     !           %  &           *  +     -     / */
    0, 1, 0, 0, 0, 1, 6, 0, 0, 0, 1, 2, 0, 2, 0, 1,
/*                                   <  =  >     ? */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 5, 4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*                                   [     ]  ^     */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 8, 7, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*                                      |           */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0,
};

static byte _args[] = {
/*  eo nu sy << >> == != <= >= -                   */
    0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*     !           %  &           *  +     -     / */
    0, 1, 0, 0, 0, 2, 2, 0, 0, 0, 2, 2, 0, 2, 0, 2,
/*                                   <  =  >     ? */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*                                   [     ]  ^     */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*                                      |           */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
};

void cexpr_impl::append(token t)
{
    if (!_lastsym && t == '-')
    {
        t = L_neg;
    }

    _lastsym = _prec[t] == 0;
    if (t != '[')
    {
        flush(t);
    }
    if (t == ']')
    {
        error(_stack.pop() != '[', "Bad brace balance");
    }
    else
    {
        _stack.push(t);
    }

    if (t == L_sym)
    {
        _syms.push(&symbol::lookup(lval<string>(t)));
    }
    else if (t == L_num)
    {
        _nums.push(lval<word>(t));
    }
}

void cexpr_impl::flush(byte until)
{
    while (_stack.size() != 0 && _prec[_stack.top()] <= _prec[until])
    {
        _program.push(_stack.pop());
    }
}

typeinfo cexpr_impl::type() const
{
    typeinfo t1, t2;
    if (_syms.size() == 0) return A_m0;
    if (_syms.size() == 1) return (*_syms.begin())->type;

    for (symbol* sym : _syms)
    {
        t2 = sym->type;
        error(!(t2.type & (A_m0 | A_mm | A_im)), "bad expr type");
        error(t1.type != A_m0 && t2.type != A_m0, "bad expr type");
        t1 = t2;
    }

    return t1;
}

void cexpr_impl::assert_defined() const
{
    for (symbol* sym : _syms)
    {
        error(!sym->defined(), "symbol <{}> used, but never defined", sym->name.data());
    }
}

bool cexpr_impl::defined() const
{
    for (symbol* sym : _syms)
    {
        if (!sym->defined())
        {
            return false;
        }
    }
    return true;
}

word cexpr_impl::eval() const
{
    word a, b;
    auto sym = _syms.begin();
    auto num = _nums.begin();
    _stack.clear();

    if (_program.size() == 0) return 0;

    for (auto it = _program.begin(); it != _program.end(); ++it)
    {
        byte opcode = *it;
        if (_args[opcode] == 2)
        {
            error(_stack.size() == 0, "bad expression syntax");
            b = _stack.pop();
        }
        if (_args[opcode] != 0)
        {
            error(_stack.size() == 0, "bad expression syntax");
            a = _stack.pop();
        }
        switch (opcode)
        {
            case '?':
            {
                opcode = *++it;
                error(opcode != L_sym, "Bad <?> expression syntax");
                _stack.push((*sym++)->defined());
                break;
            }
            case L_num: _stack.push(*num++); break;
            case L_sym: _stack.push((*sym++)->value); break;
            case L_neg: _stack.push(-a); break;
            case '!':   _stack.push(~a); break;
            case '*':   _stack.push(a * b); break;
            case '/':   _stack.push(a / b); break;
            case '+':   _stack.push(a + b); break;
            case '-':   _stack.push(a - b); break;
            case L_shl: _stack.push(a << b); break;
            case L_shr: _stack.push(a >> b); break;
            case '<':   _stack.push(a < b); break;
            case '>':   _stack.push(a > b); break;
            case L_leq: _stack.push(a <= b); break;
            case L_geq: _stack.push(a >= b); break;
            case L_equ: _stack.push(a == b); break;
            case L_neq: _stack.push(a != b); break;
            case '&':   _stack.push(a & b); break;
            case '^':   _stack.push(a ^ b); break;
            case '|':   _stack.push(a | b); break;
        }
    }
    error(_stack.size() != 1, "bad expression syntax");
    return _stack.pop();
}
