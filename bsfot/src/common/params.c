#include "common/params.h"
#include <stdio.h>
#include <stdlib.h>


void init_params(public_params_t *params, int l, int mode){

    params->l = l;
    params->mode = mode;

    /* Générateurs communs */
    g1_null(params->g1);
    g2_null(params->g2);

    g1_new(params->g1);
    g2_new(params->g2);

    g1_rand(params->g1);
    g2_rand(params->g2);

    /* Paramètres Waters */
    g1_null(params->u0);
    g1_null(params->hs);

    g1_new(params->u0);
    g1_new(params->hs);

    g1_rand(params->u0);


    /*
     * hs doit exister dans les deux modes
     */
    g1_rand(params->hs);

    params->u = malloc(l * sizeof(g1_t));

    /* Paramètres OT Classic */
    if (mode == 0){
        params->x = malloc(l * sizeof(bn_t));
        params->h = malloc(l * sizeof(g1_t));
    }

    bn_t order;
    bn_null(order);
    bn_new(order);

    g1_get_ord(order);

    for (int i = 0; i < l; i++){

        /* Waters */
        g1_null(params->u[i]);
        g1_new(params->u[i]);

        g1_rand(params->u[i]);

        /*Classic OT*/
        if (mode == 0){

            g1_null(params->h[i]);
            bn_null(params->x[i]);

            g1_new(params->h[i]);
            bn_new(params->x[i]);

            bn_rand_mod(params->x[i], order);

            /* h_i = g1^xi */
            g1_mul(params->h[i],params->g1,params->x[i]);


            /*hs = produit des h_i */
            if(i == 0){
                g1_set_infty(params->hs);
            }

            g1_add(params->hs, params->hs, params->h[i]);
        }
    }

    /*Paramètres Dual-Mode OT */

    if (mode == 1){

        g1_null(params->dm_g);
        g1_null(params->dm_h);
        g1_null(params->dm_w);
        g1_null(params->dm_w_tilde);


        g1_new(params->dm_g);
        g1_new(params->dm_h);
        g1_new(params->dm_w);
        g1_new(params->dm_w_tilde);


        bn_t eta;
        bn_null(eta);
        bn_new(eta);

        /* générateur */
        g1_get_gen(params->dm_g);

        /* eta <- Zp */
        do {
            bn_rand_mod(eta, order);
        } while (bn_is_zero(eta));

        /* dm_h = eta * dm_g */
        g1_mul(params->dm_h, params->dm_g, eta);

        bn_t r;
        bn_null(r);
        bn_new(r);

        do {
            bn_rand_mod(r, order);
        } while (bn_is_zero(r));

        /* w = g^r */
        g1_mul(params->dm_w, params->dm_g, r);

        /* w_tilde = h^r = (g^eta)^r */
        g1_mul(params->dm_w_tilde, params->dm_h, r);

        bn_free(r);
        bn_free(eta);
    }


    bn_free(order);
}


void free_params(public_params_t *params){

    for (int i = 0; i < params->l; i++){
        g1_free(params->u[i]);
    }

    free(params->u);

    if (params->mode == 0){

        for (int i = 0; i < params->l; i++) {
            g1_free(params->h[i]);
            bn_free(params->x[i]);
        }

        free(params->h);
        free(params->x);
    }

    if (params->mode == 1){

        g1_free(params->dm_g);
        g1_free(params->dm_h);
        g1_free(params->dm_w);
        g1_free(params->dm_w_tilde);
    }

    g1_free(params->g1);
    g2_free(params->g2);
    g1_free(params->u0);
    g1_free(params->hs);
}
