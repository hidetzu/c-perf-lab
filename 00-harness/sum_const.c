// 第1章 デモ1: n がコンパイル時定数 → 加算ループは閉じた式に畳み込まれる。
#include <stdio.h>
#include <time.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(void){
    const long n = 1000000000L;
    double t0 = now();
    long s = 0;
    for (long i = 0; i < n; i++) s += i;
    double t1 = now();
    printf("sum=%ld  %.6f s\n", s, t1 - t0);
    return 0;
}
