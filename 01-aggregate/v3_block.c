// 第3章 v3: fgets もやめ、read() で大きなブロックを読み、その場で解析する。
// 前提 (これを外れると誤集計かクラッシュ。本の第3章「入力の前提」を参照): 行は 'date,S0000..S0999,product(15字以下),qty,金額' の形、金額は符号なしで小数点以下ちょうど 2 桁、LF 区切り、空行・ヘッダなし。
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define NSTORE 1000
#define BUFSZ (1<<20)
static char buf[BUFSZ + 128];
int main(void){
    static long long total[NSTORE];
    size_t have = 0;
    for (;;) {
        ssize_t r = read(0, buf + have, BUFSZ - have);
        if (r < 0) return 1;
        have += r;
        char *end = buf + have, *p = buf, *last = NULL;
        if (r == 0) { last = end; }                    // EOF: 残りは全部処理
        else { last = memrchr(buf, '\n', have); if (!last) return 1; last++; }
        while (p < last) {
            p = memchr(p, ',', 16) + 1;                // date
            const char *s = p + 1;
            int store = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0');
            p += 6;                                    // "S0000,"
            p = memchr(p, ',', 16) + 1;                // product
            p = memchr(p, ',', 16) + 1;                // qty
            long long v = 0;
            while (*p != '.') v = v*10 + (*p++ - '0');
            p++;
            v = v*100 + (p[0]-'0')*10 + (p[1]-'0');
            p += 3;                                    // "dd\n"
            total[store] += v;
        }
        if (r == 0) break;
        have = end - last;
        memmove(buf, last, have);
    }
    for (int i = 0; i < NSTORE; i++) printf("S%04d,%lld.%02lld\n", i, total[i]/100, total[i]%100);
    return 0;
}
