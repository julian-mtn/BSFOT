#ifndef SIGNER_DUALMODE_H
#define SIGNER_DUALMODE_H

#include <relic/relic.h>

#include "params.h"
#include "keys.h"


typedef struct {

    int l;

    /* ek_{i,0} */
    g1_t *ek0;

    /* \tilde{ek}_{i,0} */
    g1_t *tek0;

} dualmod_ot_request_t;

typedef struct {

    int l;

    /* C_{i,0} */
    g1_t *c0_i;

    /* D_{i,0} */
    g1_t *d0_i;

    /* C_{i,1} */
    g1_t *c1_i;

    /* D_{i,1} */
    g1_t *d1_i;

    /* sigma2 = g2^t */
    g2_t sigma2;

    /* sigma2_prime = g1^t */
    g1_t sigma2_prime;

} dualmod_signer_response_t;

int signer_dualmod_read_ot_request_from_file( dualmod_ot_request_t *req, const public_params_t *params);

void signer_dualmod_ot_request_free(dualmod_ot_request_t *req);

void signer_dualmod_compute_response(dualmod_signer_response_t *resp, const dualmod_ot_request_t *req, const public_params_t *params, const signer_secret_key_t *sk);

void signer_dualmod_write_response_to_file(dualmod_signer_response_t *resp, const char *filename);

void signer_dualmod_response_free(dualmod_signer_response_t *resp);


#endif
