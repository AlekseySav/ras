#pragma once
#include <stack>
#include <istream>
#include <string>

#include "token.h"
#include "../lib/optional.h"

class lexer
{
public:
    lexer(std::istream& is) : _lf{false}, _ch{0}, _is{is} {}

    token get();
    token touch();
    optional<token> tryget(byte to);

    void unget(token t);

    std::string getstring();
    char nextch(bool err_on_eof = true);
private:
    byte op2(char cond, byte t = 1, byte f = 0);

    bool _lf;
    char _ch;
    std::istream& _is;
    std::stack<token> _untok;
};
