// 第4章: 「バッファの位置によってコピーが遅い」の確認用。ユーザー空間で rep movsb を使い、コピー元と先のページ内オフセットの差を振る。
// 使い方: copy_alias <src のページ内オフセット> <dst のページ内オフセット>   (64KB をコピーした時間を出す)
// 結果は 4096 境界での差 (dst - src) mod 4096 が 1〜31 のときだけ遅い (この環境の Ryzen 7 5700X で確認)。メカニズムは仮説 (4K エイリアシングに似た CPU 内部の挙動)。
#include "../03-string/bench.h"
int main(int argc, char **argv){
    size_t so = argc > 1 ? strtoul(argv[1],0,10) : 0, d = argc > 2 ? strtoul(argv[2],0,10) : 0, n = 65536;
    char *src = aligned_alloc(4096, 4096 * 40), *dst = aligned_alloc(4096, 4096 * 40);
    memset(src, 1, 4096 * 40); memset(dst, 0, 4096 * 40);
    double best = 1e9;
    for (int t = 0; t < 2000; t++) {
        double t0 = now_s(); void *p = dst + d; const void *q = src + so; size_t c = n;
        __asm__ volatile("rep movsb" : "+D"(p), "+S"(q), "+c"(c) : : "memory");
        double dt = now_s() - t0; if (dt < best) best = dt;
    }
    printf("src_off=%-5zu dst_off=%-5zu  差=%-5ld  %.2f us / 64KB\n", so, d, (long)((d - so) & 4095), best * 1e6);
    return 0;
}
