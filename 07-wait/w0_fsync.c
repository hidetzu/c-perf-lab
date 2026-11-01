// 第9章: 書き込みごとに fsync する。CPU はほとんど使わず、ディスクへの書き込み完了を待つ。
// 使い方: w0_fsync <each|batch|once> <出力ファイル>   (出力先は tmpfs でなく実ディスク上に置くこと)
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#define N 20000
int main(int argc, char **argv){
    const char *mode = argc > 1 ? argv[1] : "each";
    if (strcmp(mode, "each") && strcmp(mode, "batch") && strcmp(mode, "once")) { fprintf(stderr, "mode: each|batch|once\n"); return 2; }
    int fd = open(argc > 2 ? argv[2] : "w0.out", O_WRONLY|O_CREAT|O_TRUNC, 0644); if (fd < 0) { perror("open"); return 1; }
    char rec[64]; memset(rec, 'x', sizeof rec); rec[63] = '\n';
    for (int i = 0; i < N; i++) {
        if (write(fd, rec, sizeof rec) != sizeof rec) { perror("write"); return 1; }
        if (!strcmp(mode, "each") || (!strcmp(mode, "batch") && i % 1000 == 999)) { if (fsync(fd)) { perror("fsync"); return 1; } }
    }
    if (strcmp(mode, "each") && fsync(fd)) { perror("fsync"); return 1; }
    close(fd); return 0;
}
