// 第7章: 作業領域のサイズを変えてメモリアクセス 1 回あたりの時間を測る (ポインタ追跡)。
// ランダムな 1 周の巡回順列をたどる＝次のアドレスが直前の読み出しに依存し、先読みも並列化も効かない。
#include "bench.h"
static uint64_t st = 88172645463325252ULL;
static uint64_t rnd(void){ st ^= st << 13; st ^= st >> 7; st ^= st << 17; return st; }
int main(void){
    for (size_t kb = 4; kb <= 512*1024; kb *= 2) {
        size_t n = kb * 1024 / sizeof(size_t) ; size_t *p = malloc(n * sizeof *p);
        // 64 バイト (キャッシュ行) 単位に 1 要素だけ使う: 要素間隔 8 個 (=64B)
        size_t m = n / 8; size_t *perm = malloc(m * sizeof *perm);
        for (size_t i = 0; i < m; i++) perm[i] = i;
        for (size_t i = m-1; i > 0; i--) { size_t j = rnd() % (i+1); size_t t = perm[i]; perm[i] = perm[j]; perm[j] = t; }
        for (size_t i = 0; i < m; i++) p[perm[i]*8] = perm[(i+1) % m]*8;
        long steps = 20000000; size_t x = perm[0]*8; double best = 1e9;
        for (int trial = 0; trial < 3; trial++) { double t0 = now_s(); for (long s = 0; s < steps; s++) x = p[x]; double t = now_s() - t0; if (t < best) best = t; }
        KEEP(x); printf("%8zu KB  %6.2f ns/回\n", kb, best / steps * 1e9);
        free(p); free(perm);
    }
    return 0;
}
