#include "keys.h"
#include <stdio.h>




void keygen(public_params_t *params, signer_secret_key_t *sk, signer_public_key_t *pk) {
    
    /*init*/
    g1_null(sk->bsk);
    bn_null(sk->a);
    g2_null(pk->bvk);
    g1_new(sk->bsk);
    bn_new(sk->a);
    g2_new(pk->bvk);


    /*a aléatoire dans Z_p*/
    bn_t order;
    bn_null(order);
    bn_new(order);
    g1_get_ord(order);
    bn_rand_mod(sk->a, order); /* a <- Z_p */

    g2_mul(pk->bvk,params->g2,sk->a);
    g1_mul(sk->bsk,params->hs,sk->a);

    bn_free(order);
}