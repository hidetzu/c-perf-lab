// 第9章: 1 件ずつ要求→応答を待つ (往復の遅延に律速)。子プロセスが「遅い相手」(1 件 2ms かかる) を演じる。
// 使い方: w1_rpc <seq|pipeline|conc> [件数] [serial|latency]
//   pipeline は 256 件ずつの窓でまとめて送る (窓なしで全件を先に送ると、件数が数百〜数千を超えたところで双方が write で止まりデッドロックする。初回の実装の不具合)
//   serial  (既定): 相手が 1 件ずつ直列に 2ms かけて処理する (相手の処理速度が律速)
//   latency       : 相手は即応答できるが、往復に 2ms の遅延がかかる。遅延は並列に進む (ネットワークの往復遅延の模擬。複数の要求が同時に遅延中でよい)
//   seq: 1 件送って応答を待つ。pipeline: 先に全部送ってから応答をまとめて読む。conc: 相手を 8 並列で動かす (複数の子)。
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <poll.h>
#define DELAY_US 2000
static void server(int fd){            // 要求 (4 バイト) を受け取り、DELAY_US 待って同じ値を返す
    int v;
    while (read(fd, &v, sizeof v) == sizeof v) {
        usleep(DELAY_US);
        write(fd, &v, sizeof v);
    }
    _exit(0);
}
static double now_s(void){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}
static void server_latency(int fd){     // 要求を受け取った時刻 + DELAY_US に応答する。複数の要求が同時に待ってよい (FIFO)
    enum { Q = 65536 };
    static int val[Q];
    static double due[Q];
    long head = 0, tail = 0;
    int eof = 0;
    while (!eof || head < tail) {
        int timeout = -1;
        if (head < tail) {
            double w = (due[head % Q] - now_s()) * 1000;
            timeout = w <= 0 ? 0 : (int)(w + 0.999);
        }
        struct pollfd p = { fd, eof ? 0 : POLLIN, 0 };
        poll(&p, 1, timeout);
        if (!eof && (p.revents & POLLIN)) {
            int v;
            if (read(fd, &v, sizeof v) == sizeof v) {
                val[tail % Q] = v;
                due[tail % Q] = now_s() + DELAY_US * 1e-6;
                tail++;
            } else {
                eof = 1;
            }
        }
        while (head < tail && due[head % Q] <= now_s()) {
            write(fd, &val[head % Q], sizeof(int));
            head++;
        }
    }
    _exit(0);
}
int main(int argc, char **argv){
    const char *mode = argc > 1 ? argv[1] : "seq";
    int n = argc > 2 ? atoi(argv[2]) : 500;
    int lat = argc > 3 && !strcmp(argv[3], "latency");
    int W = !strcmp(mode, "conc") ? 8 : 1;
    int fds[8];
    pid_t pids[8];
    for (int w = 0; w < W; w++) {
        int sv[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
        if ((pids[w] = fork()) == 0) {
            close(sv[0]);
            if (lat) server_latency(sv[1]);
            else server(sv[1]);
        }
        close(sv[1]);
        fds[w] = sv[0];
    }
    long sum = 0;
    int v;
    if (!strcmp(mode, "seq")) {
        for (int i = 0; i < n; i++) {
            if (write(fds[0], &i, sizeof i) != sizeof i || read(fds[0], &v, sizeof v) != sizeof v) {
                perror("rpc");
                return 1;
            }
            sum += v;
        }
    } else {                                                    // pipeline / conc: WIN 件ずつ「まとめて送ってからまとめて読む」。
        const int WIN = 256;                                    // 全件を先に送るとソケットのバッファが埋まり、双方が write で止まる (デッドロック)。窓を区切って避ける
        for (int base = 0; base < n; base += WIN) {
            int cnt = n - base < WIN ? n - base : WIN;
            for (int i = base; i < base + cnt; i++) {
                if (write(fds[i % W], &i, sizeof i) != sizeof i) {
                    perror("write");
                    return 1;
                }
            }
            for (int i = base; i < base + cnt; i++) {
                if (read(fds[i % W], &v, sizeof v) != sizeof v) {
                    perror("read");
                    return 1;
                }
                sum += v;
            }
        }
    }
    for (int w = 0; w < W; w++) close(fds[w]);
    while (wait(NULL) > 0) ;
    printf("sum=%ld\n", sum);
    return 0;
}
