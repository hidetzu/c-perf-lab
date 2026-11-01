# 再現のコマンド一覧

本の各章の測定を、手元でたどるためのコマンドです。環境は、本の「はじめに」の環境表を参照してください。数値は環境で変わります。

すべてリポジトリのルートで実行する（スクリプトは自分でパスを絶対化するので、ルートからの相対パスで引数を渡してよい）。

```
bash build_all.sh                                        # 全プログラムをビルド (tools/runstat、各回の実行ファイル)
01-aggregate/gen 6000000 > sales6m.csv                   # 600万行 (約 197MB、固定シード)
head -n 600000 sales6m.csv > sales600k.csv               # 60万行 (第4章で使う)
bash 01-aggregate/bench.sh ./01-aggregate/v0 sales6m.csv # perf stat -r (第3章。実行ファイルは ./ 付き)
bash 02-io/sweep.sh sales600k.csv sales6m.csv out_dir    # 第4章 (out_dir は作られる。tmpfs でもよい)
bash 03-string/sweep.sh                                  # 第5章
bash 04-join/scaling.sh work_dir                         # 第6章 (work_dir に入力 CSV を作る。数分かかる)
bash 05-cache/sweep.sh                                   # 第7章 (chase は約 0.6GB のメモリを使う。約 30 秒)
bash 06-branch/sweep.sh                                  # 第8章
bash 07-wait/run.sh                                      # 第9章 (出力先は ext4 などの実ディスク。df -T . で確認)
```

`runstat` を直接使う例（第3章 など）: `tools/runstat sales6m.csv 9 2 ./01-aggregate/v0`（入力ファイル・回数・CPU・コマンド）。

付録（AI の案の評価。`appendix-ai/eval.sh` は候補をビルドして判定する）:

```
bash appendix-ai/eval.sh appendix-ai/ai_cand3.c 01-aggregate/v0r sales6m.csv work_dir   # 採用可なら終了コード 0
```

第7章の追実験（`05-cache/sweep.sh` に含まれないもの。`taskset -c 2` は CPU 固定）:

```
gcc -O2 -DTILE=32 -o matmul32 05-cache/matmul.c && taskset -c 2 ./matmul32 tile 2048   # タイル幅を変える (16/32/64/128)
taskset -c 2 05-cache/matmul tile 2048 2064               # 行の間隔 LD=2064 (パディング)。第 3 引数が LD (N 以上)
taskset -c 2 05-cache/matmul ijk 1000; NOTHP=1 taskset -c 2 05-cache/matmul ijk 1000   # THP あり/なし
taskset -c 2 05-cache/matmul tile 2112; taskset -c 2 05-cache/matmul tile 4160         # 2 のべきでない N (sweep.sh は 2048 まで)
NOTHP=1 taskset -c 2 05-cache/colsum 1024                 # colsum の THP なし
HUGE=1 taskset -c 2 05-cache/stride 65536 8               # stride を 2MB ページで
gcc -O2 -o copy_alias 02-io/copy_alias.c && taskset -c 2 ./copy_alias 0 16   # 第4章の位置ずれ (src と dst のページ内オフセット)
```

## 補足

- 測定の作法: `taskset -c 2` で CPU を固定し、複数回の中央値（または最小値）を見ます。この環境で同じ版を測り直したときのばらつきは ±0.1〜5% でした
- 高速版の一部（`01-aggregate` の v2/v3、`04-join` の j1/j3 など）は、受け付ける入力が元の版より狭いです。前提と、外れたときの挙動は、本の第3章と第6章の「入力の前提」を参照してください
