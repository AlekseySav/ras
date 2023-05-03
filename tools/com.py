#!/bin/python3
# put all possible combinations
# usage: <args...> <format>

import sys


fmt = sys.argv[-1]
sys.argv.pop()


def all(s: str, d: dict):
    return [[s] + i for i in d]


def com(arg):
    if arg >= len(sys.argv):
        return [[]]
    res = []
    prev = com(arg + 1)
    for i in sys.argv[arg].split('/'):
        res += all(i, prev)
    return res


for i in com(1):
    print(fmt.format('', *i, nl='\n'))
