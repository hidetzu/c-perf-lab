// 第6章 入力生成: マスタ M 件 (キー昇順) とトランザクション N 件。固定シード。
// 使い方: gen_join <M> <N> <master出力> <trans出力> [sorted]
//  master: "key,value" (key は 8 桁の昇順の数、間隔はランダム)
//  trans : "key" 1 列。9 割はマスタにある key、1 割は無い key。sorted を付けるとキー昇順に並べる。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
static uint64_t st = 88172645463325252ULL;
static uint64_t rnd(void){ st ^= st << 13; st ^= st >> 7; st ^= st << 17; return st; }
static int cmpu(const void *a, const void *b){ uint32_t x=*(uint32_t*)a, y=*(uint32_t*)b; return (x>y)-(x<y); }
int main(int argc, char **argv){
    if (argc < 5) return 2;
    long M = atol(argv[1]), N = atol(argv[2]); int sorted = argc > 5 && !strcmp(argv[5], "sorted");
    uint32_t *key = malloc(M * sizeof *key); uint32_t k = 10000000;
    if (M < 1 || N < 1) { fprintf(stderr, "M, N >= 1\n"); return 2; }
    FILE *fm = fopen(argv[3], "w"); if (!fm) { perror(argv[3]); return 1; }
    for (long i = 0; i < M; i++) { k += 2 + rnd() % 20; key[i] = k; fprintf(fm, "%08u,%u\n", k, (unsigned)(rnd() % 100000)); }
    fclose(fm);
    uint32_t *t = malloc(N * sizeof *t);
    for (long i = 0; i < N; i++) t[i] = (rnd() % 10) ? key[rnd() % M] : key[rnd() % M] + 1;   // +1 は必ず無いキー (間隔が 2 以上)
    if (sorted) qsort(t, N, sizeof *t, cmpu);
    FILE *ft = fopen(argv[4], "w"); if (!ft) { perror(argv[4]); return 1; }
    for (long i = 0; i < N; i++) fprintf(ft, "%08u\n", t[i]);
    fclose(ft); return 0;
}
