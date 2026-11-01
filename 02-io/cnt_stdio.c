// 第4章: stdio の読み方を変えて行数を数える。使い方: cnt_stdio <fgets|getline|getc> [stdioバッファ(0=バッファなし)] [aligned]
// 第 3 引数 aligned を付けると、setvbuf に 4096 境界のバッファを渡す (既定は glibc 内部の malloc バッファ)。
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iostat.h"
int main(int argc, char **argv){
    const char *mode = argc > 1 ? argv[1] : "fgets";
    if (argc > 2) {
        size_t b = strtoul(argv[2], 0, 10);
        if (b == 0) setvbuf(stdin, NULL, _IONBF, 0);
        else if (argc > 3 && !strcmp(argv[3], "aligned")) setvbuf(stdin, aligned_alloc(4096, (b + 4095) / 4096 * 4096), _IOFBF, b);
        else setvbuf(stdin, NULL, _IOFBF, b);
    }
    if (strcmp(mode, "fgets") && strcmp(mode, "getline") && strcmp(mode, "getc")) { fprintf(stderr, "mode: fgets|getline|getc\n"); return 2; }
    long lines = 0;
    if (!strcmp(mode, "fgets")) {
        char line[4096];
        while (fgets(line, sizeof line, stdin)) lines++;
    } else if (!strcmp(mode, "getline")) {
        char *line = NULL; size_t cap = 0;
        while (getline(&line, &cap, stdin) > 0) lines++;
    } else {
        int c;
        while ((c = getc(stdin)) != EOF) if (c == '\n') lines++;
    }
    printf("%ld\n", lines);
    return 0;
}
