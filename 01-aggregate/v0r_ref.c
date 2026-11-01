// 第3章 v0r: 評価ハーネス (appendix-ai/eval.sh) の「正しさの基準」。v0 の挙動をそのまま写したものではなく、
// 「出力が正しいとは何か」を仕様として固定した版。仕様 (本来は人が決める取り決め。この実験では AI が人の役で決めた):
//   1. 行は改行まで。長さの上限はない (getline)。行末の CR は無視する (出力には影響しない。atof が末尾の CR を読み捨てるため)。
//   2. 欄はカンマで区切り、字面どおりに数える (空欄は空欄)。欄が 5 つ未満の行 (空行を含む) は読み飛ばす。
//   3. 店番号の欄は 'S' に 1 桁以上の 10 進数が続く形だけを有効とする。それ以外、または 0..999 の範囲外は読み飛ばす。
//   4. 金額は 5 番目の欄 (次のカンマまたは行末まで) を atof で読んだ double を、行の順に足す。数値でなければ atof の仕様どおり 0 を足す。6 番目以降の欄は無視する。
//   5. 出力は店番号 0..999 の順に S%04d,%.2f
//   6. 行に NUL バイトがあれば、C の文字列と同じくそこで行が終わる (strchr/atof が NUL で止まる)。金額の結果が NaN になる入力の符号は規定しない
//      (-O1 と -O2 で NaN の符号が違う実装があった。入力に inf/nan を加えるなら、比較前に -nan を nan に正規化する)
// v0 との違い (仕様として決めたもの): 長い行を 1 行として読む (v0 は fgets の 4095 文字で切って SEGV)、空欄を詰めない (v0 は strtok で詰めて欄がずれる)、店番号の厳密な検査 (v0 は atoi)。
// 速さ: 通常入力で v0 (867 ms) より速い (488 ms)。fgets+strtok をやめて自前の欄分割にした分と、getline のバッファ位置が第4章の窓の外にある分。「基準比」の倍率は、この v0r を 1 として測る。
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NSTORE 1000
int main(void){
    static double total[NSTORE];
    char *line = NULL; size_t cap = 0; ssize_t n;
    while ((n = getline(&line, &cap, stdin)) > 0) {
        while (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) line[--n] = 0;
        char *f[5]; int nf = 0; char *p = line;
        for (;;) {
            f[nf++] = p;
            char *c = strchr(p, ',');
            if (!c || nf == 5) break;
            *c = 0; p = c + 1;
        }
        if (nf < 5) continue;                                   // 欄が 5 つ未満
        { char *e = strchr(f[4], ','); if (e) *e = 0; }          // 金額の欄は次のカンマまで
        char *s = f[1];
        if (s[0] != 'S' || s[1] < '0' || s[1] > '9') continue;
        long id = 0; int ok = 1;
        for (char *q = s + 1; *q; q++) { if (*q < '0' || *q > '9') { ok = 0; break; } id = id * 10 + (*q - '0'); if (id >= NSTORE) { ok = 0; break; } }
        if (!ok) continue;
        total[id] += atof(f[4]);
    }
    free(line);
    for (int i = 0; i < NSTORE; i++) printf("S%04d,%.2f\n", i, total[i]);
    return 0;
}
