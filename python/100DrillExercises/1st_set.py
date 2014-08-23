#!/usr/bin/python3

import sys

# print(len(sys.stdin.readlines()))

# for l in sys.stdin.readlines():
# 	print(l.replace('\t', ' '), end = '')

# lines = [l.split(',') for l in sys.stdin.readlines()]
# for l in [l[0] for l in lines]: open('col1.txt', 'a').write(l.strip() + '\n')
# for l in [l[1] for l in lines]: open('col2.txt', 'a').write(l.strip() + '\n')

# col1s = [l.strip() for l in open('col1.txt', 'r')]
# col2s = [l.strip() for l in open('col2.txt', 'r')]
# for (col1, col2) in zip(col1s, col2s): print(col1, col2, sep = ',')

# for l in [l for l in sys.stdin.readlines()][:int(sys.argv[1])]: print(l, end = '')

# for l in [l for l in sys.stdin.readlines()][-int(sys.argv[1]):]: print(l, end = '')

# d = {}
# for l in sorted([l.split(',')[0].strip() for l in sys.stdin.readlines()]): d[l] = 1 + (d[l] if l in d else 0)
# for k in d: print(k, d[k])

sorted([list(map(str.strip, l.split(','))) for l in sys.stdin.readlines()], key = lambda l: l[1])
