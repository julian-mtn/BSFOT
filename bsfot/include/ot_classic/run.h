#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdint.h>

#include <relic/relic.h>
#include <relic/relic_pp.h>

#include "common/params.h"
#include "ot_classic/user.h"
#include "ot_classic/signer.h"
#include "common/keys.h"
#include "waters/verifier.h"
#include "common/benchmark.h"

int protocol_run(benchmark_result_t *result);

#endif