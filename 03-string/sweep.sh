#!/bin/bash
# 第5章: 文字列比較。長さを変える表 (実行時の長さ) と、長さをコンパイル時定数にした表。
cd "$(dirname "$0")"
for c in 0 1 2; do gcc -O2 -g -DCMP=$c -o sc_$c str_cmp.c; done
echo "## 実行時の長さ L (ns/比較)"; printf "%-5s %8s %8s %8s\n" L strcmp memcmp 自前
for L in 8 16 32 64 128 256; do r(){ taskset -c 2 ./sc_$1 $L | sed 's/.* \([0-9.]*\) ns.*/\1/'; }; printf "%-5s %8s %8s %8s\n" $L $(r 0) $(r 1) $(r 2); done
echo "## 長さが定数 (ns/比較)。-fno-builtin で組み込み展開を止めた場合と比べる"
for L in 8 32 128; do for c in 0 1; do for o in "" "-fno-builtin"; do gcc -O2 -DLEN=$L -DCMP=$c $o -o k str_cmp.c
  printf "L=%-4s %s %-14s %s\n" $L "$([ $c = 0 ] && echo strcmp || echo memcmp)" "$o" "$(taskset -c 2 ./k | sed 's/.* \([0-9.]*\) ns.*/\1/')"; done; done; done
echo "## 先頭 2 バイトで違うキー (-DEARLY、ns/比較)"; printf "%-5s %8s %8s %8s\n" L strcmp memcmp 自前
for c in 0 1 2; do gcc -O2 -g -DEARLY -DCMP=$c -o se_$c str_cmp.c; done
for L in 8 32 128 256; do r(){ taskset -c 2 ./se_$1 $L | sed 's/.* \([0-9.]*\) ns.*/\1/'; }; printf "%-5s %8s %8s %8s\n" $L $(r 0) $(r 1) $(r 2); done
rm -f sc_? se_? k
