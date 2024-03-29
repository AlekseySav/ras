#include "lex.h"
#include <limits.h>

optional<word> toint(const char* s)
{
    char* p;
    long v = strtol(s, &p, 0);
    if (*p == '\0') return (word)v;
    return {};
}

#define _move(c) ({ auto t = c; c = {}; t; })

static char ansichar(char c)
{
    if (c >= '@' && c <= '_' && c != '\\') return c - '@';
    switch (c)
    {
        case 'n': return '\n';
        case 'r': return '\r';
        case 'b': return '\b';
        case 'e': return '\e';
        case 's': return ' ';
        case 't': return '\t';
        case '0': return '\0';
        case 'd': return '\x7f';
        case '>': return '>';
        case '\\': return '\\';
    }
    error("bad escape character: <'\\{}>", c);
    return c;
}

token lexer::touch()
{
    _untok.emplace(get());
    return _untok.top();
}

optional<token> lexer::tryget(byte to)
{
    token t = get();
    if (t == to) return t;
    unget(t);
    return {};
}

void lexer::unget(token t)
{
    _untok.emplace(t);
}

std::string lexer::getstring()
{
    char c;
    std::string s;
    error(get() != '<', "bad string syntax: expected '<'");
    while ((c = nextch()) != '>')
    {
        error(c == '\n', "unclosed string");
        if (c == '\\')
        {
            c = ansichar(nextch());
        }
        s += c;
    }
    return s;
}

char lexer::nextch(bool err_on_eof)
{
    state::line += _lf;
    _lf = false;

    if (_ch == 0)
    {
        _ch = _is.get();
    }
    _lf = _ch == '\n';
    if (_ch == EOF)
    {
        _ch = 0;
        error(err_on_eof, "unexpected <eof>");
    }
    return _move(_ch);
}

void lexer::undoch(char c)
{
    if (c == '\n')
    {
        _lf = false;
    }
    _ch = c;
}

byte lexer::op2(char cond, byte t, byte f)
{
    char c;
    if ((c = nextch()) != cond)
    {
        undoch(c);
        return f;
    }
    return t;
}

/*
 * actual text parsing here
 */

#define _ 0     // bad
#define N 1     // '\n'
#define O 2     // single character
#define S 3     // space
#define Q 4     // '
#define C 5     // /
#define I 6     // ident
#define r 7     // <
#define R 8     // >
#define X 9     // !
#define E 10    // =
#define q 11    // "

static uint8_t _trait[] = {
    O, _, _, _, _, _, _, _, _, S, N, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    S, X, q, I, O, O, O, Q, O, O, O, O, O, O, I, C,
    I, I, I, I, I, I, I, I, I, I, O, O, r, E, R, O,
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    I, I, I, I, I, I, I, I, I, I, I, O, _, O, O, I,
    I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,
    I, I, I, I, I, I, I, I, I, I, I, I, O, I, I, _,
};

token lexer::get()
{
    if (_untok.size())
    {
        token t = std::move(_untok.top());
        _untok.pop();
        return t;
    }

    char c, t;
    word n = 0;
lex:
    switch (t = _trait[c = nextch(false)])
    {
        case N: return ';';
        case O: return c;
        case S: goto lex;
        case q: goto dquote;
        case Q: goto quote;
        case C: goto comment;
        case I: goto ident;
        case r: return op2(c) ? L_shl : op2('=', L_leq, c);
        case R: return op2(c) ? L_shr : op2('=', L_geq, c);
        case X: return op2('=', L_neq, '!');
        case E: return op2('=', L_equ, '=');
    }
    error("invalid character");
    goto lex;

dquote:
    n = ((c = nextch()) == '\\') ? ansichar(nextch()) : c;
quote:
    c = ((c = nextch()) == '\\') ? ansichar(nextch()) : c;
    return token{L_num, t == Q ? c : n | c << 8};

    // valid comments are '/*...*/' '//...' and '/ ...'
comment:
    c = nextch();
    switch (c)
    {
        case '*': goto comm;
        case ' ':
        case '\t':
        case '/':
            while ((c = nextch()) != '\n');
        case '\n':
            return token{';'};
    }
    undoch(c);
    return token{'/'};
comm:
    c = nextch();
com1:
    if (c != '*') goto comm;
    if ((c = nextch()) != '/') goto com1;
    goto lex;

ident:
    std::string s;
    do {
        s += c;
    } while (_trait[c = nextch()] == I);
    undoch(c);

    auto w = toint(s.c_str());
    if (w) return token{L_num, std::move(w).extract()};
    return token{L_sym, string(s.c_str())};
}
