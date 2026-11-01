// 第6章 j0: 素朴。トランザクションごとにマスタを先頭から線形探索。O(N*M)
#include "join.h"
int main(int argc, char **argv){
    need_args(argc, argv);
    long M, N; Rec *mst = load_master(argv[1], &M); uint32_t *t = load_trans(argv[2], &N); lookup_begin();
    long hit = 0, miss = 0; unsigned long long sum = 0;
    for (long i = 0; i < N; i++) {
        long j; for (j = 0; j < M; j++) if (mst[j].key == t[i]) break;
        if (j < M) { hit++; sum += mst[j].value; } else miss++;
    }
    report(hit, sum, miss); return 0;
}
