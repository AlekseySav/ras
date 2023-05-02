#include "output.h"
#include "../expr/expr.h"

template<size_t S, word Min, word Max>
static void out(output* out, word w)
{
    error(!(w >= Min && w <= Max), "byte size exceeded: <{}>", w);
    out->put<S>(w);
}

void output::put_byte(word w) { out<1, -128, 255>(this, w); }
void output::put_ub(word w) { out<1, 0, 255>(this, w); }
void output::put_ib(word w) { out<1, -128, 127>(this, w); }
void output::put_word(word w) { out<2, -32768, 65535>(this, w);  }
void output::put_uw(word w) { out<2, 0, 65535>(this, w); }
void output::put_iw(word w) { out<2, -32768, 32767>(this, w); }
void output::put_long(word w) { put<4>(w); }
