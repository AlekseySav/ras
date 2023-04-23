#include "lex.h"

#define _move(c) ({ auto t = c; c = {}; t; })

static char ansichar(char c)
{
    switch (c)
    {
        case 'n': return '\n';
        case 'r': return '\r';
        case 'b': return '\b';
        case 'e': return '\e';
        case 's': return ' ';
        case 't': return '\t';
        case '0': return '\0';
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
            c = ansichar(c);
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
    if (_ch == EOF)
    {
        _ch = 0;
        error(err_on_eof, "unexpected <eof>");
    }
    _lf = _ch == '\n';
    return _move(_ch);
}

byte lexer::op2(char cond, byte t, byte f)
{
    if ((_ch = nextch()) != cond)
    {
        return f;
    }
    _ch = '\0';
    return t;
}

/*
 * actual text parsing here
 */

#define _ 0     // bad
#define N 1     // '\n'
#define O 2     // single character
#define S 3     // space
#define Q 4     // '\''
#define C 5     // /
#define I 6     // ident
#define r 7     // <
#define R 8     // >
#define X 9     // !
#define E 10    // =

static uint8_t _trait[] = {
    O, _, _, _, _, _, _, _, _, S, N, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    S, X, I, I, O, O, O, Q, O, O, O, O, O, O, I, C,
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

    char c;
lex:
    switch (_trait[c = nextch(false)])
    {
        case N: return ';';
        case O: return c;
        case S: goto lex;
        case Q: goto quote;
        case C: goto comment;
        case I: goto ident;
        case r: return op2(c) ? L_shl : op2('=', L_leq, c);
        case R: return op2(c) ? L_shr : op2('=', L_geq, c);
        case X: return op2('=', L_neq, '!');
        case E: return op2('=', L_equ, '=');
    }
    error("invalid character");
    return 0;

quote:
    if ((c = nextch()) != '\\')
    {
        return token{L_num, c};
    }
    return token{L_num, ansichar(nextch())};

    // valid comments are '/*...*/' '//...' and '/ ...'
comment:
    c = nextch();
    switch (c)
    {
        case '*': goto comm;
        case ' ':
        case '/':
            while ((c = nextch()) != '\n');
            goto lex;
    }
    _ch = c;
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
    _ch = c;

    char* p;
    auto v = strtoul(s.c_str(), &p, 0);
    if (*p == '\0')
    {
        error(v > 0xffff, "number overflow: {}", v);
        return token{L_num, (word)v};
    }
    return token{L_sym, string(s.c_str())};
}
