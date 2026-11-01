// 第1章 デモ2: 結果を使わない → ループごと消える。
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
    (void)s;
    printf("%.6f s\n", t1 - t0);
    return 0;
}
