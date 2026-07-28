#ifndef USER_DUALMOD_H
#define USER_DUALMOD_H

#include "params.h"
#include "signer_dualmode.h"
#include "waters.h"
#include "user.h"

typedef struct {

    int l;

    uint8_t *message;

    bn_t *y;

    g1_t *ek0;
    g1_t *ek1;

    g1_t *tek0;
    g1_t *tek1;

} dualmod_user_state_t;



void user_dualmod_ot_init(dualmod_user_state_t *st, const public_params_t *params, const uint8_t *message);

int user_dualmod_read_signer_response_from_file(dualmod_signer_response_t *resp);

void user_dualmod_compute_signature(signature_t *sig, const dualmod_user_state_t *st, const dualmod_signer_response_t *resp, public_params_t *params);

void user_dualmod_state_free(dualmod_user_state_t *st);

void user_write_dualmod_ot_keys_to_file(const dualmod_user_state_t *st);

int user_write_dualmod_signature_to_file(const signature_t *sig, const char *filename);

#endif
