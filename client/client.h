#pragma once
#include "../src/insn/insn.h"

struct section {
    string name;
    std::vector<ref<insn>> code;

    section(string name = {}) : name(name), code{} {}
    section(section&& s) = default;
};

inline std::vector<section> program;

// inline std::vector<ref<insn>> program;

void first_pass(lexer& lex);
void second_pass(output& out);
