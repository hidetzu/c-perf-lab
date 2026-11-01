// 第4章: 出力側。600万行を書く。使い方: out_lines <flush|unbuf|buf|block> <出力先ファイル>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "iostat.h"
#define N 6000000
int main(int argc, char **argv){
    const char *mode = argc > 1 ? argv[1] : "buf";
    const char *path = argc > 2 ? argv[2] : "/dev/null";
    if (strcmp(mode, "block") && strcmp(mode, "flush") && strcmp(mode, "unbuf") && strcmp(mode, "buf")) { fprintf(stderr, "mode: flush|unbuf|buf|block\n"); return 2; }
    if (!strcmp(mode, "block")) {
        int fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0644); if (fd < 0) { perror(path); return 1; }
        static char buf[1<<20]; size_t n = 0;
        for (long i = 0; i < N; i++) {
            if (n > sizeof buf - 64) { write(fd, buf, n); n = 0; }
            n += sprintf(buf + n, "S%04ld,%ld.%02ld\n", i % 1000, i % 100000, i % 100);
        }
        write(fd, buf, n); close(fd); return 0;
    }
    FILE *fp = fopen(path, "w"); if (!fp) { perror(path); return 1; }
    if (!strcmp(mode, "unbuf")) setvbuf(fp, NULL, _IONBF, 0);
    for (long i = 0; i < N; i++) {
        fprintf(fp, "S%04ld,%ld.%02ld\n", i % 1000, i % 100000, i % 100);
        if (!strcmp(mode, "flush")) fflush(fp);
    }
    fclose(fp);
    return 0;
}
