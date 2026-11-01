# C プログラムの遅さを調べる実験コードと実測ノート

「遅い」と言われた C プログラムを、**測る → 仮説 → 実験 → 改善 → 再測定** の順で調べるための題材プログラムと、その実測記録です。Zenn Book『測ってから直す — Cプログラムのボトルネックを追う実験ノート』の追実験用コードを兼ねています。

- 題材プログラムはすべて自作です
- 数値は **桁と傾向** を見るためのものです。環境（CPU、gcc、カーネル、日付）を必ず付けて読んでください。**自分の環境では出ない現象もあります**（出ないこと自体が実験の結果です）

## 動作環境

Linux x86-64、gcc、perf、strace。測定は Arch Linux（kernel 7.2、gcc 16.2、perf 7.3、AMD Ryzen 7 5700X）で行いました。root がなくても測れるものがほとんどです。root が必要なもの（`drop_caches`、`tc netem`、`perf sched`）は `07-wait/root/` にまとめてあります。

## 構成

| Book の章 | ディレクトリ | 症状 | 主な道具 |
|---|---|---|---|
| 第1章 | `00-harness` | 最適化したのに速さが変わらない／結果が変わる（計測そのものが壊れる） | `objdump -d` |
| 第2章 | `tools` | 計測の道具（`runstat`: wall/user/sys の中央値）、`perf`/`strace`/`time` の出力サンプル | |
| 第3章 | `01-aggregate` | 集計バッチが遅い（`fgets`/`strtok`/`atof` から、入力検査つきの高速版まで） | `perf stat`、`perf record`、flamegraph、`perf annotate` |
| 第4章 | `02-io` | ファイルを読むだけで遅い／結果の書き出しが遅い（バッファの大きさと位置） | `strace`、`/proc/self/io` |
| 第5章 | `03-string` | 文字列の照合が遅い（`strcmp`/`memcmp`、先頭で違う場合と最後まで比べる場合） | `perf annotate`、`objdump` |
| 第6章 | `04-join` | 件数が増えたら急に遅い（線形探索 → 二分探索 → ハッシュ → マージ結合） | 入力を倍々にして測る |
| 第7章 | `05-cache` | サイズを少し変えたら急に遅い（行列積、キャッシュの階層と連想度） | `perf stat`（L1/L2 ミス） |
| 第8章 | `06-branch` | データによって速さが変わる（分岐予測、`cmov`） | `perf stat`（`branch-misses`） |
| 第9章 | `07-wait` | CPU は空いているのに遅い（`fsync`、往復遅延、ロック、名前解決） | `strace -f -c -w`、`perf sched`、`perf trace` |
| 付録 | `appendix-ai` | AI が出した改善案を、決めた基準で機械的に評価する（評価ハーネスと、3 周の実ループの記録。基準の決定はこの実験では AI が演じた） | `eval.sh`、差分ファジング |

## 動かし方

リポジトリのルートで実行します（スクリプトは自分でパスを絶対化するので、相対パスで引数を渡せます）。

```
bash build_all.sh                                        # 全プログラムをビルド（tools/runstat を含む）
01-aggregate/gen 6000000 > sales6m.csv                   # 600万行の入力（約 197MB、固定シード）
head -n 600000 sales6m.csv > sales600k.csv               # 60万行（第4章で使う）
tools/runstat sales6m.csv 9 2 ./01-aggregate/v0          # 入力・回数・CPU・コマンド。wall/user/sys の中央値
bash 02-io/sweep.sh sales600k.csv sales6m.csv out_dir    # 第4章
bash 04-join/scaling.sh work_dir                         # 第6章（数分かかる）
bash 05-cache/sweep.sh                                   # 第7章
bash 06-branch/sweep.sh                                  # 第8章
bash 07-wait/run.sh                                      # 第9章（出力先は ext4 などの実ディスク。tmpfs だと fsync が待たない）
```

詳しい手順（第7章の LD/NOTHP/HUGE/TILE の追実験、付録の評価）は [`docs/reproduce.md`](docs/reproduce.md) にあります。

## 注意

- **入力の前提**: 高速版の一部（`01-aggregate` の v2/v3、`04-join` の j1/j3 など）は、受け付ける入力が元の版より狭いです。前提と、外れたときの挙動は、本の第3章と第6章の「入力の前提」に書いています
- **測定の作法**: `taskset -c 2` で CPU を固定し、複数回の中央値（または最小値）を見ます。governor は `powersave` のままでも ±数% に収まりましたが、初回の実行は周波数が上がりきらず遅いことがあります。ページキャッシュに載った状態で測っています（ディスクからの読みは `07-wait/root/` の cold の測定）
- `tools/make_flamegraph.sh` は外部のスクリプト（FlameGraph）を使いますが、リポジトリには同梱していません。`01-aggregate/flamegraph_v0.svg` は、brendangregg/FlameGraph（CDDL 1.0、commit `41fee1f`）のスクリプトで生成した成果物です。中のコメントの手順で取得し、**内容を確認してから**実行してください
- `appendix-ai/` の `ai_cand*.c` は、AI（Claude のエージェント）が書いたコードです。評価の基準と入力の追加は、この実験では、実験を進めていた AI が決めました（本来は人が決める部分です）。評価の対象として載せており、仕様の取り決めと限界は `appendix-ai/loop/RECORD.md` に書いています

## ライセンス

MIT（[`LICENSE`](LICENSE)）。AI が書いたコードも同じライセンスです。
