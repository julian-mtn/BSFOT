#define _POSIX_C_SOURCE 200809L
#include "timer.h"
#include <time.h>

double timespec_diff_ms(struct timespec start, struct timespec end){
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
}


void timer_start(benchmark_timer_t *timer){
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}


double timer_stop_ms(benchmark_timer_t *timer){
    
    clock_gettime(CLOCK_MONOTONIC, &timer->end);

    return timespec_diff_ms(timer->start, timer->end);
}