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
    char buf[16]{};
    fatal(strlen(s.name.data()) >= 14, "too long symbol name");
    strcpy(buf + 2, s.name.data());
    memcpy(buf, &s.value, 2);
    syms_stream.write(buf, 16);
}

static ref<insn> make_insn(insn_factory f, std::initializer_list<token> from)
{
    std::ifstream is("/dev/null");
    lexer lex(is);
    lex.unget(';');
    for (auto it = std::rbegin(from); it != std::rend(from); ++it)
    {
        lex.unget(*it);
    }
    return f(lex, 0, 0, true);
}

int main(int argc, char** argv)
{
    output out(std::cout);
    as::init_builtins();
    state::if_stack.push(true);

    std::vector<const char*> files;
    argmatch match(argc, argv, {"-D..=-1", "-M.=0", "-M.bits=16"});

    while (!match.done())
    {
        if (match("-s$"))
        {
            syms_stream = std::ofstream(match.at(0));
        }
        else if (match("-o$"))
        {
            bin_stream = std::ofstream(bin_name = match.at(0));
            out = output(bin_stream);
        }
        else if (match("-D*=*"))
        {
            program.push_back(make_insn(assign_insn, {{L_sym, match.at(0)}, '=', {L_num, toint(match.at(1)).extract()}}));
        }
        else if (match("-M*=*"))
        {
            symbol::lookup(match.at(0)).make_mutable();
            program.push_back(make_insn(assign_insn, {{L_sym, match.at(0)}, '=', {L_num, toint(match.at(1)).extract()}}));
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
