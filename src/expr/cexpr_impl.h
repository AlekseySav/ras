#pragma once

#include "../lib/common.h"
#include "../lib/pool.h"
#include "../io/lex.h"

enum : byte
{
    A_m0 = 0x01,
    A_mb = 0x02,
    A_mw = 0x04,
    A_rb = 0x08,
    A_rw = 0x10,
    A_rl = 0x20,
    A_sr = 0x40,
    A_im = 0x80,

    A_mm = A_mb | A_mw
};

struct typeinfo
{
    byte type, n, x;

    typeinfo(byte type = A_m0, byte n = 0, byte x = 0) : type(type), n(n), x(x) {}

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
