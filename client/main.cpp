#include "../src/insn/insn.h"
#include "../src/as.h"
#include <iostream>
#include <fstream>

void first_pass(std::vector<ref<insn>>& program, lexer& lex);
void second_pass(std::vector<ref<insn>>& program, output& out);

std::ofstream syms_stream("/dev/null");

void print_symbol(symbol& s)
{
    if (s.is_mutable() || s.type.type & (A_rb | A_rw | A_sr))
    {
        return;
    }
    char buf[16]{};
    fatal(strlen(s.name.data()) >= 14, "too long symbol name");
    strcpy(buf + 2, s.name.data());
    memcpy(buf, &s.value, 2);
    syms_stream.write(buf, 16);
}

int main(int argc, char** argv)
{
    output out(std::cout);
    as::init_builtins();
    state::if_stack.push(true);
    state::dot = &symbol::lookup(string("."));;

    std::vector<ref<insn>> program;

    try {
        for (int i = 1; i < argc; i++)
        {
            if (!strcmp(argv[i], "-s")) {
                syms_stream = std::ofstream(argv[++i]);
                continue;
            }
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

    symbol::visit_symtab(print_symbol);

    return state::errors != 0;
}
