#!/bin/bash
# 使い方: ./bench.sh ./v0 <入力CSV> [繰り返し回数]   (実行ファイルは ./ 付きで。bash 経由なので bash の起動 (約 1.6 ms) が含まれる)
# 注: perf stat -r で入力を < で渡すと 2 回目以降は EOF になるため、1 回ごとに bash で開き直している
# CPU 2 に固定して N 回測り、perf stat の要約を出す。標準出力は捨てる。
exe=$1; in=$2; n=${3:-5}
[ -x "$exe" ] || { echo "実行ファイルがない (./ 付きで指定): $exe" >&2; exit 2; }
perf stat -r "$n" -e task-clock,cycles,instructions,branch-misses,cache-misses \
  taskset -c 2 bash -c "$exe < $in > /dev/null" 2>&1 | grep -E 'task-clock|cycles|instructions|branch-misses|cache-misses|elapsed'
