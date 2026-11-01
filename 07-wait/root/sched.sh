#!/bin/bash
# 第9章の待ちを perf sched / perf trace で見る。root が必要 (tracefs の権限)。使い方: sudo -v のあと bash sched.sh
set -e; ROOT=$(cd "$(dirname "$0")/../.." && pwd); W=$ROOT/07-wait; O=$(mktemp -d)
for b in "$W/w2" "$W/w1"; do [ -x "$b" ] || { echo "$b がない。bash build_all.sh を先に" >&2; exit 2; }; done
echo "## perf sched record: ロック待ち (w2 4 スレッド, ロック中に 1ms 眠る)"
sudo perf sched record -o $O/sched.data -- $W/w2 4 sleep 50 >/dev/null 2>&1
echo "-- perf sched latency (スレッドごとの遅延。max の大きい順)"; sudo perf sched latency -i $O/sched.data --sort max 2>/dev/null | head -12
echo "-- perf sched timehist (w2 の行だけ先頭 16 行。wait time = 眠っていた時間、sch delay = 実行可能になってから CPU を得るまでの遅れ、run time)"; sudo perf sched timehist -i $O/sched.data 2>/dev/null | grep -E "time +cpu|w2\[" | head -17
echo "## perf trace -s: システムコールごとの時間 (w1 seq 200 serial)"
sudo perf trace -s -- $W/w1 seq 200 serial 2>&1 | grep -E "events|clock_nanosleep|read |write |syscall  " | head -12
sudo rm -rf "$O"
