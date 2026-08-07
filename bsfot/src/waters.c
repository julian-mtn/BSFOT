#include "waters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "params.h"

void waters_hash(g1_t result, g1_t *u, int l, uint8_t *m){
    
    g1_t tmp;
    g1_null(tmp);
    g1_new(tmp);

    bn_t bit;
    bn_null(bit);
    bn_new(bit);

    g1_set_infty(result);

    for (int i = 0; i < l; i++) {

        if (m[i] == 0)
            continue;

        bn_set_dig(bit, m[i]);

        g1_mul(tmp, u[i], bit);

        g1_add(result, result, tmp);
    }

    g1_free(tmp);
    bn_free(bit);
}


void waters_hash_dualmode(g1_t result, g1_t *u, int l, uint8_t *m, public_params_t *params){
    
    g1_t tmp;
    g1_null(tmp);
    g1_new(tmp);

    bn_t bit;
    bn_null(bit);
    bn_new(bit);

    g1_set_infty(result);

    for (int i = 0; i < l; i++) {

        if (m[i] == 0)
            continue;

        bn_set_dig(bit, m[i]);

        g1_mul(tmp, u[i], bit);

        g1_add(result, result, tmp);
    }

    g1_add(result, result, params->u0);

    g1_free(tmp);
    bn_free(bit);


}