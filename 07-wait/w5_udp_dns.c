// 第9章: DNS の問い合わせを UDP で直接スタブリゾルバ (既定 127.0.0.53:53) に送り、応答までの待ちを測る。
// glibc の getaddrinfo は、この環境 (nsswitch の hosts に resolve) では systemd-resolved と Unix ソケットで話すため、lo の遅延 (tc netem) が効かない。
// 使い方: w5_udp_dns <seq|pipeline> [件数] [名前] [サーバー]   seq: 1 件ずつ問い合わせて応答を待つ / pipeline: 全部送ってからまとめて受ける
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
static double now_s(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }
static int build(unsigned char *b, unsigned id, const char *name){
    int n = 0; b[n++] = id >> 8; b[n++] = id; b[n++] = 0x01; b[n++] = 0x00; b[n++] = 0; b[n++] = 1; memset(b + n, 0, 6); n += 6;   // ヘッダ: RD=1, QDCOUNT=1
    const char *p = name; while (*p) { const char *d = strchr(p, '.'); size_t l = d ? (size_t)(d - p) : strlen(p); if (l > 63) return -1; b[n++] = l; memcpy(b + n, p, l); n += l; p += l + (d ? 1 : 0); if (!d) break; }
    b[n++] = 0; b[n++] = 0; b[n++] = 1; b[n++] = 0; b[n++] = 1; return n;                                                          // 終端、QTYPE=A、QCLASS=IN
}
int main(int argc, char **argv){
    const char *mode = argc > 1 ? argv[1] : "seq"; int n = argc > 2 ? atoi(argv[2]) : 50; const char *name = argc > 3 ? argv[3] : "x.invalid"; const char *srv = argc > 4 ? argv[4] : "127.0.0.53";
    if ((strcmp(mode, "seq") && strcmp(mode, "pipeline")) || n < 1 || n > 60000) { fprintf(stderr, "usage: %s seq|pipeline [n<=60000] [name] [server]\n", argv[0]); return 2; }
    int s = socket(AF_INET, SOCK_DGRAM, 0); struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons(53) };
    if (inet_pton(AF_INET, srv, &a.sin_addr) != 1 || connect(s, (void*)&a, sizeof a)) { perror("connect"); return 1; }
    unsigned char q[300], r[600]; int got = 0, lost = 0; double t0 = now_s();
    if (!strcmp(mode, "seq")) {
        for (int i = 0; i < n; i++) { int l = build(q, i, name); if (send(s, q, l, 0) < 0) { perror("send"); return 1; }
            struct pollfd p = { s, POLLIN, 0 }; if (poll(&p, 1, 2000) > 0 && recv(s, r, sizeof r, 0) > 0) got++; else lost++; }
    } else {
        for (int i = 0; i < n; i++) { int l = build(q, i, name); if (send(s, q, l, 0) < 0) { perror("send"); return 1; } }
        for (int i = 0; i < n; i++) { struct pollfd p = { s, POLLIN, 0 }; if (poll(&p, 1, 2000) > 0 && recv(s, r, sizeof r, 0) > 0) got++; else { lost = n - got; break; } }
    }
    double d = now_s() - t0; printf("%-9s n=%d  応答 %d 失 %d  全体 %.1f ms  (1 件あたり %.2f ms)  server=%s name=%s\n", mode, n, got, lost, d * 1e3, d * 1e3 / n, srv, name); return 0;
}
