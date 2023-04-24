#pragma once

#include <gtest/gtest.h>
#include <sstream>

#include "../../src/insn/insn.h"
#include "../../src/as.h"

#define ASSERT_SEQ(a, b) ASSERT_STREQ(a.data(), b)

#define START() \
    string::clear_strtab(); \
    symbol::clear_symtab(); \
    as::init_builtins(); \
    state::if_stack = {}; \
    state::if_stack.push(true); \
    state::filename = __func__; \
    expr _dot; \
    symbol& dot = symbol::lookup(string(".")); \
    dot.assign(_dot); \
    symbol::dot = &dot; \
    symbol::ddot = &symbol::lookup(string(".."));


#define __LEXER(lex, input, line) \
    std::stringstream ss##line; \
    lexer lex(ss##line); \
    ss##line << input;

#define _LEXER(lex, input, line) __LEXER(lex, input, line)
#define LEXER(lex, input) _LEXER(lex, input, __LINE__)

#define OUTPUT(ss, out) \
    std::stringstream ss; \
    output out(ss);

#define ASSERT_OUT(out, s) \
    ASSERT_EQ(out.size(), sizeof(s) - 1); \
    for (int i = 0; i < sizeof(s); i++) \
        ASSERT_EQ(out[i], s[i]);
