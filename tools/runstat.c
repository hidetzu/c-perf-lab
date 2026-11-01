// runstat: プログラムを N 回実行し、実時間・user・sys の中央値を出す。
// perf_event_paranoid=2 では perf stat がカーネル側 (sys) の時間を分けて出せないため、wait4 の rusage を使う。
// 使い方: runstat <入力ファイル|-> <回数> <CPU番号|-> コマンド [引数...]   (標準入力にしたいファイル名が - で始まるときは ./ を付ける。回数が偶数なら上側の中央値)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
static int cmp(const void *a, const void *b){ double x=*(double*)a, y=*(double*)b; return (x>y)-(x<y); }
static double med(double *v, int n){ qsort(v, n, sizeof *v, cmp); return v[n/2]; }
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }
int main(int argc, char **argv){
    if (argc < 5 || atoi(argv[2]) <= 0) { fprintf(stderr, "usage: %s <input|-> <runs> <cpu|-> cmd [args]\n", argv[0]); return 2; }
    const char *in = argv[1]; int runs = atoi(argv[2]);
    char *endp; int cpu = argv[3][0]=='-' ? -1 : (int)strtol(argv[3], &endp, 10);
    if (argv[3][0] != '-' && *endp) { fprintf(stderr, "cpu は数字か - : %s\n", argv[3]); return 2; }
    double *w = malloc(runs*sizeof *w), *u = malloc(runs*sizeof *u), *s = malloc(runs*sizeof *s);
    for (int i = 0; i < runs; i++) {
        double t0 = now(); pid_t pid = fork();
        if (pid == 0) {
            if (cpu >= 0) { cpu_set_t m; CPU_ZERO(&m); CPU_SET(cpu,&m); if (sched_setaffinity(0,sizeof m,&m)) { perror("sched_setaffinity (CPU 番号を確認: nproc)"); _exit(125); } }
            if (in[0] != '-') { int fd = open(in, O_RDONLY); if (fd < 0) { perror(in); _exit(126); } dup2(fd, 0); close(fd); }
            int o = open("/dev/null", O_WRONLY); dup2(o, 1); close(o);
            execvp(argv[4], argv+4); perror(argv[4]); _exit(127);
        }
        int st; struct rusage ru; wait4(pid, &st, 0, &ru);
        if (!WIFEXITED(st) || WEXITSTATUS(st) != 0) { fprintf(stderr, "runstat: 子プロセスが失敗した (status=0x%x)。コマンド名・入力パス・ビルドを確認\n", st); return 1; }
        w[i] = now()-t0;
        u[i] = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec*1e-6;
        s[i] = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec*1e-6;
    }
    printf("wall %.0f ms  user %.0f ms  sys %.0f ms  (median of %d)\n", med(w,runs)*1e3, med(u,runs)*1e3, med(s,runs)*1e3, runs);
    return 0;
}
