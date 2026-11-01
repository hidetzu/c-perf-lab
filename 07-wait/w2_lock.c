// 第9章: ロック待ち。T 本のスレッドが同じミューテックスを取り合う。ロック中に 1ms 眠る (I/O の模擬) と、他は全員 futex で待つ。
// 使い方: w2_lock <スレッド数> <ロック中の処理: sleep|spin|none> [1スレッドあたりの回数]   (-pthread)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; static const char *work; static int iters; static long counter;
static void *th(void *a){ (void)a;
    for (int i = 0; i < iters; i++) {
        pthread_mutex_lock(&m);
        if (!strcmp(work, "sleep")) usleep(1000);
        else if (!strcmp(work, "spin")) { volatile long x = 0; for (int k = 0; k < 200000; k++) x += k; (void)x; }
        counter++; pthread_mutex_unlock(&m);
    } return 0; }
int main(int argc, char **argv){
    int T = argc > 1 ? atoi(argv[1]) : 4; if (T < 1 || T > 64) { fprintf(stderr, "threads 1..64\n"); return 2; } work = argc > 2 ? argv[2] : "sleep"; iters = argc > 3 ? atoi(argv[3]) : 200;
    if (strcmp(work, "sleep") && strcmp(work, "spin") && strcmp(work, "none")) { fprintf(stderr, "work: sleep|spin|none\n"); return 2; }
    pthread_t t[64]; for (int i = 0; i < T; i++) pthread_create(&t[i], 0, th, 0);
    for (int i = 0; i < T; i++) pthread_join(t[i], 0);
    printf("counter=%ld\n", counter); return 0;
}
