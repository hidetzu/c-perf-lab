// 第7章: 行列積のループ順。C = A * B (N x N, int)。使い方: matmul <ijk|ikj|tile> [N] [LD]   (LD = 行の間隔。既定 N。N+16 などにすると行の間隔を 2 のべきから外せる)
#include "bench.h"
#include <sys/mman.h>
#ifndef TILE
#define TILE 64
#endif
int main(int argc, char **argv){
    const char *mode = argc > 1 ? argv[1] : "ijk"; int N = argc > 2 ? atoi(argv[2]) : 1000; int LD = argc > 3 ? atoi(argv[3]) : N; if (LD < N) { fprintf(stderr, "LD must be >= N\n"); return 2; }   // LD: 行の間隔 (要素数)。N より大きくするとパディングになる
    int *A = malloc((size_t)N*LD*4), *B = malloc((size_t)N*LD*4), *C = calloc((size_t)N*LD, 4);
    if (getenv("NOTHP")) { size_t sz = ((size_t)N*LD*4 + 4095) & ~4095UL; madvise((void*)((uintptr_t)A & ~4095UL), sz, MADV_NOHUGEPAGE); madvise((void*)((uintptr_t)B & ~4095UL), sz, MADV_NOHUGEPAGE); madvise((void*)((uintptr_t)C & ~4095UL), sz, MADV_NOHUGEPAGE); }   // NOTHP=1: 透過ヒュージページを切る
    for (long i = 0; i < (long)N*LD; i++) { A[i] = i % 7; B[i] = i % 11; }
    double best = 1e9;
    for (int trial = 0; trial < 3; trial++) {
        memset(C, 0, (size_t)N*LD*4); double t0 = now_s();
        if (!strcmp(mode, "ijk")) {
            for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) { int s = 0; for (int k = 0; k < N; k++) s += A[i*LD+k] * B[k*LD+j]; C[i*LD+j] = s; }
        } else if (!strcmp(mode, "ikj")) {
            for (int i = 0; i < N; i++) for (int k = 0; k < N; k++) { int a = A[i*LD+k]; for (int j = 0; j < N; j++) C[i*LD+j] += a * B[k*LD+j]; }
        } else {                                                   // tile: 64x64 のブロックに分ける (ikj の順)。N は 64 の倍数に限る
            const int T = TILE; if (N % TILE) { fprintf(stderr, "tile needs N %% TILE == 0\n"); return 1; }
            for (int ii = 0; ii < N; ii += T) for (int kk = 0; kk < N; kk += T) for (int jj = 0; jj < N; jj += T)
                for (int i = ii; i < ii+T; i++) for (int k = kk; k < kk+T; k++) { int a = A[i*LD+k];
                    for (int j = jj; j < jj+T; j++) C[i*LD+j] += a * B[k*LD+j]; }
        }
        double t = now_s() - t0; if (t < best) best = t;
    }
    long chk = 0; for (long i = 0; i < (long)N*LD; i += 997) chk += C[i];
    printf("%-4s N=%d  %.3f s  (check=%ld)\n", mode, N, best, chk);
    return 0;
}
