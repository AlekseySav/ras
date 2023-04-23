#pragma once

#include "../lib/string.h"
#include "../lib/common.h"

enum {
    L_eof,
    L_sym,
    L_num,
    L_shl,      /* << */
    L_shr,      /* >> */
    L_equ,      /* == */
    L_neq,      /* != */
    L_leq,      /* <= */
    L_geq,      /* >= */
    L_neg,      /* -a */
    L_special_count
};

#define L_iscexpr(n) ({ \
    byte x = (n); \
    (x) != ',' && (x) != ';' && (x) != ':' && (x) != '=' && (x) != '(' && (x) != ')' && (x) != '$'; \
})

class token
{
public:
    token(const token& t) : _type(t._type), _value{.number = t._value.number} { _value.symbol = t._value.symbol; }
    token(byte type = 0, word number = 0) : _type{type}, _value{.number = number} {}
    token(byte type, string symbol) : _type{type}, _value{.symbol = symbol} {}
    operator byte() const { return _type; }
private:
    byte _type;
    union {
        word number;
        string symbol;
    } _value;

    template<typename T>
    friend T lval(token t);
};

template<typename Y>
struct __lval_id { constexpr static const int N = 0; };

template<>
struct __lval_id<string> { constexpr static const int N = 1; };

template<>
struct __lval_id<word> { constexpr static const int N = 2; };

template<typename T>
T lval(token t)
{
    if constexpr (__lval_id<T>::N == 1)
    {
        panic(t._type != L_sym, "used symbol-lvalue, whereas type is {}", t._type);
        return t._value.symbol;
    }
    if constexpr (__lval_id<T>::N == 2)
    {
        panic(t._type != L_num, "used number-lvalue, whereas type is {}", t._type);
        return t._value.number;
    }
}
