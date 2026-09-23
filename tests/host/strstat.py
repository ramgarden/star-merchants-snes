import re

src = open("src/main.c").read()
strs = re.findall(r'"((?:[^"\\]|\\.)*)"', src)
tot = sum(len(s) + 1 for s in strs)
print("literals:", len(strs), "bytes ~", tot)
from collections import Counter

src = open("src/main.c").read()
strs = re.findall(r'"((?:[^"\\]|\\.)*)"', src)
tot = sum(len(s) + 1 for s in strs)
print("literals:", len(strs), "bytes ~", tot)
for s in sorted(strs, key=len, reverse=True)[:6]:
    print(len(s) + 1, repr(s[:64]))
print("--- repeats (>=3x, >=3 chars) ---")
c = Counter(strs)
waste = 0
for s, n in c.most_common(20):
    if n >= 3 and len(s) >= 2:
        print(n, repr(s), "waste", (n - 1) * (len(s) + 1))
        waste += (n - 1) * (len(s) + 1)
print("total dedupable ~", waste)
