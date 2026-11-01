// 第6章 j3: トランザクションもキー昇順の入力に限定したマージ結合。O(N+M)。並んでいる前提が崩れると結果が壊れる
#include "join.h"
int main(int argc, char **argv){
    need_args(argc, argv);
    long M, N; Rec *mst = load_master(argv[1], &M); uint32_t *t = load_trans(argv[2], &N); lookup_begin();
    long hit = 0, miss = 0, j = 0; unsigned long long sum = 0;
    for (long i = 0; i < N; i++) {
        while (j < M && mst[j].key < t[i]) j++;
        if (j < M && mst[j].key == t[i]) { hit++; sum += mst[j].value; } else miss++;
    }
    report(hit, sum, miss); return 0;
}
