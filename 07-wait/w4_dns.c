// 第9章: 名前解決 (getaddrinfo) の待ち。使い方: w4_dns <名前> <回数> <スレッド数>
//   名前の例: localhost (/etc/hosts で解決。ネットワークに出ない) / x.invalid (存在しない名前。リゾルバが NXDOMAIN を返す)
//   スレッド数 1: 直列、8: 8 スレッドで分担 (待ちが重なる)。出力: 全体の時間と 1 回あたりの時間。
//   リゾルバが 127.0.0.53 (systemd-resolved のスタブ) の環境では、lo に tc netem で遅延を入れると名前解決が遅くなる (root が必要)。
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>
static const char *name; static int per; static int T;
static double now_s(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }
static void *th(void *a){ (void)a; for (int i = 0; i < per; i++) { struct addrinfo *r = NULL, h = { .ai_socktype = SOCK_STREAM }; if (getaddrinfo(name, NULL, &h, &r) == 0) freeaddrinfo(r); } return 0; }
int main(int argc, char **argv){
    name = argc > 1 ? argv[1] : "localhost"; int n = argc > 2 ? atoi(argv[2]) : 100; T = argc > 3 ? atoi(argv[3]) : 1;
    if (T < 1 || T > 64 || n < T) { fprintf(stderr, "threads 1..64, n >= threads\n"); return 2; }
    per = n / T; pthread_t t[64]; double t0 = now_s();
    for (int i = 0; i < T; i++) pthread_create(&t[i], 0, th, 0);
    for (int i = 0; i < T; i++) pthread_join(t[i], 0);
    double d = now_s() - t0; printf("%-14s n=%d threads=%d  全体 %.1f ms  (1 回あたり %.2f ms)\n", name, per * T, T, d * 1e3, d * 1e3 / (per * T));
    return 0;
}
