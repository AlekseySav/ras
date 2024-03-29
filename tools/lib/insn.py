#
# insn class -- repseresnts one instruction group
#

from dataclasses import dataclass, astuple
import re
import sys

def arg_traits(arg):
    rb = ['al', 'cl', 'dl', 'bl', 'ah', 'ch', 'dh', 'bh']
    rw = ['ax', 'cx', 'dx', 'bx', 'sp', 'bp', 'si', 'di']
    sr = ['es', 'cs', 'ss', 'ds', 'fs', 'gs']
    arg_types = {
        'sr': ('A_sr', '', ''),
        'cr': ('A_cr', '', ''),
        'dr': ('A_dr', '', ''),
        'tr': ('A_tr', '', ''),
        'rb': ('A_rb', '', ''),
        'rw': ('A_rw', '', ''),
        'rs': ('A_rs', '', ''),
        'rl': ('A_rl', '', ''),
        'rr': ('A_rb | A_rw', '', ''),
        'xw': ('A_rw | A_xr', '', ''),
        'xs': ('A_rs | A_xr', '', ''),
        'xl': ('A_rl | A_xr', '', ''),
        'm0': ('A_m0 | A_ms', '', ''),
        'dw': ('A_m0', '', ''),
        'db': ('A_m0', '', 'is_ib(%%e%%, this_size + state::dot().value)'),
        'mm': ('A_m0 | A_ms | A_mm', '', ''),
        'mb': ('A_m0 | A_ms | A_mm | A_rb', '', ''),
        'mw': ('A_m0 | A_ms | A_mm | A_rw', '', ''),
        'ms': ('A_m0 | A_ms | A_mm | A_rs', '', ''),
        'mr': ('A_m0 | A_ms | A_mm | A_rb | A_rw', '', ''),
        'im': ('A_im', '', ''),
        'bb': ('A_im', 'is_byte(%%e%%)', ''),
        'ib': ('A_im', 'is_ib(%%e%%)', ''),
        'ub': ('A_im', 'is_ub(%%e%%)', ''),
        '0':  ('A_im', '%%e%%.eval() == 0', ''),
        '1':  ('A_im', '%%e%%.eval() == 1', '')
    }
    for a, name in [(rb, 'rb'), (rw, 'rw'), (sr, 'sr')]:
        if arg in a:
            return (f'A_{name}', f'%%e%%.type().n == {a.index(arg)}', '')
    return arg_types[arg]


@dataclass(slots=True)
class Arg:
    ctraits: str
    prefix_check: str
    postfix_check: str
    index: int
    change_size: bool

    def __init__(self, size: bool, arg: str, i: int):
        self.index = i
        self.change_size = size
        self.ctraits, self.prefix_check, self.postfix_check = arg_traits(arg)

    def condition(self) -> str:
        p = self.prefix_check.replace('%%e%%', f'args[{self.index}]')
        return f'args[{self.index}].type().type & ({self.ctraits})' + (f' && {p}' if p else '')
    
    def postcheck(self) -> str:
        p = self.postfix_check.replace('%%e%%', f'args[{self.index}]')
        return p if p else 'true'

    def required_size(self) -> str:
        sizes = {'A_rb': 1, 'A_rs': 2, 'A_rl': 4, 'A_rw': 6}
        trait = set(self.ctraits.split()) & set(sizes)
        if not self.change_size:
            return '0xff'
        if not trait:
            return '0xff'
        n = 0
        for i in trait:
            n |= sizes[i]
        return str(n)

    def wanted_size(self) -> str:
        sizes = {'A_rb': 1, 'A_rs': 2, 'A_rl': 4, 'A_rw': 6}
        trait = set(self.ctraits.split()) & set(sizes)
        check = lambda name: f'args[{self.index}].type().type & {name}'
        if not self.change_size:
            return '0'
        if not trait:
            return '0'
        if len(trait) == 1:
            n = next(iter(trait))
            if n == 'A_rw':
                return f'({check("A_rl")} ? 4 : {check("A_rs")} ? 2 : 0)'
            return str(sizes[n])
        return f'({check("A_rb")} ? 1 : {check("A_rs")} ? 2 : {check("A_rl")} ? 4 : 0)'


@dataclass(slots=True)
class Case:
    args: list[Arg]
    code: str
    cnt: int

    def arg(self, i) -> str:
        return f'args[{i}]'

    def modrm(self) -> str:
        for i, a in enumerate(self.args):
            if 'A_mm' in a.ctraits or 'A_xr' in a.ctraits or 'A_m0' in a.ctraits:
                return self.arg(i)
        return '?'

    def sr(self) -> str:
        for i, a in enumerate(self.args):
            if 'A_sr' in a.ctraits:
                return self.arg(i)
        return '?'

    def rr(self) -> str:
        for i, a in enumerate(self.args):
            if ('A_rb' in a.ctraits or 'A_rw' in a.ctraits or 'A_rs' in a.ctraits or 'A_rl' in a.ctraits) and 'A_mm' not in a.ctraits and not a.prefix_check:
                return self.arg(i)
        return '?'

    def cr(self) -> str:
        for i, a in enumerate(self.args):
            if 'A_cr' in a.ctraits:
                return self.arg(i)
        return '?'

    def dr(self) -> str:
        for i, a in enumerate(self.args):
            if 'A_dr' in a.ctraits:
                return self.arg(i)
        return '?'

    def tr(self) -> str:
        for i, a in enumerate(self.args):
            if 'A_tr' in a.ctraits:
                return self.arg(i)
        return '?'


@dataclass
class Insn:
    name: str
    size: int
    min_args: int
    max_args: int
    body: list[Case]


def create_insn(header, rules) -> Insn:
    RE = r'(\w*)\s*\(([\w\.]+)(,[\w,]*)?(\[,((,?.)*)\])?\)'
    name, size, req, _, opt, _ = re.match(RE, header).groups()
    req, opt = req.split(',')[1:] if req else [], opt.split(',') if opt else []
    sizes = [i == 's' for i in req + opt]
    body = []

    prev_arg = None
    cnt = {}
    for args, code in rules.items():
        if not prev_arg or rules[prev_arg]:
            cnt[args] = 0
        else:
            cnt[args] = cnt[prev_arg] + 1
        prev_arg = args

    for _args, code in reversed(rules.items()):
        args = _args.split(',') if _args else []
        if not code:
            code = last
        last = code
        body.append(Case([Arg(s, a, i) for i, (a, s) in enumerate(zip(args, sizes))], code, cnt[_args]))
    body = list(reversed(body))
    return Insn(name, size, len(req), len(req + opt), body)
