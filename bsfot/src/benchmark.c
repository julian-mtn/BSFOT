#include "benchmark.h"
#include "timer.h"
#include "run.h"

#include <stdio.h>


int benchmark_run(benchmark_result_t *result) {
    benchmark_timer_t timer;


    timer_start(&timer);

    int ret = protocol_run(result);

    result->total_ms = timer_stop_ms(&timer);


    result->crypto_ms =
          result->params_ms
        + result->ot_ms
        + result->keygen_ms
        + result->signer_ms
        + result->signature_ms
        + result->verify_ms;


    return ret;
}



void benchmark_print(const benchmark_result_t *result) {

    printf("\n----- Benchmark -----\n");
    printf("Params      : %.3f ms\n", result->params_ms);
    printf("OT          : %.3f ms\n", result->ot_ms);
    printf("KeyGen      : %.3f ms\n", result->keygen_ms);
    printf("Signer      : %.3f ms\n", result->signer_ms);
    printf("Signature   : %.3f ms\n", result->signature_ms);
    printf("Verify      : %.3f ms\n", result->verify_ms);
    printf("---------------------\n");
    printf("Crypto      : %.3f ms\n", result->crypto_ms);
    printf("Total       : %.3f ms\n", result->total_ms);
    printf("---------------------\n");
}