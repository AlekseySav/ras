#pragma once
/*
 * expr: rb | rw | sr | ['$'] [cexpr] ['(' disp ')']
 *
 * cexpr:
 *  '[' cexpr ']' |
 *  ['-' '!' '?'] cexpr |
 *  cexpr ['+' '-' '*' '/' '<' '>' '==' '!=' '|' '&' '^' '<<' '>>'] cexpr
 */

#include "../lib/string.h"
#include "../io/lex.h"
#include "cexpr_impl.h"

class cexpr
{
public:
    bool compile(lexer& lex);
    void compile(std::initializer_list<token> list);

    /* 2-nd stage */
    inline typeinfo type() const { return _impl.type(); }
    inline word eval() const { return _impl.eval(); }
    inline void assert_defined() const { _impl.assert_defined(); }
    inline bool defined() const { return _impl.defined(); }
private:
    cexpr_impl _impl;
};

class expr
{
public:
    expr() = default;
    expr(typeinfo t);
    expr(cexpr&& c);
    expr(lexer& lex);

    /* 2-nd stage */
    inline bool defined() const { return _expr.defined(); }
    typeinfo type() const;
    word eval() const;
private:
    typeinfo _type;
    cexpr _expr;
    bool _const;
};

class symbol
{
public:
    string name;
    inline bool defined() const { return _expr; }
    inline bool is_mutable() const { return _mutable; }

    /* 1-st stage */
    void make_mutable();

    static void clear_symtab();
    static void visit_symtab(void (*callback)(symbol& sym));
    static symbol& lookup(string name, bool define = false);
    static symbol& lookup(byte label);

    /* 2-nd stage */
    word value;
    typeinfo type;
    void assign(expr& expr);
    void update();
private:
    expr* _expr;
    byte _mutable: 1;
    byte _used: 1;
};

namespace state
{
#define SYMBOL(func, name) \
    inline symbol& func() \
    { \
        static symbol* d; \
        if (!d) d = &symbol::lookup(name); \
        return *d; \
    }

    SYMBOL(dot, ".");
    SYMBOL(ddot, "..");
    SYMBOL(bits, ".bits");
}
