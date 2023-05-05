#pragma once
#include "expr.h"

class expr_iterator
{
public:
    expr_iterator(lexer& lex) : lex(lex), done(lex.touch() == ';') {}
    expr_iterator(lexer& lex, bool done) : lex(lex), done(done) {}

    bool operator==(expr_iterator it) const { return done == it.done; }
    bool operator!=(expr_iterator it) const { return done != it.done; }

    expr_iterator begin() { return *this; }
    expr_iterator end() { return {lex, true}; }

    expr operator*() { return expr(lex); }
    
    expr_iterator& operator++()
    {
        token t = lex.get();
        error(t != ',' && t != ';', "bad expr separator: <{}>", (char)t);
        if (t == ';')
        {
            done = true;
        }
        return *this;
    }

    expr exactly_one()
    {
        error(done, "expected exactly one argument");
        expr e = *(*this);
        ++(*this);
        error(!done, "expected exactly one argument");
        return e;
    }

    lexer& lex;
    bool done;
};
