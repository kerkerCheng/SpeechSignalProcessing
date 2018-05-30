import numpy as np
import sys
from collections import defaultdict

f = open(sys.argv[1], 'r', encoding='big5-hkscs')
chs = []
phs = []
for line in f:
    seg = line.strip().split()
    phones = seg[1].split('/')
    for ph in phones:
        chs.append(seg[0])
        phs.append(ph)

f.close()

dic = defaultdict(list)
for i, ph in enumerate(phs):
    if not (chs[i] in dic[ph[0]]):
        dic[ph[0]].append(chs[i])

f_out = open(sys.argv[2], 'w', encoding='big5-hkscs')
sorted_key = ''.join(sorted(dic.keys()))

for key in sorted_key:
    f_out.write((key + "      "))
    f_out.write((" ".join(dic[key])))
    f_out.write('\n')
    for word in dic[key]:
        f_out.write(word + "      " + word)
        f_out.write('\n')
f_out.close()
