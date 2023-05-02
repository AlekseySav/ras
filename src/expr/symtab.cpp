#include "expr.h"
#include <vector>
#include <array>

static std::array<symbol, MAX_SYMBOLS> _symtab;
static pool<symbol, MAX_NUMERIC_LABELS> _numerics;
static std::array<symbol*, 10> _f, _b;

void symbol::clear_symtab()
{
    for (symbol& s : _symtab) s = {};
    for (symbol*& s : _f) s = nullptr;
    for (symbol*& s : _b) s = nullptr;
    _numerics.clear();
}

void symbol::visit_symtab(void (*callback)(symbol& sym))
{
    for (symbol& s : _symtab)
    {
        if (s.name.id())
        {
            callback(s);
        }
    }
}

symbol& symbol::lookup(string name, bool define)
{
    if (!isdigit(name.data()[0]))
    {
        symbol& s = _symtab[name.id()];
        s.name = name;
        return s;
    }

    const char* p = name.data();

    error(define, "unable to define symbol with name <{}>", p);
    error(strlen(p) != 2 || p[1] != 'f' && p[1] != 'b', "bad numeric label <{}>", p);
    int n = p[0] - '0', c = p[1];

    if (c == 'b')
    {
        error(_b[n] == nullptr, "undefined 'b' numeric label: <{}{}>", n, c);
        return *_b[n];
    }
    if (_f[n] == nullptr)
    {
        _f[n] = &_numerics.alloc();
    }
    return *_f[n];
}

symbol& symbol::lookup(byte label)
{
    error(label > 10, "numeric labels must be in range 0-9");
    _b[label] = _f[label];
    _f[label] = nullptr;
    if (_b[label] == nullptr)
    {
        _b[label] = &_numerics.alloc();
    }
    return *_b[label];
}
