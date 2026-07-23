#ifndef BENCHMARK_H
#define BENCHMARK_H


typedef struct {

    double params_ms;
    double ot_ms;
    double keygen_ms;
    double signer_ms;
    double signature_ms;
    double verify_ms;

    double crypto_ms;
    double total_ms;

} benchmark_result_t;


int benchmark_run(benchmark_result_t *result, int mode);

void benchmark_print(const benchmark_result_t *result);


#endif