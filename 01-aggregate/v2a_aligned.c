// 第3章 v2a: v2 と同じ。違いは stdio のバッファを 4096 境界に揃え、サイズも SB (既定 64KB) にしたこと (setvbuf)。-DSB=4096 なら位置だけの違い。glibc 内部の malloc バッファは 16 バイトずれ、カーネルのコピーが遅くなる。
// 前提 (これを外れると誤集計かクラッシュ。本の第3章「入力の前提」を参照): 行は 'date,S0000..S0999,product(15字以下),qty,金額' の形、金額は符号なしで小数点以下ちょうど 2 桁、LF 区切り、空行・ヘッダなし。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NSTORE 1000
#ifndef SB
#define SB 65536
#endif
int main(void){
    static long long total[NSTORE];   // 単位: 0.01
    char line[4096];
    setvbuf(stdin, aligned_alloc(4096, SB), _IOFBF, SB);   // ← v2 との違い: バッファの位置 (4096 境界) と、SB バイトのサイズ (既定 65536。4096 にすると位置だけの違い)
    while (fgets(line, sizeof line, stdin)) {
        char *p = memchr(line, ',', sizeof line) + 1;
        const char *s = p + 1;                         // 'S' を飛ばす
        int store = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0');
        p = memchr(p, ',', 16) + 1;
        p = memchr(p, ',', 16) + 1;
        p = memchr(p, ',', 16) + 1;
        long long v = 0;
        while (*p != '.') v = v*10 + (*p++ - '0');
        p++;
        v = v*100 + (p[0]-'0')*10 + (p[1]-'0');
        total[store] += v;
    }
    for (int i = 0; i < NSTORE; i++) printf("S%04d,%lld.%02lld\n", i, total[i]/100, total[i]%100);
    return 0;
}
