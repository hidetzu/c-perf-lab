// 第6章 j1: マスタがキー昇順なので二分探索。O(N log M)
#include "join.h"
static Rec *mst; static long M;
static const Rec *find(uint32_t k){
    long lo = 0, hi = M;
    while (lo < hi) { long mid = lo + (hi - lo) / 2; if (mst[mid].key < k) lo = mid + 1; else hi = mid; }
    return lo < M && mst[lo].key == k ? &mst[lo] : NULL;
}
int main(int argc, char **argv){
    need_args(argc, argv);
    long N; mst = load_master(argv[1], &M); uint32_t *t = load_trans(argv[2], &N); lookup_begin();
    long hit = 0, miss = 0; unsigned long long sum = 0;
    for (long i = 0; i < N; i++) { const Rec *r = find(t[i]); if (r) { hit++; sum += r->value; } else miss++; }
    report(hit, sum, miss); return 0;
}
