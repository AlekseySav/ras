#include "client.h"
#include "argmatch.h"
#include "../src/as.h"

#include <iostream>
#include <unordered_map>
#include <fstream>

static std::ofstream string_stream("/dev/null"), bin_stream("/dev/null"), list_stream("/dev/null");
static std::unordered_map<const char*, int> file_ids;
static const char* bin_name;

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

static void print_symbol(symbol& s)
{
    if (s.is_mutable() || (s.type.type & A_m0) == 0) return;

    string_stream.write((char*)&s.value, 2);
    string_stream.write(s.name.data(), strlen(s.name.data()) + 1);
}

void add_listing()
{
    list_stream.write((char*)&state::dot().value, 2);
    list_stream.write((char*)&state::line, 2);
    list_stream.write((char*)&file_ids[state::filename], 2);
}

static void usage() {
    fprintf(stderr, "usage: ras [options...] files...\n"
        "  -h           print this [h]elp\n"
        "  -f           perform [f]aster compilation, less optimizations\n"
        "  -o file      specify [o]utput (defaults to stdout)\n"
        "  -s file      save [s]tring table\n"
        "  -g file      [g]enerate and save symbol table\n"
        "  -Lname       define [L]abel\n"
        "  -Dname=value [D]efine immutable symbol\n"
        "  -Mname=value define [M]utable symbol\n"
        "  -Avalue      specify default section [A]lignment\n"
        "  -Sname       define [S]ection\n"
    );
    exit(1);
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
        if (match("-h"))
        {
            usage();
        }
        if (match("-f"))
        {
            state::unoptim = true;
        }
        else if (match("-o$"))
        {
            bin_stream = std::ofstream(bin_name = match.at(0));
            out = output(bin_stream);
        }
        else if (match("-s$"))
        {
            string_stream = std::ofstream(match.at(0));
        }
        else if (match("-g$"))
        {
            list_stream = std::ofstream(match.at(0));
        }
        else if (match("-L*"))
        {
            push_insn(program.size() - 1, label_insn, {{L_sym, match.at(0)}, ':'});
        }
        else if (match("-D*=*"))
        {
            push_insn(program.size() - 1, assign_insn, {{L_sym, match.at(0)}, '=', {L_num, toint(match.at(1)).extract()}});
        }
        else if (match("-M*=*"))
        {
            symbol::lookup(match.at(0)).make_mutable();
            push_insn(program.size() - 1, assign_insn, {{L_sym, match.at(0)}, '=', {L_num, toint(match.at(1)).extract()}});
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
            list_stream.write(match.at(0), strlen(match.at(0)) + 1);
            file_ids[match.at(0)] = files.size();
            files.push_back(match.at(0));
        }
    }
    list_stream << '\0';

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
        symbol::visit_symtab(print_symbol);
    }
    catch (...) {}

    if (state::errors != 0 && bin_name)
    {
        remove(bin_name);
    }

    return state::errors != 0;
}
