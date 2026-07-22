#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdint.h>

#include <relic/relic.h>
#include <relic/relic_pp.h>

#include "params.h"
#include "user.h"
#include "signer.h"
#include "keys.h"
#include "verifier.h"
#include "benchmark.h"

int protocol_run(benchmark_result_t *result);

#endif