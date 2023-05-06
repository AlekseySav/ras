#!/bin/python
# sum 2 floats

import sys

atof = lambda x: float(x.replace(',', '.'))

print(f'{atof(sys.argv[1]) + atof(sys.argv[2]):.3f}'.replace('.', ','))
