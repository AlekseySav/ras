#include "../src/insn/insn.h"
#include "../src/as.h"
#include <iostream>
#include <fstream>
#include <list>

#include "argmatch.h"

void first_pass(std::vector<ref<insn>>& program, lexer& lex);
void second_pass(std::vector<ref<insn>>& program, output& out);

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

static symbol& predefine_symbol(const char* name, word value, bool mut)
{
    fatal(!strcmp(name, "."), "symbol <.> can not be predefined");
    static std::list<expr> predefines;
    cexpr e;
    symbol& s = symbol::lookup(string(name));
    if (mut)
    {
        s.make_mutable();
    }
    e.compile({token{L_num, value}});
    s.assign(predefines.emplace_back(std::move(e)));
    s.update();
    return s;
}

int main(int argc, char** argv)
{
    output out(std::cout);
    as::init_builtins();
    state::dot().make_mutable();
    state::if_stack.push(true);

    std::vector<ref<insn>> program;
    std::vector<const char*> files;
    argmatch match(argc, argv, {"-D..=-1", "-M.bits=16"});

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
            predefine_symbol(match.at(0), toint(match.at(1)).extract(), false);
        }
        else if (match("-M*=*"))
        {
            predefine_symbol(match.at(0), toint(match.at(1)).extract(), true);
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
            first_pass(program, lex);
        }
        if (state::errors == 0)
        {
            second_pass(program, out);
        }
    }
    catch (...) {}

    if (state::errors != 0)
    {
        remove(bin_name);
    }

    symbol::visit_symtab(print_symbol);

    return state::errors != 0;
}
