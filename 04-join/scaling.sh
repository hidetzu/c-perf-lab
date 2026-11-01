#!/bin/bash
# 第6章: 件数を倍々にして時間の増え方を見る。使い方: bash 04-join/scaling.sh <作業ディレクトリ>   (事前に bash build_all.sh。j2 は「構築込み」と「検索のみ」の両方を出す)
[ $# -ge 1 ] || { echo "usage: bash 04-join/scaling.sh <作業ディレクトリ>" >&2; exit 2; }
set -e; D=$(readlink -f "$1"); mkdir -p "$D"; cd "$(dirname "$0")"
run(){ ./$1 $D/m.csv $D/t.csv 2>&1 >/dev/null | grep lookup= | sed 's/lookup=//'; }
echo "## j0 (線形探索): N=50000 固定、M を倍々"
for M in 5000 10000 20000 40000 80000; do ./gen_join $M 50000 $D/m.csv $D/t.csv; printf "M=%-7d" $M; run j0; done
echo "## j0: M=20000 固定、N を倍々"
for N in 25000 50000 100000 200000; do ./gen_join 20000 $N $D/m.csv $D/t.csv; printf "N=%-7d" $N; run j0; done
echo "## 二分探索・分岐なし二分探索・ハッシュ・マージ (N=2,000,000、検索フェーズの ms。5 回の最小 / 中央値)"
best(){ key=${2:-lookup}; for i in 1 2 3 4 5; do ./$1 $D/m.csv $D/t.csv 2>&1 >/dev/null | grep "^$key=" | sed "s/$key=//; s/ ms//"; done | sort -n | awk '{a[NR]=$1} END{printf "%.1f/%.1f", a[1], a[int((NR+1)/2)]}'; }
for M in 10000 100000 1000000 4000000; do for ord in random sorted; do
  ./gen_join $M 2000000 $D/m.csv $D/t.csv $([ $ord = sorted ] && echo sorted)
  printf "M=%-8d %-7s j1=%s j1b=%s j2(構築込み)=%s j2(検索のみ)=%s" $M $ord "$(best j1)" "$(best j1b)" "$(best j2 total)" "$(best j2 lookup)"; [ $ord = sorted ] && printf " j3=%s" "$(best j3)"; echo
done; done
