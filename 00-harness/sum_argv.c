// 第1章 デモ2b: n を argv から取り、結果を printf で使う。-O2 でもループは消えず、2 要素ずつ進む形に変換される (0.216 ns/iter)。
// 結果を使えば消えない。n が定数 (sum_const.c) か、結果を使わない (sum_unused.c) と消える。
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(int argc, char **argv){
    long n = argc > 1 ? atol(argv[1]) : 1000000000L;
    double t0 = now();
    long s = 0;
    for (long i = 0; i < n; i++) s += i;
    double t1 = now();
    printf("sum=%ld  %.3f s  (%.3f ns/iter)\n", s, t1 - t0, (t1 - t0) / n * 1e9);
    return 0;
}
