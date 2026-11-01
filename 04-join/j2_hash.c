// 第6章 j2: ハッシュ表 (オープンアドレス法)。平均 O(N)
#include "join.h"
int main(int argc, char **argv){
    need_args(argc, argv);
    long M, N; Rec *mst = load_master(argv[1], &M); uint32_t *t = load_trans(argv[2], &N); lookup_begin();
    double tb = now_s();                               // 構築 (calloc と挿入) を別に測る
    long cap = 1; while (cap < M * 2) cap <<= 1;
    Rec *tab = calloc(cap, sizeof *tab);                       // key 0 = 空き (キーは 0 にならない)
    for (long j = 0; j < M; j++) { uint32_t h = (mst[j].key * 2654435761u) >> 7; long p = h & (cap - 1); while (tab[p].key) p = (p + 1) & (cap - 1); tab[p] = mst[j]; }
    double bms = (now_s() - tb) * 1e3; fprintf(stderr, "build=%.1f ms\n", bms); g_build = bms; lookup_begin();   // ここから検索だけ
    long hit = 0, miss = 0; unsigned long long sum = 0;
    for (long i = 0; i < N; i++) {
        uint32_t k = t[i], h = (k * 2654435761u) >> 7; long p = h & (cap - 1);
        while (tab[p].key && tab[p].key != k) p = (p + 1) & (cap - 1);
        if (tab[p].key) { hit++; sum += tab[p].value; } else miss++;
    }
    report(hit, sum, miss); return 0;
}
