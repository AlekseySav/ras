#include "client.h"
#include "argmatch.h"
#include "../src/as.h"

#include <iostream>
#include <fstream>

static std::ofstream syms_stream("/dev/null"), bin_stream("/dev/null");
static const char* bin_name;

static void print_symbol(symbol& s)
{
    if (s.is_mutable() || s.type.type & (A_rb | A_rw | A_sr))
    {
        return;
    }
    // char buf[16]{};
    // fatal(strlen(s.name.data()) >= 14, "too long symbol name");
    // strcpy(buf + 2, s.name.data());
    // memcpy(buf, &s.value, 2);
    // syms_stream.write(buf, 16);
}

static void push_insn(size_t at, insn_factory f, std::initializer_list<token> from)
{
    std::ifstream is("/dev/null");
    lexer lex(is);
    lex.unget(';');
    for (auto it = std::rbegin(from); it != std::rend(from); ++it)
    {
        lex.unget(*it);
    }
    program[at].code.push_back(f(lex, 0, 0, true));
}

int main(int argc, char** argv)
{
    word section_align = 2;
    output out(std::cout);
    as::init_builtins();
    state::if_stack.push(true);

    std::vector<const char*> files;
    argmatch match(argc, argv, {
        "-M..=0",
        "-M.=0",
        "-M.bits=16",
    });

    program.emplace_back();

    while (!match.done())
    {
        if (match("-s$"))
        {
            syms_stream = std::ofstream(match.at(0));
        }
        else if (match("-U"))
        {
            state::unoptim = true;
        }
        else if (match("-o$"))
        {
            bin_stream = std::ofstream(bin_name = match.at(0));
            out = output(bin_stream);
        }
        else if (match("-D*=*"))
        {
            push_insn(0, assign_insn, {{L_sym, match.at(0)}, '=', {L_num, toint(match.at(1)).extract()}});
        }
        else if (match("-M*=*"))
        {
            symbol::lookup(match.at(0)).make_mutable();
            push_insn(0, assign_insn, {{L_sym, match.at(0)}, '=', {L_num, toint(match.at(1)).extract()}});
        }
        else if (match("-A*"))
        {
            section_align = toint(match.at(0)).extract();
        }
        else if (match("-S*"))
        {
            program.emplace_back(match.at(0));
            push_insn(program.size() - 1, pseudo_align, {{L_num, section_align}});
        }
        else if (match("*"))
        {
            files.push_back(match.at(0));
        }
    }

    try {
        for (const char* file : files)
        {
            std::ifstream is(file);
            fatal(!is.is_open(), "bad input file: {}", file);
            lexer lex(is);
            state::line = 1;
            state::filename = file;
            first_pass(lex);
        }
        if (state::errors == 0)
        {
            second_pass(out);
        }
    }
    catch (...) {}

    if (state::errors != 0 && bin_name)
    {
        remove(bin_name);
    }

    symbol::visit_symtab(print_symbol);

    return state::errors != 0;
}
