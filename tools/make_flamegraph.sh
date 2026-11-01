#!/bin/bash
# 第3章 v0 の flamegraph を作る。外部の perl スクリプト (brendangregg/FlameGraph の stackcollapse-perf.pl と flamegraph.pl) を使う。
# 事前準備 (1 回だけ。コードを確認してから):
#   git clone https://github.com/brendangregg/FlameGraph "$FG" && git -C "$FG" checkout 41fee1f
#   gcc -O2 -o 01-aggregate/gen 01-aggregate/gen.c && 01-aggregate/gen 6000000 > "$IN"
# 使い方: FG=<FlameGraph の場所> IN=<売上CSV> bash tools/make_flamegraph.sh   (出力: 01-aggregate/flamegraph_v0.svg と .folded)
# 注意: 作業ファイル (perf.data は約 130MB) は mktemp -d の場所に作る。/tmp の空きが少ない環境では、TMPDIR=<空きのある実ディスク> を付ける。
set -euo pipefail
HERE=$(cd "$(dirname "$0")/.." && pwd)
: "${FG:?FG に FlameGraph の clone 先を指定}" "${IN:?IN に 600万行の売上CSV を指定}"
case "$(git -C "$FG" rev-parse HEAD)" in 41fee1f*) ;; *) echo "FlameGraph の commit が 41fee1f ではない" >&2; exit 1 ;; esac
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
gcc -O2 -g -fno-omit-frame-pointer -static -o "$W/v0s" "$HERE/01-aggregate/v0_naive.c"
CPU=${CPU:-2}
taskset -c "$CPU" perf record -o "$W/v0.data" -F 10000 --call-graph dwarf,16384 "$W/v0s" < "$IN" > /dev/null 2> "$W/rec.log" || { rc=$?; cat "$W/rec.log" >&2; echo "perf record が失敗 (rc=$rc)。作業先 $W の空き容量 (df -h) と perf_event_paranoid を確認。TMPDIR で作業先を変えられる" >&2; exit 1; }
perf script -i "$W/v0.data" 2>/dev/null | perl "$FG/stackcollapse-perf.pl" > "$W/v0.folded"
perl "$FG/flamegraph.pl" --title "v0: fgets+strtok+atof (6M rows)" "$W/v0.folded" > "$W/v0.svg"
mv "$W/v0.folded" "$HERE/01-aggregate/flamegraph_v0.folded"; mv "$W/v0.svg" "$HERE/01-aggregate/flamegraph_v0.svg"   # 途中で失敗しても既存の成果物を壊さない
echo "done: $HERE/01-aggregate/flamegraph_v0.svg"
