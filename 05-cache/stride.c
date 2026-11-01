// 第7章: 連想度 (キャッシュの「棚の数」) の実測。R 個の要素を stride バイト間隔で置いた輪をたどる (ポインタ追跡)。
// 同じセットに入る要素が連想度 (ウェイ数) を超えると、追い出し合って遅くなる。使い方: stride <stride バイト> <R>
#include "bench.h"
#include <sys/mman.h>
int main(int argc, char **argv){
    size_t stride = argc > 1 ? strtoul(argv[1],0,10) : 4096, R = argc > 2 ? strtoul(argv[2],0,10) : 16;
    if (stride % 8 || R < 1) { fprintf(stderr, "stride は 8 の倍数、R は 1 以上\n"); return 2; }
    size_t sz = (stride * R + 4096 + (2<<20) - 1) & ~((size_t)(2<<20) - 1);       // 2MB の倍数
    char *mem = aligned_alloc(2<<20, sz);
    madvise(mem, sz, getenv("HUGE") ? MADV_HUGEPAGE : MADV_NOHUGEPAGE);        // HUGE=1: 物理的に連続しやすい 2MB ページ。既定は 4KB ページ
    for (size_t i = 0; i < R; i++) *(size_t*)(mem + i*stride) = ((i + 1) % R) * stride;     // 輪: 次の要素のオフセット
    size_t x = 0; long steps = 50000000; double best = 1e9;
    for (int t = 0; t < 3; t++) { double t0 = now_s(); for (long s = 0; s < steps; s++) x = *(size_t*)(mem + x); double d = now_s() - t0; if (d < best) best = d; }
    KEEP(x); printf("stride=%-6zu R=%-4zu  %.2f ns/回\n", stride, R, best / steps * 1e9);
    return 0;
}
