#ifndef TIMER_H
#define TIMER_H

#include <time.h>

typedef struct {
    struct timespec start;
    struct timespec end;
} benchmark_timer_t;


void timer_start(benchmark_timer_t *timer);

double timer_stop_ms(benchmark_timer_t *timer);


#endif