#!/bin/bash
# 第9章: 待ちが支配するケース。事前に bash ../build_all.sh。出力先 (./w0.out) は実ディスク (ext4 など) 上に置くこと (df -T . で確認。tmpfs だと fsync が待たない)。
set -e; cd "$(dirname "$0")"; R=../tools/runstat
for b in w0 w1 w2 $R; do [ -x ./$b ] || [ -x $b ] || { echo "$b がない。bash ../build_all.sh を先に"; exit 1; }; done
echo "## fsync (2万件)"; for m in each batch once; do printf "fsync %-6s" $m; $R - 3 2 ./w0 $m ./w0.out; done; rm -f w0.out
echo "## 往復 500 件: 相手が直列(serial) / 往復遅延(latency)"
for d in serial latency; do for m in seq pipeline conc; do printf "rpc %-9s %-8s" $m $d; timeout 120 $R - 3 2 ./w1 $m 500 $d; done; done
echo "## ロック待ち (CPU 固定あり / なし)"
for cfg in "1 sleep" "4 sleep" "8 sleep" "4 spin" "8 spin"; do printf "lock T=%-2s %-6s pin " $cfg; $R - 3 2 ./w2 $cfg 200; printf "lock T=%-2s %-6s free" $cfg; $R - 3 - ./w2 $cfg 200; done
