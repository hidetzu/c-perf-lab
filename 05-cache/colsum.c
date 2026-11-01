// 第7章: 2 次元配列を「列方向」に足す。行の長さ (ストライド) が 2 のべきだとキャッシュの同じセットに集中する。
// 使い方: colsum <行の長さ(int 個)>  行数は 4096 固定。列ごとに全行を足す。
#include "bench.h"
#include <sys/mman.h>
int main(int argc, char **argv){
    long W = argc > 1 ? atol(argv[1]) : 1024, R = 4096, COLS = 64;   // COLS 列だけ足す (全列だと長いので)
    int *a = aligned_alloc(4096, (size_t)W*R*4 + 4096); if (getenv("NOTHP")) madvise((void*)((uintptr_t)a & ~4095UL), ((size_t)W*R*4 + 4095) & ~4095UL, MADV_NOHUGEPAGE);   // NOTHP=1: 透過ヒュージページを切る
    for (long i = 0; i < W*R; i++) a[i] = i & 15;
    double best = 1e9; long sum = 0;
    for (int trial = 0; trial < 5; trial++) {
        double t0 = now_s(); sum = 0;
        for (int rep = 0; rep < 200; rep++) for (long c = 0; c < COLS; c++) for (long r = 0; r < R; r++) sum += a[r*W + c];
        double t = now_s() - t0; if (t < best) best = t;
    }
    KEEP(sum); printf("W=%-5ld (行の幅 %ld B)  %.3f s  (%.2f ns/要素)\n", W, W*4, best, best/(200.0*COLS*R)*1e9);
    return 0;
}
