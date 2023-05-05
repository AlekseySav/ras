#pragma once
#include "../src/insn/insn.h"

inline std::vector<ref<insn>> program;

void first_pass(lexer& lex);
void second_pass(output& out);
