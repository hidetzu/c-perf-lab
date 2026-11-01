#!/bin/bash
# すべてのプログラムをビルドする。各ディレクトリに実行ファイルができる。使い方: bash build_all.sh
set -e; cd "$(dirname "$0")"
gcc -O2 -Wall -o tools/runstat tools/runstat.c
(cd 00-harness && for f in sum_const sum_unused sum_argv sum_barrier ub_overflow; do gcc -O2 -o $f $f.c; done; gcc -O2 -fwrapv -o ub_overflow_wrapv ub_overflow.c)
(cd 01-aggregate && gcc -O2 -o gen gen.c && for v in v0_naive v1_nostrtok v1a_atoi v2_cents v2a_aligned v2c_checked v2d_nomem v0r_ref v3_block; do gcc -O2 -g -o ${v%%_*} $v.c; done)
(cd 01-aggregate && gcc -O2 -g -DSB=4096 -o v2b v2a_aligned.c)
(cd 02-io && for f in cnt_read cnt_stdio cnt_mmap out_lines copy_alias; do gcc -O2 -g -o $f $f.c; done)
(cd 03-string && for c in 0 1 2; do gcc -O2 -g -DCMP=$c -o sc_$c str_cmp.c; done)
(cd 04-join && gcc -O2 -o gen_join gen_join.c && for f in j0_linear j1_bsearch j1b_bsearch_branchless j2_hash j3_merge; do gcc -O2 -g -o ${f%%_*} $f.c; done)
(cd 05-cache && for f in matmul chase colsum stride; do gcc -O2 -g -o $f $f.c; done)
(cd 06-branch && gcc -O2 -g -o branch branch.c && gcc -O2 -g -fno-if-conversion -fno-if-conversion2 -fno-tree-loop-if-convert -fno-tree-vectorize -fno-tree-phiprop -o branch_nocmov branch.c)
(cd 07-wait && gcc -O2 -o w0 w0_fsync.c && gcc -O2 -o w1 w1_rpc.c && gcc -O2 -pthread -o w2 w2_lock.c && gcc -O2 -o w3 w3_tcp.c && gcc -O2 -pthread -o w4 w4_dns.c && gcc -O2 -o w5 w5_udp_dns.c)
echo "built. 実行前に: df -T . /tmp   (第9章の出力先は tmpfs でない実ディスクにすること)"
