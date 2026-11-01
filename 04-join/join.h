// 第6章 共通: ファイル読み込みと出力。突合そのもの (lookup) 以外は全版で同じコードを使い、比較を公平にする。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
static double g_t0, g_build;
static double now_s(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }
static void lookup_begin(void){ g_t0 = now_s(); }   // ファイル読み込みの後、突合の準備と検索だけを測る
typedef struct { uint32_t key; uint32_t value; } Rec;
// 引数の検査は別関数に出す。main の先頭に直接書くと、GCC 16 では j0 の探索ループの配置が変わり 2 倍遅くなった (本の第6章を参照)
__attribute__((noinline)) static void need_args(int argc, char **argv){ if (argc < 3) { fprintf(stderr, "usage: %s master.csv trans.csv\n", argv[0]); exit(2); } }
static Rec *load_master(const char *path, long *n){
    FILE *f = fopen(path, "r"); if (!f) { perror(path); exit(1); }
    long cap = 1024, m = 0; Rec *a = malloc(cap * sizeof *a); unsigned k, v;
    while (fscanf(f, "%u,%u\n", &k, &v) == 2) { if (m == cap) a = realloc(a, (cap *= 2) * sizeof *a); a[m].key = k; a[m].value = v; m++; }
    fclose(f); *n = m; return a;
}
static uint32_t *load_trans(const char *path, long *n){
    FILE *f = fopen(path, "r"); if (!f) { perror(path); exit(1); }
    long cap = 1024, m = 0; uint32_t *a = malloc(cap * sizeof *a); unsigned k;
    while (fscanf(f, "%u\n", &k) == 1) { if (m == cap) a = realloc(a, (cap *= 2) * sizeof *a); a[m++] = k; }
    fclose(f); *n = m; return a;
}
// 結果: 一致した件数・value の合計・一致しなかった件数 (全版で同じになるはず)
static void report(long hit, unsigned long long sum, long miss){ { double l = (now_s()-g_t0)*1e3; fprintf(stderr, "lookup=%.1f ms\n", l); if (g_build > 0) fprintf(stderr, "total=%.1f ms\n", g_build + l); } printf("hit=%ld sum=%llu miss=%ld\n", hit, sum, miss); }
