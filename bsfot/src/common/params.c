#include "common/params.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void init_params(public_params_t *params, int l){

    params->l = l;

    g1_null(params->g1);
    g2_null(params->g2);
    g1_null(params->u0);
    g1_null(params->hs);

    g1_new(params->g1);
    g2_new(params->g2);
    g1_new(params->u0);
    g1_new(params->hs);

    //éléments aléatoires dans les groupes G1 et G2
    g1_rand(params->g1);
    g2_rand(params->g2);
    g1_rand(params->u0);

    params->u = malloc(l * sizeof(g1_t)); //paramètres de Waters
    params->h = malloc(l * sizeof(g1_t)); 
    params->x = malloc(l * sizeof(bn_t)); //exposants aléatoires

    bn_t order;
    bn_null(order);
    bn_new(order);
    g1_get_ord(order);

    g1_set_infty(params->hs); //infty : élément neutre du groupe

    for (int i = 0; i < l; i++) {

        g1_null(params->u[i]);
        g1_null(params->h[i]);
        bn_null(params->x[i]);

        g1_new(params->u[i]);
        g1_new(params->h[i]);
        bn_new(params->x[i]);

        g1_rand(params->u[i]); //valeurs aléatoires 
        
        bn_rand_mod(params->x[i], order); //exposants aléatoires

        g1_mul(params->h[i], params->g1, params->x[i]); //h_i = g1^xi
        g1_add(params->hs, params->hs, params->h[i]); //hs = somme des h_i

    }

    bn_free(order);
}

void free_params(public_params_t *params){

    for (int i = 0; i < params->l; i++) {
        g1_free(params->u[i]);
        g1_free(params->h[i]);
        bn_free(params->x[i]);
    }

    free(params->u);
    free(params->h);
    free(params->x);

    g1_free(params->g1);
    g2_free(params->g2);
    g1_free(params->u0);
    g1_free(params->hs);
}
