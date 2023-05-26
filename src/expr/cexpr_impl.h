#pragma once

#include "../lib/common.h"
#include "../lib/pool.h"
#include "../io/lex.h"

enum : uint16_t
{
    A_m0 = 0x001,
    A_m1 = 0x002,    /* 16-bit mod r/m */
    A_m2 = 0x004,    /* 32-bit mod r/m */
    A_rb = 0x008,
    A_rs = 0x010,
    A_rl = 0x020,
    A_sr = 0x040,
    A_im = 0x080,
    A_cr = 0x100,
    A_dr = 0x200,
    A_tr = 0x400,

    A_ms = 0x800,   /* *offset */

    A_mm = A_m1 | A_m2,
    A_rw = A_rs | A_rl
};

struct typeinfo
{
    uint16_t type;
    byte n, x;

    typeinfo(uint16_t type = A_m0, byte n = 0, byte x = 0) : type(type), n(n), x(x) {}

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
