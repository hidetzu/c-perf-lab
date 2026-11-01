#!/bin/bash
# AI が提案した改善版を、決めた基準で機械的に評価する (基準は本来人が決める。この実験では AI が人の役で決めた) (第3章の集計プログラム用)。AI の「速くなったはず」を信用しない。
# 使い方: bash eval.sh <候補.c> <基準の実行ファイル(仕様を固定した版: 01-aggregate/v0r)> <通常入力CSV(大)> [作業ディレクトリ]
# 基準 (採否):
#  1. 通常入力の出力が基準と完全一致 (cmp)
#  2. 異常系入力 (make_inputs.sh。固定の異常系 + 差分ファジング fuzz.csv) で、クラッシュせず、基準と同じ出力
#  3. ASan+UBSan 付きで異常系と小さい通常入力が通る
#  4. 速さ: runstat 9 回の中央値で基準比 10% 以上速い (ばらつきの判定は実装していない。9 回の中央値で足りるよう、閾値に余裕を持たせる)
# 「N 件で問題」は、make_inputs.sh が作る入力 (正常な normal_small とファジング入力を含む) の通常実行 + sanitizer 実行のうち、問題があった実行の数 (ファイル数ではない)。
# 終了コード: 0 = 全部通った / 1 = 1 つ以上落ちた
[ $# -ge 3 ] || { echo "usage: bash eval.sh <候補.c> <基準の実行ファイル> <通常入力CSV> [作業ディレクトリ]" >&2; exit 2; }
set -u; HERE=$(cd "$(dirname "$0")" && pwd); ROOT=$HERE/..
SRC=$(readlink -f "${1:?候補.c}"); REF=$(readlink -f "${2:?基準の実行ファイル}"); BIG=$(readlink -f "${3:?通常入力CSV}"); W0=${4:-$(mktemp -d)}; mkdir -p "$W0"; W=$(readlink -f "$W0")
[ -x "$REF" ] || { echo "基準の実行ファイルがない: $REF" >&2; exit 2; }; [ -x "$ROOT/tools/runstat" ] || { echo "tools/runstat がない。bash build_all.sh を先に" >&2; exit 2; }
rm -f "$W/cand" "$W/cand_san" "$W/build.log" "$W/san_build.log"   # 前の候補のバイナリで判定しない
fail=0; say(){ printf '%-34s %s\n' "$1" "$2"; }; bad(){ fail=1; say "$1" "FAIL: $2"; }
gcc -O2 -Wall -o "$W/cand" "$SRC" 2> "$W/build.log" || { cat "$W/build.log"; echo "ビルド失敗"; exit 1; }
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -o "$W/cand_san" "$SRC" 2> "$W/san_build.log" || bad "3. sanitizer ビルド" "失敗 ($W/san_build.log)"
rm -f "$W"/in/*.csv; bash "$HERE/make_inputs.sh" "$W/in" >/dev/null || { echo "make_inputs.sh が失敗" >&2; exit 2; }
# 1. 通常入力
"$REF" < "$BIG" > "$W/ref_big.txt" || { echo "基準が通常入力で失敗"; exit 2; }; timeout 120 "$W/cand" < "$BIG" > "$W/cand_big.txt" 2>/dev/null; rc=$?
if [ $rc -eq 0 ] && cmp -s "$W/ref_big.txt" "$W/cand_big.txt"; then say "1. 通常入力の出力一致" "PASS"; else bad "1. 通常入力の出力一致" "rc=$rc"; fi
# 2./3. 異常系
n=0; nf=0
for f in "$W"/in/*.csv; do n=$((n+1)); b=$(basename "$f")
  "$REF" < "$f" > "$W/r.txt" 2>/dev/null || { echo "    基準が $b でクラッシュ: 基準として不適格"; fail=1; continue; }
  timeout 10 "$W/cand" < "$f" > "$W/c.txt" 2>/dev/null; crc=$?
  if [ $crc -ne 0 ]; then nf=$((nf+1)); echo "    異常系 $b: 異常終了 (rc=$crc$( [ $crc -eq 124 ] && echo ", タイムアウト" ))"; elif ! cmp -s "$W/r.txt" "$W/c.txt"; then nf=$((nf+1)); echo "    異常系 $b: 出力が基準と違う"; fi
  # パイプで渡しても同じ出力か (stdin が通常ファイルであることに頼る候補を落とす)
  if [ $crc -eq 0 ]; then timeout 20 cat "$f" | timeout 10 "$W/cand" > "$W/p.txt" 2>/dev/null; prc=${PIPESTATUS[1]}; if [ $prc -ne 0 ] || ! cmp -s "$W/r.txt" "$W/p.txt"; then nf=$((nf+1)); echo "    異常系 $b: パイプ入力で異常終了または出力が違う (rc=$prc)"; fi; fi
  if [ -x "$W/cand_san" ]; then ASAN_OPTIONS=detect_leaks=0 timeout 20 "$W/cand_san" < "$f" >/dev/null 2>"$W/san.txt"; src=$?; if [ $src -ne 0 ]; then echo "    sanitizer $b: $( [ $src -eq 124 ] && echo タイムアウト || grep -m1 -E 'runtime error|ERROR' "$W/san.txt" )"; nf=$((nf+1)); fi; fi
done
if [ $nf -eq 0 ]; then say "2./3. 異常系 $n 件 + sanitizer" "PASS"; else bad "2./3. 異常系 $n 件 + sanitizer" "$nf 件で問題"; fi
# 4. 速さ (通常入力で落ちたら測らない)
med(){ timeout 300 "$ROOT/tools/runstat" "$BIG" 9 2 "$1" 2>&1 | sed -n 's/.*wall \([0-9]*\) ms.*/\1/p'; }
tr=$(med "$REF"); tc=$(med "$W/cand"); [ -n "$tr" ] && [ -n "$tc" ] || { bad "4. 速さ" "測定できなかった (候補が通常入力で失敗した可能性)"; echo "不採用 (人が理由を読む)"; exit 1; }
sp=$(awk -v r="$tr" -v c="$tc" 'BEGIN{printf "%.2f", r/c}')
say "4. 速さ (中央値 ms)" "基準 ${tr} / 候補 ${tc} (基準比 ${sp} 倍)"
awk -v s="$sp" 'BEGIN{exit !(s>=1.10)}' && say "   基準比 10% 以上速い" "PASS" || bad "   基準比 10% 以上速い" "${sp} 倍"
[ $fail -eq 0 ] && echo "採用可" || echo "不採用 (人が理由を読む)"; exit $fail
