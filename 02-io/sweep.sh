#!/bin/bash
# 第4章: 読み方・書き方ごとの時間 (wall / user / sys の中央値)。事前に bash ../build_all.sh
# 使い方: ./sweep.sh <sales600k.csv> <sales6m.csv> <出力用ディレクトリ>
[ $# -ge 3 ] || { echo "usage: bash 02-io/sweep.sh <600k行CSV> <6M行CSV> <出力ディレクトリ>" >&2; exit 2; }
set -e; S=$(readlink -f "$1"); L=$(readlink -f "$2"); O=$(readlink -f "$3"); mkdir -p "$O"; cd "$(dirname "$0")"; R=../tools/runstat
echo "## read() のバッファサイズ (600k 行 / 19.7MB)"
for b in 1 16 256 4096 65536 1048576; do printf "read %-9d" $b; $R $S 5 2 ./cnt_read $b; done
echo "## read の位置のずれ (6M 行): 第2引数が 16 だとカーネルのコピーが遅い"
for o in 0 16; do printf "read 1MB off=%-3s" $o; $R $L 9 2 ./cnt_read 1048576 $o; done
echo "## stdio の使い方 (6M 行 / 197MB)"
printf "%-20s" "fgets aligned"; $R $L 7 2 ./cnt_stdio fgets 4096 aligned
for m in "fgets" "fgets 1048576" "getline" "getc"; do printf "%-20s" "$m"; $R $L 7 2 ./cnt_stdio $m; done
printf "%-20s" "mmap"; $R $L 7 2 ./cnt_mmap
echo "## 出力 (6M 行, 出力先のファイルシステム: $(stat -f -c %T "$O"))"
for m in buf block flush unbuf; do printf "%-8s" $m; $R - 3 2 ./out_lines $m $O/out_$m.txt; done
