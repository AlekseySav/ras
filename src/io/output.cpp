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
    error(!(w >= -128 && w <= 255), "byte size exceeded: <{}>", w);
    put<byte>(w);
}

void output::put_ub(word w) // [0, 255]
{
    error(!(w >= 0 && w <= 255), "byte size exceeded: <{}>", w);
    put<byte>(w);
}

void output::put_ib(word w) // [-128, 127]
{
    error(!(w >= -128 && w <= 127), "byte size exceeded: <{}>", w);
    put<byte>(w);
}
