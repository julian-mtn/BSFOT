
#include "common/keys.h"
#include <stdio.h>


void keygen(public_params_t *params, signer_secret_key_t *sk, signer_public_key_t *pk){

    bn_t order;
    bn_null(order);
    bn_new(order);

    g1_get_ord(order);

    g1_null(sk->bsk);
    bn_null(sk->a);
    g2_null(pk->bvk);
    g1_new(sk->bsk);
    bn_new(sk->a);
    g2_new(pk->bvk);

    /* a <- Zp */
    bn_rand_mod(sk->a, order);

    /* bvk = g2^a */
    g2_mul(pk->bvk, params->g2, sk->a);

    /* bsk = hs^a */
    g1_mul(sk->bsk,params->hs,sk->a);

    /* init pour Dual-Mode */
    if(params->mode == 1){

        g1_null(sk->dm_bsk);
        g1_new(sk->dm_bsk);

        g2_null(pk->dm_bvk);
        g2_new(pk->dm_bvk);

        g1_copy(sk->dm_bsk,params->dm_w);

        g2_rand(pk->dm_bvk);

    }

    bn_free(order);

}