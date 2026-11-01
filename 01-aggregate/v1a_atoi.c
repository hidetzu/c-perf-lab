// 第3章 v1a: v1 の atoi だけを自前の 4 桁変換に置き換えた版 (atof は残す)。v1→v2 を「店番号の変換」と「金額の変換」に分けて測る。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NSTORE 1000
int main(void){
    static double total[NSTORE];
    char line[4096];
    while (fgets(line, sizeof line, stdin)) {
        char *p = line;
        p = memchr(p, ',', sizeof line) + 1;          // date を飛ばす
        char *store = p;
        p = memchr(p, ',', 16) + 1;                   // store の終わり
        p = memchr(p, ',', 16) + 1;                   // product を飛ばす
        p = memchr(p, ',', 16) + 1;                   // qty を飛ばす
        const char *sp = store + 1; int sid = (sp[0]-0x30)*1000 + (sp[1]-0x30)*100 + (sp[2]-0x30)*10 + (sp[3]-0x30);
        total[sid] += atof(p);
    }
    for (int i = 0; i < NSTORE; i++) printf("S%04d,%.2f\n", i, total[i]);
    return 0;
}
