#!/bin/bash
# root が必要な測定をまとめて実行し、結果を 07-wait/root/out/ に保存する。使い方 (自分の端末で):
#   (リポジトリのルートで) bash build_all.sh && sudo -v && bash 07-wait/root/run_all.sh <6M行のCSV>
set -e; cd "$(dirname "$0")"; mkdir -p out; sudo -v
bash cold_read.sh "${1:?6M 行の CSV を指定}" 2>&1 | tee out/cold_read.txt
bash netem.sh 2 2>&1 | tee out/netem.txt
bash sched.sh 2>&1 | tee out/sched.txt
echo "done: $(pwd)/out/"
