#include "output.h"
#include "../expr/expr.h"

output::output(std::ostream& os) : _os(os)
{}

void output::put_word(word w)
{
    put<word>(w);
}

void output::put_byte(word w) // [-128, 255]
{
    error(!is_byte(w), "byte size exceeded: <{}>", w);
    put<byte>(w);
}

void output::put_ub(word w) // [0, 255]
{
    error(!ub(w), "byte size exceeded: <{}>", w);
    put<byte>(w);
}

void output::put_ib(word w) // [-128, 127]
{
    error(!ib(w), "byte size exceeded: <{}>", w);
    put<byte>(w);
}

void output::put_db(word w)
{
    error(!db(w), "byte size exceeded: <{}>", w);
    put<byte>(w - state::dot->value);
}

void output::put_dw(word w)
{
    put<word>(w - state::dot->value);
}

bool output::is_byte(word w)
{
    return w >= -128 && w <= 255;
}

bool output::ub(word w)
{
    return w >= 0 && w <= 255;
}

bool output::ib(word w)
{
    return w >= -128 && w <= 127;
}

bool output::db(word w)
{
    return w >= state::dot->value - 128 && w <= state::dot->value + 127;
}

