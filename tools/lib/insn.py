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
        'rb': ('A_rb', '', ''),
        'rw': ('A_rw', '', ''),
        'rr': ('A_rb | A_rw', '', ''),
        'xw': ('A_rw | A_xr', '', ''),
        'm0': ('A_m0', '', ''),
        'dw': ('A_m0', '', ''),
        'db': ('A_m0', '', 'output::db(%%e%%.eval() - size)'),
        'mm': ('A_m0 | A_mm', '', ''),
        'mb': ('A_m0 | A_mm | A_rb', '', ''),
        'mw': ('A_m0 | A_mm | A_rw', '', ''),
        'mr': ('A_m0 | A_mm | A_rb | A_rw', '', ''),
        'im': ('A_im', '', ''),
        'bb': ('A_im', 'output::is_byte(%%e%%.eval())', ''),
        'ib': ('A_im', 'output::ib(%%e%%.eval())', ''),
        'ub': ('A_im', 'output::ub(%%e%%.eval())', ''),
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

    def wanted_size(self) -> str:
        if not self.change_size:
            return '0'
        if 'A_rw' not in self.ctraits and 'A_rb' not in self.ctraits:
            return '0'
        if 'A_rw' not in self.ctraits:
            return '1'
        if 'A_rb' not in self.ctraits:
            return '2'
        return f'(args[{self.index}].type().type == A_rb ? 1 : args[{self.index}].type().type == A_rw ? 2 : 0)'


@dataclass(slots=True)
class Case:
    args: list[Arg]
    code: str
    cnt: int

    def arg(self, i) -> str:
        return f'args[{i}]'

    def modrm(self) -> str:
        for i, a in enumerate(self.args):
            if 'A_mm' in a.ctraits or 'A_xr' in a.ctraits:
                return self.arg(i)
        return '?'

    def sr(self) -> str:
        for i, a in enumerate(self.args):
            if 'A_sr' in a.ctraits:
                return self.arg(i)
        return '?'

    def rr(self) -> str:
        for i, a in enumerate(self.args):
            if ('A_rb' in a.ctraits or 'A_rw' in a.ctraits) and 'A_mm' not in a.ctraits and not a.prefix_check:
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
    RE = r'(\w*)\s*\((.)(,[\w,]*)?(\[,((,?.)*)\])?\)'
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
