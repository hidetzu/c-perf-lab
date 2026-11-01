// 第3章 v1: strtok をやめ、行を自分で走査して欄を切り出す。atof/atoi は残す。
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
        total[atoi(store + 1)] += atof(p);            // 末尾の改行は atof が読み飛ばす
    }
    for (int i = 0; i < NSTORE; i++) printf("S%04d,%.2f\n", i, total[i]);
    return 0;
}
