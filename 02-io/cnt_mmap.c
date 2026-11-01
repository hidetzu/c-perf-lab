// 第4章: mmap で行数を数える。使い方: cnt_mmap < file   (パイプでは使えない)
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "iostat.h"
int main(void){
    struct stat st;
    if (fstat(0, &st) || !S_ISREG(st.st_mode)) { fprintf(stderr, "stdin must be a regular file\n"); return 1; }
    if (st.st_size == 0) { puts("0"); return 0; }
    const char *p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, 0, 0);
    if (p == MAP_FAILED) { perror("mmap"); return 1; }
    long lines = 0; const char *e = p + st.st_size, *q = p;
    while ((q = memchr(q, '\n', e - q))) { lines++; q++; }
    printf("%ld\n", lines);
    return 0;
}
