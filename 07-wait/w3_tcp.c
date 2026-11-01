// 第9章: ループバック TCP の往復。サーバー (子プロセス) は即座に応答する。往復の遅延は tc netem (root) で lo に加える。
// 使い方: w3_tcp <seq|pipeline> [件数]   seq: 1 件ずつ往復 / pipeline: 256 件ずつまとめて送って読む
// netem を使わない状態 (遅延なし) の値が基準。lo に `tc qdisc add dev lo root netem delay 2ms` を入れると、往復で約 4ms (行きと帰りの各 2ms) かかる。
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/wait.h>
static void server(int ls){
    int c = accept(ls, NULL, NULL); int one = 1; setsockopt(c, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    int v; while (read(c, &v, sizeof v) == sizeof v) if (write(c, &v, sizeof v) != sizeof v) break;
    _exit(0);
}
int main(int argc, char **argv){
    const char *mode = argc > 1 ? argv[1] : "seq"; int n = argc > 2 ? atoi(argv[2]) : 200;
    if (strcmp(mode, "seq") && strcmp(mode, "pipeline")) { fprintf(stderr, "mode: seq|pipeline\n"); return 2; }
    int ls = socket(AF_INET, SOCK_STREAM, 0); struct sockaddr_in a = { .sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK) };
    if (bind(ls, (void*)&a, sizeof a) || listen(ls, 1)) { perror("bind/listen"); return 1; }
    socklen_t al = sizeof a; getsockname(ls, (void*)&a, &al);
    pid_t pid = fork(); if (pid == 0) server(ls);
    int c = socket(AF_INET, SOCK_STREAM, 0); int one = 1; setsockopt(c, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    if (connect(c, (void*)&a, sizeof a)) { perror("connect"); return 1; }
    long sum = 0; int v;
    if (!strcmp(mode, "seq")) {
        for (int i = 0; i < n; i++) { if (write(c, &i, sizeof i) != sizeof i || read(c, &v, sizeof v) != sizeof v) { perror("rpc"); return 1; } sum += v; }
    } else {
        const int WIN = 256;
        for (int base = 0; base < n; base += WIN) { int cnt = n - base < WIN ? n - base : WIN;
            for (int i = base; i < base + cnt; i++) if (write(c, &i, sizeof i) != sizeof i) { perror("write"); return 1; }
            for (int i = base; i < base + cnt; i++) { if (read(c, &v, sizeof v) != sizeof v) { perror("read"); return 1; } sum += v; } }
    }
    close(c); waitpid(pid, NULL, 0); printf("sum=%ld\n", sum); return 0;
}
