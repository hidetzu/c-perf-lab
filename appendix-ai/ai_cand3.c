// このファイルは、AI (Claude のサブエージェント) が生成したコードです。評価の対象として載せています。
// cand1: read() で大きなブロックを読み、手書きで区切り/整数/金額をパースする。
// 金額は「整数セント / 100.0」で double 化 (atof と同じ正しい丸め結果) し、加算順も元のまま。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define NSTORE 1000
#define BUFSZ (1 << 20)

static double total[NSTORE];

static const char *nextc(const char *p, const char *e) {   // p 以降で最初のカンマ。無ければ NULL
    return p < e ? memchr(p, ',', e - p) : NULL;
}

// [p,e) は改行を含まない1行。仕様 (v0r_ref.c) どおり: 行末の CR を無視、NUL で文字列が終わる扱い、
// 欄は字面どおり、5欄未満は読み飛ばし、店番号は 'S'+1桁以上の10進で 0..999 のみ有効、金額は5欄目を atof。
static void do_line(const char *p, const char *e) {
    while (e > p && e[-1] == '\r') e--;
    const char *z = memchr(p, 0, e - p);
    if (z) e = z;
    const char *c1 = nextc(p, e);          if (!c1) return;
    const char *c2 = nextc(c1 + 1, e);     if (!c2) return;
    const char *c3 = nextc(c2 + 1, e);     if (!c3) return;
    const char *c4 = nextc(c3 + 1, e);     if (!c4) return;
    const char *s = c1 + 1;
    if (c2 - s < 2 || s[0] != 'S') return;
    int idx = 0;
    for (const char *q = s + 1; q < c2; q++) {
        if ((unsigned)(*q - '0') >= 10) return;
        idx = idx * 10 + (*q - '0');
        if (idx >= NSTORE) return;         // 早めに打ち切るので int は溢れない
    }
    const char *a = c4 + 1;
    const char *ae = nextc(a, e);
    if (!ae) ae = e;                       // 金額欄は [a,ae)
    // 高速経路: 整数部1〜13桁 + '.' + ちょうど2桁。全体が 10^15 未満の整数セントなので double に厳密に載り、
    // /100.0 は正しく丸められる (= atof と同値)。
    const char *t = a;
    unsigned long ip = 0;
    while (t < ae && (unsigned)(*t - '0') < 10 && t - a < 14) ip = ip * 10 + (*t++ - '0');
    double v;
    if (t > a && t - a <= 13 && ae - t == 3 && *t == '.' &&
        (unsigned)(t[1] - '0') < 10 && (unsigned)(t[2] - '0') < 10) {
        v = (double)(ip * 100 + (t[1] - '0') * 10 + (t[2] - '0')) / 100.0;
    } else {                               // それ以外の書式は標準関数に任せる
        size_t n = ae - a;
        char *tmp = malloc(n + 1);
        if (!tmp) { fputs("out of memory\n", stderr); exit(1); }
        memcpy(tmp, a, n); tmp[n] = 0;
        v = atof(tmp);
        free(tmp);
    }
    total[idx] += v;
}

int main(void) {
    size_t cap = BUFSZ, have = 0, scan = 0;   // scan: buf[0..scan) に改行が無いと分かっている範囲
    char *buf = malloc(cap + 1);
    if (!buf) { fputs("out of memory\n", stderr); return 1; }
    for (;;) {
        if (have == cap) {                    // 改行なしで満杯: 行長に上限を設けず倍に拡張
            if (cap > ((size_t)-1) / 4) { fputs("line too long\n", stderr); return 1; }
            cap *= 2;
            char *nb = realloc(buf, cap + 1);
            if (!nb) { fputs("out of memory\n", stderr); return 1; }
            buf = nb;
        }
        ssize_t r = read(0, buf + have, cap - have);
        if (r < 0) { perror("read"); return 1; }
        if (r == 0) break;
        have += r;
        char *p = buf, *end = buf + have, *nl;
        while ((nl = memchr(p + (p == buf ? scan : 0), '\n', end - p - (p == buf ? scan : 0)))) { do_line(p, nl); p = nl + 1; }
        size_t rest = end - p;
        if (p == buf) scan = have; else { memmove(buf, p, rest); scan = 0; }
        have = rest;
    }
    if (have) do_line(buf, buf + have);
    for (int i = 0; i < NSTORE; i++) printf("S%04d,%.2f\n", i, total[i]);
    return 0;
}
