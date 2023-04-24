#include "../src/insn/insn.h"
#include "../src/as.h"
#include <iostream>
#include <fstream>

void first_pass(std::vector<ref<insn>>& program, lexer& lex);
void second_pass(std::vector<ref<insn>>& program, output& out);

int main(int argc, char** argv)
{
    output out(std::cout);
    as::init_builtins();
    state::if_stack.push(true);
    symbol::dot = &symbol::lookup(string("."));;

    std::vector<ref<insn>> program;

    try {
        for (int i = 1; i < argc; i++)
        {
            std::ifstream is(argv[i]);
            lexer lex(is);
            state::line = 0;
            state::filename = argv[i];
            first_pass(program, lex);
        }
        if (state::errors == 0)
        {
            second_pass(program, out);
        }
    }
    catch (...) {}

    return state::errors != 0;
}
