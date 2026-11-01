// 第1章 デモ4: asm volatile で s を「外から見られうる」ことにして畳み込み・除去を止める。
#include <stdio.h>
#include <time.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(void){
    const long n = 1000000000L;
    double t0 = now();
    long s = 0;
    for (long i = 0; i < n; i++) {
        s += i;
        __asm__ volatile("" : "+r"(s));
    }
    double t1 = now();
    printf("sum=%ld  %.3f s  (%.3f ns/iter)\n", s, t1 - t0, (t1 - t0) / n * 1e9);
    return 0;
}
