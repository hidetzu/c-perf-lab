// 第3章 v2c: v2a に入力検査を加えた版。欄の数、店番号の桁と範囲、金額の桁と小数点以下の桁数を検査し、不正な行は数えて読み飛ばす。
// 金額の整数部は 10 桁まで (合計が 64bit に収まる前提: 1 行 1e12 銭で約 920 万行まで)。date / product / qty の中身は検査しない。
// 目的: 「速さのために省いた検査」を入れ直すと何 ms かかるかを測る。許す形: 金額は 整数部(1桁以上).小数2桁、符号なし。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NSTORE 1000
static inline int dig(char c){ return (unsigned)(c - '0') <= 9; }
int main(void){
    static long long total[NSTORE];
    char line[4096]; long bad = 0;
    setvbuf(stdin, aligned_alloc(4096, 65536), _IOFBF, 65536);
    while (fgets(line, sizeof line, stdin)) {
        size_t len = strlen(line); char *end = line + len;
        if (len == sizeof line - 1 && line[len-1] != '\n') {      // 行が長すぎて fgets が途中で切った: 残りを読み捨てて 1 行の不正として数える
            int c; while ((c = getchar()) != EOF && c != '\n') ;
            bad++; continue;
        }     // fgets は残りを上書きしないので、行の終わりを自分で見る (空行で前の行の残骸を読まないため)
        char *p = memchr(line, ',', len);
        if (!p) { bad++; continue; }
        p++;
        if (end - p < 6 || p[0] != 'S' || !dig(p[1]) || !dig(p[2]) || !dig(p[3]) || !dig(p[4]) || p[5] != ',') { bad++; continue; }
        int store = (p[1]-'0')*1000 + (p[2]-'0')*100 + (p[3]-'0')*10 + (p[4]-'0');
        if (store >= NSTORE) { bad++; continue; }
        p += 6;
        char *q = memchr(p, ',', end - p); if (!q) { bad++; continue; } p = q + 1;      // product
        q = memchr(p, ',', end - p); if (!q) { bad++; continue; } p = q + 1;            // qty
        long long v = 0; int nd = 0;
        while (p < end && dig(*p)) { v = v*10 + (*p++ - '0'); if (++nd > 10) break; }
        if (nd == 0 || nd > 10 || end - p < 3 || p[0] != '.' || !dig(p[1]) || !dig(p[2]) || (p[3] != '\n' && p[3] != '\r' && p[3] != 0)) { bad++; continue; }
        v = v*100 + (p[1]-'0')*10 + (p[2]-'0');
        total[store] += v;
    }
    for (int i = 0; i < NSTORE; i++) printf("S%04d,%lld.%02lld\n", i, total[i]/100, total[i]%100);
    if (bad) fprintf(stderr, "skipped %ld bad lines\n", bad);
    return 0;
}
