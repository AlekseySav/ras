#pragma once
#include "../src/insn/insn.h"

struct section {
    string name;
    std::vector<ref<insn>> code;

    section(string name = {}) : name(name), code{} {}
    section(section&& s) = default;
};

inline std::vector<section> program;

void add_listing();

void first_pass(lexer& lex);
void second_pass(output& out);

static inline section& lookup(string name)
{
    for (section& s : program)
    {
        if (s.name == name) return s;
    }
    fatal("undefined section: <{}>", name.data());
    return program[0];
}
