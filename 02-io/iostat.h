// strace が使えない環境 (root なし) の代替。環境変数 IOSTAT=1 のとき、終了時に
// /proc/self/io の syscr / syscw (read/write 系システムコールの回数) と rchar / wchar を stderr に出す。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void iostat_dump(void){
    FILE *f = fopen("/proc/self/io", "r");
    if (!f) return;
    char k[64]; long long v; fprintf(stderr, "[iostat]");
    while (fscanf(f, "%63[^:]: %lld\n", k, &v) == 2)
        if (!strcmp(k,"syscr") || !strcmp(k,"syscw")) fprintf(stderr, " %s=%lld", k, v);
    fputc('\n', stderr); fclose(f);
}
__attribute__((constructor)) static void iostat_init(void){
    if (getenv("IOSTAT")) atexit(iostat_dump);
}
