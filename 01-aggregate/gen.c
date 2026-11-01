// 第3章 入力生成: date,store,product,qty,amount を n 行。固定シードで再現可能。
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
static uint64_t st = 88172645463325252ULL;
static uint64_t rnd(void){ st ^= st << 13; st ^= st >> 7; st ^= st << 17; return st; }
int main(int argc, char **argv){
    long n = argc > 1 ? atol(argv[1]) : 6000000;
    for (long i = 0; i < n; i++) {
        unsigned m = rnd() % 12 + 1, d = rnd() % 28 + 1, s = rnd() % 1000, p = rnd() % 5000, q = rnd() % 9 + 1;
        unsigned long c = rnd() % 999999 + 1;   // 0.01 .. 9999.99
        printf("2026-%02u-%02u,S%04u,P%04u,%u,%lu.%02lu\n", m, d, s, p, q, c / 100, c % 100);
    }
    return 0;
}
