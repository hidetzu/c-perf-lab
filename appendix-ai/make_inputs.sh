#!/bin/bash
# 第3章の集計プログラム用の異常系入力。AI が出した「速い版」が、元の版の暗黙の前提を壊していないかを見る。
# 使い方: bash make_inputs.sh <出力ディレクトリ>
set -e; D=${1:?出力ディレクトリ}; mkdir -p "$D"; ok='2026-01-01,S0001,P0001,1,1.00'
printf '%s\n%s\n' "$ok" '2026-01-02,S0002,P0002,2,2.50' > "$D/normal_small.csv"
printf '%s' "$ok" > "$D/no_newline_one_line.csv"
printf '%s\n%s' "$ok" '2026-01-02,S0002,P0002,2,2.50' > "$D/no_newline_last.csv"
printf '%s\n\n%s\n' "$ok" '2026-01-02,S0002,P0002,2,2.50' > "$D/blank_line.csv"
printf 'date,store,product,qty,amount\n%s\n' "$ok" > "$D/header.csv"
printf '%s\r\n%s\r\n' "$ok" '2026-01-02,S0002,P0002,2,2.50' > "$D/crlf.csv"
printf '%s\n%s\n' "$ok" '2026-01-02,S0002,P0002,2,12' > "$D/amount_no_decimal.csv"
printf '%s\n%s\n' "$ok" '2026-01-02,S0002,P0002,2,2.5' > "$D/amount_one_decimal.csv"
printf '%s\n%s\n' "$ok" '2026-01-02,S0002,P0002,2,-2.50' > "$D/amount_negative.csv"
printf '%s\n%s\n' "$ok" '2026-01-02,S1000,P0002,2,2.50' > "$D/store_out_of_range.csv"
printf '%s\n%s\n' "$ok" '2026-01-02,S0002,P0002,2,2.50,extra' > "$D/extra_field.csv"
{ printf '2026-01-01,S0001,'; head -c 5000 /dev/zero | tr '\0' P; printf ',1,1.00\n%s\n' "$ok"; } > "$D/long_line.csv"
# AI が自己申告した「1MiB 超の行はエラー終了」を入力として追加 (2回目のループで追加した)
{ printf '2026-01-01,S0001,'; head -c 2000000 /dev/zero | tr '\0' P; printf ',1,1.00\n%s\n' "$ok"; } > "$D/huge_line.csv"
# 3 周目の査読 (別の AI による) で見つかった、AI 案が落ちる入力を追加 (3 周目のループ)
printf 'a,\n%s\n' "$ok" > "$D/comma_at_line_end.csv"
printf '%s\na,' "$ok" > "$D/comma_at_eof_no_newline.csv"
printf '%s\n%s\n' "$ok" '2026-01-02,S99999999999,P0002,2,2.50' > "$D/store_11_digits.csv"
printf '%s\n%s\n%s\n' "$ok" '2026-01-02,S0002,,2,2.50' ',S0003,P0003,3,3.50' > "$D/empty_fields.csv"
printf '%s\n%s\n' "$ok" '2026-01-02,S-2,P0002,2,2.50' > "$D/store_negative.csv"
printf '%s\n%s\n' "$ok" '2026-01-02,S0002,P0002,2,99999999999999.99' > "$D/amount_14_digits.csv"
# ミューテーション検査で、ハーネスが見逃した壊し方の入力を追加
printf '%s\n' "$ok" > "$D/nul_in_field.csv"; printf 'd\0x,S0001,P,1,2.00\n2026-01-02,S0002,P,1,3.00\n' >> "$D/nul_in_field.csv"
printf '2026-01-01,S0003,P,1,4.00\0junk\n%s\n' "$ok" > "$D/nul_in_amount.csv"
{ printf '2026-01-01,S0001,'; head -c 2000000 /dev/zero | tr '\0' P; printf ',1,1.00\n'; i=0; while [ $i -lt 3000 ]; do printf '2026-01-02,S%04d,P0002,2,2.50\n' $((i % 1000)); i=$((i+1)); done; } > "$D/huge_line_then_many.csv"
for n in 1048570 1048575 1048576 1048577 1048582; do { printf '2026-01-01,S0004,'; head -c $((n - 40)) /dev/zero | tr '\0' P; printf ',1,1.00\n%s\n' "$ok"; } > "$D/line_$n.csv"; done
# 差分ファジング用の入力 (固定シード。python3 が必要)
python3 "$(dirname "$0")/fuzz_gen.py" 20000 > "$D/fuzz.csv" || { echo "fuzz_gen.py が失敗 (python3 が必要)" >&2; exit 1; }
