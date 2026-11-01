// 第8章: 条件分岐。配列の要素が閾値 P 未満なら加算する。P=「分岐が成立する確率(%)」。
// 使い方: branch <branchy|branchless|sorted> <P>
//   branchy    : if (x < P) sum += x;  と書く (asm で、コンパイラによる畳み込みと消去を止める。cmov にするかどうかはコンパイラ次第)
//   branchless : sum += (x < P) ? x : 0 を、マスク演算で書く
//   sorted     : branchy と同じだが入力を昇順に並べ替える (分岐は前半が全部成立、後半が全部不成立で、予測しやすい)
#include "bench.h"
#define N (1<<24)
static uint64_t st = 88172645463325252ULL;
static uint64_t rnd(void){ st ^= st << 13; st ^= st >> 7; st ^= st << 17; return st; }
__attribute__((noinline)) static long run_branchy(const uint8_t *a, int P){
    long sum = 0;
    for (long i = 0; i < N; i++) { uint8_t x = a[i]; __asm__ volatile("" : "+r"(x)); if (x < P) sum += x; }
    return sum;
}
__attribute__((noinline)) static long run_branchless(const uint8_t *a, int P){
    long sum = 0;
    for (long i = 0; i < N; i++) { uint8_t x = a[i]; long m = -(long)(x < P); sum += x & m; }
    return sum;
}
int main(int argc, char **argv){
    const char *mode = argc > 1 ? argv[1] : "branchy"; int P = argc > 2 ? atoi(argv[2]) : 50;
    uint8_t *a = malloc(N); for (long i = 0; i < N; i++) a[i] = rnd() % 100;
    if (!strcmp(mode, "sorted")) {            // 計数ソート (値は 0..99)。分岐の少ない整列で、branch-misses を汚さない
        long cnt[100] = {0}; for (long i = 0; i < N; i++) cnt[a[i]]++;
        long k = 0; for (int v = 0; v < 100; v++) for (long c = 0; c < cnt[v]; c++) a[k++] = v;
    }
    double best = 1e9; long sum = 0;
    for (int trial = 0; trial < 5; trial++) {
        double t0 = now_s();
        sum = !strcmp(mode, "branchless") ? run_branchless(a, P) : run_branchy(a, P);
        double t = now_s() - t0; if (t < best) best = t;
    }
    printf("%-10s P=%3d%%  %.2f ns/要素  (sum=%ld)\n", mode, P, best / N * 1e9, sum);
    return 0;
}
