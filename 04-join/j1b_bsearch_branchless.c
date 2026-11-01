// 第6章 j1b: 二分探索を分岐なし (条件付き代入) で書いた版。分岐予測ミスの影響を切り分けるための対照。
#include "join.h"
int main(int argc, char **argv){
    need_args(argc, argv);
    long M, N; Rec *mst = load_master(argv[1], &M); uint32_t *t = load_trans(argv[2], &N); lookup_begin();
    long hit = 0, miss = 0; unsigned long long sum = 0;
    for (long i = 0; i < N; i++) {
        uint32_t k = t[i]; const Rec *base = mst; long n = M;
        while (n > 1) { long half = n / 2; base = (base[half].key <= k) ? base + half : base; n -= half; }   // 比較結果で base を選ぶ (cmov になる)
        if (n == 1 && base->key == k) { hit++; sum += base->value; } else miss++;
    }
    report(hit, sum, miss); return 0;
}
