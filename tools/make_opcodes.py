#!/bin/python3


from lib.insn import Insn, create_insn
import lib.gencode as gc


import yaml
import sys

with open(sys.argv[1]) as f:
    dt: dict[str, dict | list] = yaml.safe_load(f)
opcodes = dt['opcodes']
dt.pop('opcodes')

sizes = {'pseudo': 'w', 'simple': 'w'}

print(gc.code_header)
for header, rules in dt.items():
    insn: Insn = create_insn(header, rules)
    sizes[insn.name] = insn.size
    print(gc.code_insn.format(insn=insn, error=gc.error, update=gc.update(insn), flush=gc.flush(insn)))

items = []

for name, group in opcodes.items():
    group = group.split()
    if len(group) > 1:
        group, n = group
    else:
        group, = group
        n = 0
    group_name = f'{group}_group' if group != 'pseudo' else f'pseudo_{name[1:]}'

    match sizes[group]:
        case 'a':
            items.append(f'{{"{name}b", {group_name}, 0x{n}, 0, true}}')
            items.append(f'{{"{name}w", {group_name}, 0x{n}, 1, true}}')
        case 'b':
            items.append(f'{{"{name}b", {group_name}, 0x{n}, 0, true}}')
            items.append(f'{{"{name}", {group_name}, 0x{n}, 1, false}}')
        case 'w':
            items.append(f'{{"{name}", {group_name}, 0x{n}, 1, true}}')

print(gc.code_table.format(opcodes=',\n\t'.join(sorted(items)), count=len(items)))
