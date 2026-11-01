// 第5・7・8章 共通: 最小時間 (複数回の最小) を測る小さなヘルパ。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
static inline double now_s(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }
#define KEEP(x) __asm__ volatile("" : : "r"(x) : "memory")      // 結果を捨てられないようにする (第1章)
