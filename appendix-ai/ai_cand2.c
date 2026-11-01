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

static void do_line(const char *p, const char *e) {   // [p,e) は改行を含まない1行
    const char *q = memchr(p, ',', e - p);
    if (!q) return;
    q++;                                   // store フィールド先頭 ('S')
    const char *s = q + 1;
    int idx = 0;
    while (s < e && (unsigned)(*s - '0') < 10) idx = idx * 10 + (*s++ - '0');
    q = memchr(s, ',', e - s);             // product の前
    if (!q) return;
    q = memchr(q + 1, ',', e - q - 1);     // qty の前
    if (!q) return;
    q = memchr(q + 1, ',', e - q - 1);     // amount の前
    if (!q) return;
    const char *a = q + 1;
    // 高速経路: 数字+ '.' + ちょうど2桁、直後が行末か ',' か '\r'
    const char *t = a;
    unsigned long ip = 0; int nd = 0;
    while (t < e && (unsigned)(*t - '0') < 10 && nd < 15) { ip = ip * 10 + (*t++ - '0'); nd++; }
    double v;
    if (nd > 0 && nd < 15 && t + 2 < e + 0 && *t == '.' &&
        (unsigned)(t[1] - '0') < 10 && (unsigned)(t[2] - '0') < 10 &&
        (t + 3 == e || t[3] == ',' || t[3] == '\r')) {
        v = (double)(ip * 100 + (t[1] - '0') * 10 + (t[2] - '0')) / 100.0;
    } else {                               // 想定外の書式は標準関数に任せる
        size_t n = e - a;
        char *tmp = malloc(n + 1);
        if (!tmp) { fputs("out of memory\n", stderr); exit(1); }
        memcpy(tmp, a, n); tmp[n] = 0;
        char *c = strchr(tmp, ','); if (c) *c = 0;
        v = atof(tmp);
        free(tmp);
    }
    if ((unsigned)idx < NSTORE) total[idx] += v;
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
