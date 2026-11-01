#!/bin/bash
# 第8章: 分岐の成立確率と、分岐あり/cmov/マスクの比較。ビルド:
#   gcc -O2 -o branch branch.c                                    (既定: if が cmov になる)
#   gcc -O2 -fno-if-conversion -fno-if-conversion2 -fno-tree-loop-if-convert -fno-tree-vectorize -fno-tree-phiprop -o branch_nocmov branch.c  (実際に分岐させる)
cd "$(dirname "$0")"
for P in 0 1 5 10 25 50 75 90 100; do
  printf "P=%-3s 分岐あり %s | cmov %s | マスク %s\n" $P "$(taskset -c 2 ./branch_nocmov branchy $P | sed 's/.* \([0-9.]*\) ns.*/\1/')" "$(taskset -c 2 ./branch branchy $P | sed 's/.* \([0-9.]*\) ns.*/\1/')" "$(taskset -c 2 ./branch_nocmov branchless $P | sed 's/.* \([0-9.]*\) ns.*/\1/')"
done
