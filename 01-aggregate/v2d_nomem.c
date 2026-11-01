// 第3章 v2d: v2c と同じ書き方 (店番号は固定幅として p += 6 で進める。memchr を 1 回減らす) だが、入力検査はしない版。
// v2c との差が「検査だけのコスト」になる。前提は v2a と同じ (入力は生成器の書式)。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NSTORE 1000
int main(void){
    static long long total[NSTORE];
    char line[4096];
    setvbuf(stdin, aligned_alloc(4096, 65536), _IOFBF, 65536);
    while (fgets(line, sizeof line, stdin)) {
        char *p = memchr(line, ',', sizeof line) + 1;
        int store = (p[1]-'0')*1000 + (p[2]-'0')*100 + (p[3]-'0')*10 + (p[4]-'0');
        p += 6;
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
