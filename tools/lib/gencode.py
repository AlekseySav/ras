#
# code generator
#
from lib.insn import Insn, Arg, create_insn

error = 'bad combination of opcodes & operands'

code_header = '''/*
 * opcodes.cpp
 */
#include "insn/insn.h"
#include "as.h"
#define A_xr 0
'''

code_insn = '''
struct {insn.name}_insn : insn
{{
    pool<expr, {insn.max_args}> args;
    byte n, insn_size, rule;
    bool fixed_size;

    {insn.name}_insn(expr_iterator it, byte n, byte size, bool fixed_size)
    : n(n), insn_size(size), rule(200), fixed_size(fixed_size)
    {{
        for (expr e : it)
        {{
            error(args.size() >= {insn.max_args}, "{error}");
            args.push(std::move(e));
        }}
    }}

    bool update() override
    {{
        byte old = rule, olds = size;
        {update}
        error("{error}");
    end:
        return old != rule || olds != size;
    }}

    void flush(output& out)
    {{
        word this_size = size;
        #define size insn_size
        switch (rule)
        {{
        {flush}
        }}
        #undef size
    }}
}};

static ref<insn> {insn.name}_group(expr_iterator it, byte n, byte default_size, bool fixed_size)
{{
    return make_insn<{insn.name}_insn>(it, n, default_size, fixed_size);
}}
'''

code_update = '''
if{L1}:
    if ({cond})
    {{
        {varsize}word s = {size};
        {varsize}if (s == 3) goto if{L2};
        {varsize}if (s)
        {varsize}{{
        {varsize}   if (fixed_size && s - 1 != insn_size) goto if{L2};
        {varsize}   insn_size = s - 1;
        {varsize}}}
        #define db(x)   (size += 1)
        #define dw(x)   (size += 2)
        #define ib(x)   (size += 1)
        #define ub(x)   (size += 1)
        #define im(x)   (size += 1 << insn_size)
        #define byte(x) (size += 1)
        #define word(x) (size += 2)
        #define mr(x)   (size += as::modrm_size({modrm}))
        size = 0;
        {code};
        #undef db
        #undef dw
        #undef ib
        #undef ub
        #undef im
        #undef byte
        #undef word
        #undef mr
        if (!({post})) goto if{L2};
        rule = {L1};
        goto end;
    }}
'''

code_flush = '''case {L1}:
    #define rr {rr}.type().n
    #define sr {sr}.type().n
    #define db(x)   out.put_db(x - this_size)
    #define dw(x)   out.put_dw(x - this_size)
    #define ib(x)   out.put_ib(x)
    #define ub(x)   out.put_ub(x)
    #define im(x)   (insn_size ? out.put_word(x) : out.put_byte(x))
    #define byte(x) out.put_byte(x)
    #define word(x) out.put_word(x)
    #define mr(x)   as::put_modrm(out, {modrm}, x)
    #define cnt {cnt}
    {code};
    #undef rr
    #undef sr
    #undef db
    #undef dw
    #undef ib
    #undef ub
    #undef im
    #undef byte
    #undef word
    #undef mr
    #undef cnt
    break;
'''

code_table = '''
struct opcode
{{
    const char* name;
    ref<insn> (*group)(expr_iterator it, byte n, byte default_size, bool fixed_size);
    byte n, default_size;
    bool fixed_size;
}} opcodes[] = {{
    {opcodes}
}};

optional<ref<insn>> opcode(const char* name, expr_iterator it)
{{
    int l = 0, h = {count} - 1, m, r;
    while (l <= h)
    {{
        m = (l + h) / 2;
        if (!(r = strcmp(name, opcodes[m].name)))
        {{
            struct opcode& o = opcodes[m];
            return o.group(it, o.n, o.default_size, o.fixed_size);
        }}
        if (r < 0)
        {{
            h = m - 1;
        }}
        else
        {{
            l = m + 1;
        }}
    }}
    return {{}};
}}
'''

def shift_spaces(s: str, n: int):
    return (' ' * n).join(s.splitlines(keepends=True))

def conj(conds):
    return ' && '.join(conds)

def comm(cond):
    return '// ' if not cond else ''

def update(insn: Insn) -> str:
    res = []
    for label, case in enumerate(insn.body):
        res.append(code_update.format(
            L1=label,
            L2=label+1,
            cond=conj([f'args.size() == {len(case.args)}'] + [a.condition() for a in case.args]),
            post=conj(['true'] + [a.postcheck() for a in case.args]),
            size=' | '.join(['0'] + [a.wanted_size() for a in case.args]),
            varsize=comm(insn.size == 'b'),
            modrm=case.modrm(),
            code=case.code
        ))
    res.append(f'if{len(insn.body)}:')
    return shift_spaces('\n'.join(res), 4)

def replace_i0(code: str) -> str:
    for i in range(10):
        code = code.replace(f'i{i}', f'args[{i}].eval()')
    return code

def flush(insn: Insn) -> str:
    res = []
    for label, case in enumerate(insn.body):
        res.append(code_flush.format(
            L1=label,
            L2=label+1,
            modrm=case.modrm(),
            rr=case.rr(),
            sr=case.sr(),
            code=replace_i0(case.code),
            cnt=case.cnt
        ))
    return shift_spaces('\n'.join(res), 8)
