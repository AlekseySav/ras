#!/bin/python3
import re
import yaml
import sys


header_re = re.compile(r'(\w*)\s*\((.)(,.*)?,?(\[(,?.)*\])?\)')

arg_types = {
    'sr': ('A_sr', None),
    'rb': ('A_rb', None),
    'rw': ('A_rw', None),
    'rr': ('A_rb | A_rw', None),
    'm0': ('A_m0', None),
    'mm': ('A_m0 | A_mm', None),
    'mb': ('A_m0 | A_mm | A_rb', None),
    'mw': ('A_m0 | A_mm | A_rw', None),
    'mr': ('A_m0 | A_mm | A_rb | A_rw', None),
    'im': ('A_im', None),
    'bb': ('A_im', 'output::is_byte(%%e%%.eval())'),
    'ib': ('A_im', 'output::ib(%%e%%.eval())'),
    'ub': ('A_im', 'output::ub(%%e%%.eval())'),
    'db': ('A_im', 'output::db(%%e%%.eval())'),
    '0':  ('A_im', '%%e%%.eval() == 0'),
    '1':  ('A_im', '%%e%%.eval() == 1')
}

header = '''#include "insn/insn.h"
#include "as.h"
'''

insn = '''
struct %%group%%_insn : insn
{
    pool<expr, %%max_args%%> args;
    byte n, count, cnt, rule;
    %%size_on%%byte insn_size, default_size;
    %%size_on%%bool fixed_size;
    %%used_rr%%byte rr;
    %%used_sr%%byte sr;

    %%group%%_insn(expr_iterator it, byte n, byte default_size = 1, bool fixed_size = true)
    : n(n), count(0xff)%%size_on%%, default_size(default_size), fixed_size(fixed_size)
    {
        for (expr e : it)
        {
            args.push(std::move(e));
        }
        error(args.size() > %%max_args%% || args.size() < %%min_args%%, "wrong number of arguments");
    }

    bool update() override
    {
        byte oldc = count, olds = size, oldr = rule;
        cnt = 0;
        %%update%%
        return oldc != count || olds != size || oldr != rule;
    }

    void flush(output& out)
    {
        #define size insn_size
        %%flush%%
        #undef size
    }
};
'''

insn_match = '''
if (%%cond%%) // %%comm%%
{
case%%label%%:
    size = 0;
    %%size_on%%insn_size = default_size;
    %%upds_on%%bool b = %%insnsize_b%%;
    %%upds_on%%bool w = %%insnsize_w%%;
    %%upds_on%%error(b && w, "bad combination of opcode & operands");
    %%upds_on%%if (b)
    %%upds_on%%{
    %%upds_on%%    error(fixed_size && insn_size == 1, "bad combination of opcode & operands");
    %%upds_on%%    insn_size = 0;
    %%upds_on%%}
    %%upds_on%%else if (w)
    %%upds_on%%{
    %%upds_on%%    error(fixed_size && insn_size == 0, "bad combination of opcode & operands");
    %%upds_on%%    insn_size = 1;
    %%upds_on%%}
    %%sets%%;
    #define db(x)   (size += 1)
    #define dw(x)   (size += 2)
    #define ib(x)   (size += 1)
    #define ub(x)   (size += 1)
    #define im(x)   (size += insn_size)
    #define byte(x) (size += 1)
    #define word(x) (size += 2)
    #define mr(x)   (size += as::modrm_size(args[%%modrm%%]))
    %%size%%;
    %%undefines%%
    %%code%%;
    goto %%goto%%;
}
'''

flush_defines = '''
    #define db(x)   out.put_db(x)
    #define dw(x)   out.put_dw(x)
    #define ib(x)   out.put_ib(x)
    #define ub(x)   out.put_ub(x)
    #define im(x)   (insn_size ? out.put_word(x) : out.put_byte(x))
    #define byte(x) out.put_byte(x)
    #define word(x) out.put_word(x)
    #define mr(x)   as::put_modrm(out, *modrm, x)
'''

undefines = '''
    #undef db
    #undef dw
    #undef ib
    #undef ub
    #undef im
    #undef byte
    #undef word
    #undef mr
'''


def die(*any):
    print('make_opcodes died:', *any, file=sys.stderr)
    exit(1)


def extract(*names):
    r = [dt[i] for i in names]
    for i in names:
        dt.pop(i)
    return r


def extend(n, list, d):
    return [list[i] if i < len(list) else d for i in range(n)]


def group_header(mnemonic):
    name, size, min, _, ext = header_re.match(mnemonic).groups()
    min = min or ''
    ext = ext or ''
    return name, size, min.split(',')[1:], ext.split(',')


def arg_traits(a: str):  # -> (type, restrict)
    if a in rb:
        return ('A_rb', f'%%e%%.type().n == {rb.index(a)}')
    if a in rw:
        return ('A_rw', f'%%e%%.type().n == {rw.index(a)}')
    if a in sr:
        return ('A_sr', f'%%e%%.type().n == {sr.index(a)}')
    return arg_types[a]


def contains(rules: list[str], what: list[str]):
    for r in rules:
        for i in r.split(','):
            if i in what:
                return True
    return False


def insn_size(sizes: list[str], args: list[str]):
    b = []
    w = []
    for i, (s, a) in enumerate(zip(sizes, args)):
        if s != 's':
            continue
        trait = arg_traits(a)
        if 'A_rb' not in trait[0] and 'A_rw' not in trait[0]:
            continue
        if 'A_rw' not in trait[0]:
            b.append(f'true /* {a} */')
            w.append(f'false /* {a} */')
            continue
        if 'A_rb' not in trait[0]:
            b.append(f'false /* {a} */')
            w.append(f'true /* {a} */')
            continue
        b.append(f'args[{i}].type().type == A_rb /* {a} */')
        w.append(f'args[{i}].type().type == A_rw /* {a} */')

    return ' || '.join(b), ' || '.join(w)


def insn_modrm(args):
    for i, a in enumerate(args):
        a = arg_traits(a)
        if 'A_mm' in a[0]:
            return i
    return 'error'


def replace_im(code):
    for i in range(0, 10):
        code = code.replace(f'i{i}', f'args[{i}].eval()')
    return code


def insn_update(rules: dict[str, str], req: list[str], opt: list[str]) -> str:
    res = ''
    for label, (args, code) in enumerate(rules.items()):
        set = []
        args = args.split(',') if args else []
        b, w = insn_size((req + opt)[:len(args)], args)
        cond = [f'args.size() == {len(args)}']
        for i, a in enumerate(args):
            if a == 'rb' or a == 'rw' or a == 'rr':
                set.append(f'rr = args[{i}].type().n')
            if a == 'sr':
                set.append(f'sr = args[{i}].type().n')
            a = arg_traits(a)
            cond.append(f'args[{i}].type().type & ({a[0]})')
            if a[1]:
                cond.append(a[1].replace('%%e%%', f'args[{i}]'))
        res += (insn_match.
                replace('%%comm%%', ','.join(args)).
                replace('%%cond%%', ' && '.join(cond)).
                replace('%%sets%%', ', '.join(set)).
                replace('%%insnsize_b%%', b).
                replace('%%insnsize_w%%', w).
                replace('%%label%%', str(label)).
                replace('%%modrm%%', str(insn_modrm(args))).
                replace('%%undefines%%', undefines).
                replace('%%size%%', code or '').
                replace('%%size_on%%', '// ' if headers[name] == 'w' else '').
                replace('%%upds_on%%', '// ' if headers[name] != 'b' else '').
                replace('%%code%%', f'rule = {label}' if code else f'cnt++').
                replace('%%goto%%', f'end' if code else f'case{label+1}'))
    res += 'end:'

    return '\n'.join(' ' * 8 + i for i in res.splitlines())


def insn_flush(rules: dict[str, str]):
    lf = '\n    '
    switch = []
    prev_code = None
    for i, (args, code) in enumerate(reversed(rules.items())):
        i = len(rules) - i - 1
        args = args.split(',') if args else []
        code = code or prev_code
        prev_code = code
        setrm = f'modrm = &args[{str(insn_modrm(args))}]; '
        switch.append(f'case {i}: {setrm if insn_modrm(args) != "error" else ""}{replace_im(code)}; break;\n')
    return f'''expr* modrm;
        {lf.join(flush_defines.splitlines())}
        switch (rule)
        {{
            {(' ' * 12).join(switch)}
            default: error("invalid combination of opcode & operands"); break;
        }}
        {lf.join(undefines.splitlines())}
    '''


if len(sys.argv) != 2:
    die(f'usage: {sys.argv[0]} <opcodes.yaml> [>output]')

with open(sys.argv[1]) as f:
    dt: dict[str, dict | list] = yaml.safe_load(f)

rb, rw, sr, mr = extract('rb', 'rw', 'sr', 'mr')
opcodes, = extract('opcodes')
headers = {}

print(header)

for header, rules in dt.items():
    name, size, req, opt = group_header(header)
    headers[name] = size
    print(insn.
          replace('%%group%%', name).
          replace('%%size_on%%', ' // ' if size == 'w' else '').
          replace('%%min_args%%', str(len(req))).
          replace('%%max_args%%', str(len(req + opt))).
          replace('%%used_rr%%', '' if contains(rules, ['rr', 'rb', 'rw']) else '// ').
          replace('%%used_sr%%', '' if contains(rules, ['sr']) else '// ').
          replace('%%update%%', insn_update(rules, req, opt)).
          replace('%%flush%%', insn_flush(rules)))

    print(f'''
static ref<insn> {name}_group(expr_iterator it, byte n, byte default_size = 1, bool fixed_size = true)
{{
    return make_insn<{name}_insn>(it, n, default_size, fixed_size);
}}''')

print(f'''
struct opcode {{
    const char* name;
    ref<insn> (*group)(expr_iterator it, byte n, byte default_size, bool fixed_size);
    byte n, default_size;
    bool fixed_size;
}} opcodes[] = {{''')

op = []

for name, group in opcodes.items():
    group, n = extend(2, group.split(), 0)
    size = headers.get(group, "w")
    group = f'{group}_group' if group != 'pseudo' else f'pseudo_{name[1:]}'
    op.append(f'    {{"{name}{"w" if size == "a" else ""}", {group}, 0x{n}, 1, {"false" if size == "b" else "true"}}},')
    if size != 'w':
        op.append(f'    {{"{name}b", {group}, 0x{n}, 0, true}},')

print('\n'.join(sorted(op)))

print('};')

print(f'''
optional<ref<insn>> opcode(const char* name, expr_iterator it)
{{
    int l = 0, h = {len(op)} - 1, m, r;
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
''')
