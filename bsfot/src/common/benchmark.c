#include "common/benchmark.h"
#include "common/timer.h"
#include "common/run.h"
#include "common/config.h"

#include <stdio.h>


int benchmark_run(benchmark_result_t *result, int mode, int MESSAGE_LENGTH) {

    benchmark_result_t tmp;

    double total_ms = 0.0;
    double crypto_ms = 0.0;
    double params_ms = 0.0;
    double ot_ms = 0.0;
    double keygen_ms = 0.0;
    double signer_ms = 0.0;
    double signature_ms = 0.0;
    double verify_ms = 0.0;

    int ret = 0;


    for (int i = 0; i < NB_BENCHMARKS; i++) {

        benchmark_timer_t timer;

        timer_start(&timer);

        ret = protocol_run(&tmp, mode, MESSAGE_LENGTH);

        total_ms += timer_stop_ms(&timer);


        params_ms += tmp.params_ms;
        ot_ms += tmp.ot_ms;
        keygen_ms += tmp.keygen_ms;
        signer_ms += tmp.signer_ms;
        signature_ms += tmp.signature_ms;
        verify_ms += tmp.verify_ms;


        crypto_ms +=
              tmp.params_ms
            + tmp.ot_ms
            + tmp.keygen_ms
            + tmp.signer_ms
            + tmp.signature_ms
            + tmp.verify_ms;
    }


    result->params_ms = params_ms / NB_BENCHMARKS;
    result->ot_ms = ot_ms / NB_BENCHMARKS;
    result->keygen_ms = keygen_ms / NB_BENCHMARKS;
    result->signer_ms = signer_ms / NB_BENCHMARKS;
    result->signature_ms = signature_ms / NB_BENCHMARKS;
    result->verify_ms = verify_ms / NB_BENCHMARKS;

    result->crypto_ms = crypto_ms / NB_BENCHMARKS;
    result->total_ms = total_ms / NB_BENCHMARKS;


    return ret;
}



void benchmark_print(const benchmark_result_t *result, int mode, int MESSAGE_LENGTH) {

    printf("\n----- Benchmark -----\n");
    printf("Message     : %d bits\n", MESSAGE_LENGTH);
    printf("Iterations  : %d\n", NB_BENCHMARKS);
    printf("Mode        : %s\n", (mode == 0) ? "Classic OT" : "Dual-Mode OT");
    printf("---------------------\n");
    printf("BS_Keygen      : %.3f ms\n", result->keygen_ms);
    printf("BS_Params      : %.3f ms\n", result->params_ms);
    printf("BS_User     : %.3f ms\n", result->ot_ms);
    printf("BS_Signer   : %.3f ms\n", result->signer_ms);
    printf("BS_Derive   : %.3f ms\n", result->signature_ms);
    printf("BS_Verify   : %.3f ms\n", result->verify_ms);
    printf("---------------------\n");
    printf("crypto_time : %.3f ms\n", result->crypto_ms);
    printf("Total       : %.3f ms\n", result->total_ms);
    printf("---------------------\n");
}