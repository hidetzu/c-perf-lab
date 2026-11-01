// 第1章 デモ3: 符号付きオーバーフロー (未定義動作)。a + b < a は、GCC 16 では -O0 でも「b < 0」に畳み込まれる (objdump で shr $31 を確認)。
// 本来の桁あふれ検出に戻すには -fwrapv を付ける。-fsanitize=undefined を付けると実行時にオーバーフローを報告する。
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
__attribute__((noinline)) int will_overflow(int a, int b){ return a + b < a; }
int main(int argc, char **argv){
    int a = argc > 1 ? atoi(argv[1]) : INT_MAX;
    printf("a=%d b=1 -> will_overflow=%d\n", a, will_overflow(a, 1));
    return 0;
}
