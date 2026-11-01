// 第5章: 「文字列の照合が遅い」。長さ L の文字列 K 個の配列を先頭から比べ、最後の 1 個だけ一致する。
// 比較関数を変えて、1 回の比較あたりの時間 (ns) を測る。使い方: str_cmp <L> [K]
//   キーは末尾の 2 バイトだけが違う (比較が最後まで進む最悪ケース。K<=400 で全部違うキーになる)。
#include "bench.h"
#ifndef CMP
#define CMP 0
#endif
static int mycmp(const char *a, const char *b, size_t n){          // 自前の 1 バイトずつ
    for (size_t i = 0; i < n; i++) if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
    return 0;
}
int main(int argc, char **argv){
    #ifdef LEN
    const size_t L = LEN, K = 64;
#else
    size_t L = argc > 1 ? strtoul(argv[1],0,10) : 32, K = argc > 2 ? strtoul(argv[2],0,10) : 64;
#endif
    char *tab = aligned_alloc(64, K * (L+1));
#ifdef EARLY   /* 先頭 2 バイトで違うキー (実務で多い: 比較は 1 バイト目で終わる) */
    for (size_t k = 0; k < K; k++) { memset(tab + k*(L+1), 'a', L); tab[k*(L+1)] = 'a' + (k % 20); if (L >= 2) tab[k*(L+1) + 1] = 'a' + (k / 20); tab[k*(L+1) + L] = 0; }
#else          /* 末尾 2 バイトだけ違うキー (最悪ケース) */
    for (size_t k = 0; k < K; k++) { memset(tab + k*(L+1), 'a', L); tab[k*(L+1) + L-1] = 'a' + (k % 20); if (L >= 2) tab[k*(L+1) + L-2] = 'a' + (k / 20); tab[k*(L+1) + L] = 0; }
#endif
    char *key = malloc(L+1); memcpy(key, tab + (K-1)*(L+1), L+1);   // 一致するのは最後の 1 個
    long reps = 20000000 / (long)K; if (reps < 1000) reps = 1000;
    double best = 1e9; long found = -1;
    for (int trial = 0; trial < 5; trial++) {
        double t0 = now_s();
        for (long r = 0; r < reps; r++) {
            long idx = -1;
            for (size_t k = 0; k < K; k++) {
                const char *s = tab + k*(L+1); int c;
#if CMP == 0
                c = strcmp(s, key);
#elif CMP == 1
                c = memcmp(s, key, L);
#else
                c = mycmp(s, key, L);
#endif
                if (c == 0) { idx = k; break; }
            }
            found = idx; KEEP(found);
        }
        double t = now_s() - t0; if (t < best) best = t;
    }
    if (found != (long)K-1) { fprintf(stderr, "BUG: found=%ld\n", found); return 1; }
    printf("L=%zu K=%zu  %.2f ns/比較  (found=%ld)\n", L, K, best / ((double)reps * K) * 1e9, found);
    return 0;
}
