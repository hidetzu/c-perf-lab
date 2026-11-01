#!/bin/bash
# 第4章の「キャッシュなしのディスク読み」。root が必要 (drop_caches)。使い方: sudo -v のあと bash cold_read.sh <大きなCSV>
# 実行ごとにページキャッシュを捨ててから read する (cold) 場合と、直前に読んだ (warm) 場合を比べる。
set -e; F=$(readlink -f "${1:?大きな CSV (6M 行) を指定}"); ROOT=$(cd "$(dirname "$0")/../.." && pwd)
R=$ROOT/tools/runstat; CR=$ROOT/02-io/cnt_read; CM=$ROOT/02-io/cnt_mmap
for b in "$R" "$CR" "$CM"; do [ -x "$b" ] || { echo "$b がない。bash build_all.sh を先に" >&2; exit 2; }; done
drop(){ sudo sh -c 'sync; echo 3 > /proc/sys/vm/drop_caches'; }
FS=$(df -T "$F" | tail -1 | awk '{print $2}')
[ "$FS" != "tmpfs" ] && [ "$FS" != "ramfs" ] || { echo "$F は $FS 上にある。drop_caches は tmpfs のページを捨てられないので、ext4 などのディスク上のファイルを指定する (例: ~/ 配下にコピー)" >&2; exit 2; }
echo "## $(df -T "$F" | tail -1 | awk '{print "ファイルシステム:", $2, " デバイス:", $1}')   ファイル: $(stat -c %s "$F") バイト"
for prog in "cnt_read 65536" "cnt_read 4096" "cnt_read 1048576" "cnt_mmap"; do
  set -- $prog; exe=$ROOT/02-io/$1; shift
  printf "%-18s cold:" "$prog"; for i in 1 2 3 4 5; do drop; $R "$F" 1 2 $exe "$@" | sed 's/.*wall \([0-9]*\) ms  user \([0-9]*\) ms  sys \([0-9]*\) ms.*/ \1(u\2,s\3)/' | tr -d '\n'; done; echo
  printf "%-18s warm:" "$prog"; $R "$F" 1 2 $exe "$@" >/dev/null; for i in 1 2 3 4 5; do $R "$F" 1 2 $exe "$@" | sed 's/.*wall \([0-9]*\) ms  user \([0-9]*\) ms  sys \([0-9]*\) ms.*/ \1(u\2,s\3)/' | tr -d '\n'; done; echo
done
echo "(wall ms、括弧内は user/sys ms。cold = 毎回 drop_caches 直後。先読みの設定は readahead_kb: $(cat /sys/block/$(basename "$(readlink -f /sys/class/block/$(lsblk -no pkname "$(df --output=source "$F" | tail -1)" 2>/dev/null | head -1) 2>/dev/null)" 2>/dev/null)/queue/read_ahead_kb 2>/dev/null || echo 不明))"
