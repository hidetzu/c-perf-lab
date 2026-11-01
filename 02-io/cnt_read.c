// 第4章: read() のバッファサイズを変えて、行数を数える。使い方: cnt_read <bufsize> [バッファの位置のずれ(バイト、既定 0)]
// 読み込み先は aligned_alloc(4096) で確保し、先頭を off バイトずらせる。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "iostat.h"
int main(int argc, char **argv){
    size_t n = argc > 1 ? strtoul(argv[1], 0, 10) : 65536;
    if (n == 0) { fprintf(stderr, "bufsize must be >= 1\n"); return 2; }
    size_t off = argc > 2 ? strtoul(argv[2], 0, 10) : 0;
    char *base = aligned_alloc(4096, (n + off + 4095) / 4096 * 4096), *buf = base + off;
    long lines = 0; ssize_t r;
    while ((r = read(0, buf, n)) > 0) {
        const char *p = buf, *e = buf + r;
        while ((p = memchr(p, '\n', e - p))) { lines++; p++; }
    }
    printf("%ld\n", lines);
    return 0;
}
