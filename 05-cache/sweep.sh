#!/bin/bash
# 第7章: キャッシュ。行列積のループ順、作業領域サイズの段差、2 のべきの幅。
cd "$(dirname "$0")"
gcc -O2 -g -o matmul matmul.c; gcc -O2 -g -o chase chase.c; gcc -O2 -g -o colsum colsum.c; gcc -O2 -g -o stride stride.c
echo "## 行列積 (s)"; for N in 1000 1023 1024 1025 1032; do taskset -c 2 ./matmul ijk $N; done
for N in 1024 2048; do for m in ikj tile; do taskset -c 2 ./matmul $m $N; done; done
gcc -O2 -fno-tree-vectorize -fno-tree-slp-vectorize -o matmul_nv matmul.c; echo '## ベクトル化なし (N=1000)'; for m in ijk ikj; do taskset -c 2 ./matmul_nv $m 1000; done
echo "## 作業領域サイズごとのアクセス時間"; taskset -c 2 ./chase
echo "## 列方向の足し算 (行の幅)"; for W in 1000 1024 1025 1032 2048 2049; do taskset -c 2 ./colsum $W; done
echo "## 連想度 (stride.c): R 個を stride バイト間隔で置いた輪"
for R in 2 4 8 9 10 16 64; do taskset -c 2 ./stride 4096 $R; done
for st in 4096 4160 8192; do taskset -c 2 ./stride $st 16; done
