#pragma once

#include "../lib/common.h"
#include "../lib/pool.h"
#include "../io/lex.h"

enum : byte
{
    A_m0 = 1,
    A_mm = 2,
    A_rb = 4,
    A_rw = 8,
    A_sr = 16,
    A_im = 32,
    A_modrm = 64,
};

struct typeinfo
{
    byte type, n;

    typeinfo(byte type = A_m0, byte n = 0) : type(type), n(n) {}

    bool operator==(typeinfo t) const { return t.type == type && t.n == n; }
    bool operator!=(typeinfo t) const { return !(*this == t); }
};

/* m0 | mm */
class cexpr_impl
{
public:
    void append(token t);
    void flush(byte until = L_eof);

    /* 2-nd stage */
    typeinfo type() const;
    word eval() const;
    void assert_defined() const;
    bool defined() const;
private:
    bool _lastsym{false};
    mutable pool<word, MAX_EXPR_SIZE / 2> _stack;
    pool<byte, MAX_EXPR_SIZE> _program;
    pool<class symbol*, MAX_EXPR_SIZE / 2> _syms;
    pool<word, MAX_EXPR_SIZE / 2> _nums;
};
