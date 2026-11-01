// 第3章 v0: 素朴な実装。fgets + strtok + atof + double 集計。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NSTORE 1000
int main(void){
    static double total[NSTORE];
    char line[4096];
    while (fgets(line, sizeof line, stdin)) {
        char *date = strtok(line, ",");
        char *store = strtok(NULL, ",");
        char *prod = strtok(NULL, ",");
        char *qty = strtok(NULL, ",");
        char *amt = strtok(NULL, ",\n");
        (void)date; (void)prod; (void)qty;
        total[atoi(store + 1)] += atof(amt);
    }
    for (int i = 0; i < NSTORE; i++) printf("S%04d,%.2f\n", i, total[i]);
    return 0;
}
