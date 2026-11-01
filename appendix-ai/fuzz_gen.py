#!/usr/bin/env python3
# 差分ファジング用の入力生成: 第3章の集計プログラムの入力として「ありそうな壊れ方」を乱数で組み合わせる。固定シードで再現可能。
# 使い方: python3 fuzz_gen.py [行数] > fuzz.csv      (NUL を含む行を少数混ぜる。NUL で文字列が終わる扱いは v0r の仕様 6)
import random, sys
random.seed(20261101)
n = int(sys.argv[1]) if len(sys.argv) > 1 else 20000
dates = ['2026-01-01', '2026-12-31', '', 'x', '2026-01-01 00:00:00']
stores = ['S%04d' % random.randrange(1000) for _ in range(40)] + ['S0000', 'S0999', 'S1000', 'S-2', 'S', '', 'S99999999999', 's0001', 'S0001x', 'S 1', 'S+3', 'S00001', 'S1', 'S00']
prods = ['P0001', '', 'P' * 100, ' ', '"a,b"']
qtys = ['1', '', '0', '-1', 'x']
amts = ['1.00', '12', '2.5', '-2.50', '+3.5', '1e3', '.5', '5.', '0.00', '9999.99', '99999999999999.99', '123456789012345678.00', '', 'abc', ' 7.25', '1.006', '0.1', '0.2', '0.3', '1,000.00']
def field(pool): return random.choice(pool)
out = []
for _ in range(n):
    r = random.random()
    if r < 0.45:                                   # ほぼ正常な行
        line = ','.join([field(dates[:2]), 'S%04d' % random.randrange(1000), 'P%04d' % random.randrange(5000), str(random.randrange(1, 9)), '%d.%02d' % (random.randrange(10000), random.randrange(100))])
    elif r < 0.85:                                 # 欄の中身を壊す
        line = ','.join([field(dates), field(stores), field(prods), field(qtys), field(amts)])
    elif r < 0.93:                                 # 欄の数を変える
        k = random.choice([0, 1, 2, 3, 4, 6, 7])
        line = ','.join(field(dates + stores + prods + qtys + amts) for _ in range(k))
    elif r < 0.97:
        line = ''
    else:                                          # 乱数の文字
        line = ''.join(chr(random.randrange(1, 127)) for _ in range(random.randrange(0, 60))).replace('\n', ' ').replace('\r', ' ')
    if random.random() < 0.01 and line:                # NUL を欄の途中・末尾に置く
        k = random.randrange(len(line) + 1); line = line[:k] + '\0' + line[k:]
    out.append(line + ('\r' if random.random() < 0.03 else '') + '\n')
sys.stdout.write(''.join(out))
