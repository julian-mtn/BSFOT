#ifndef PARAMS_H
#define PARAMS_H

#include <relic/relic.h>
#include <relic/relic_pp.h>


typedef struct {

    /* Générateurs Waters */
    g1_t g1;
    g2_t g2;


    /* Paramètres Waters */
    g1_t u0;
    g1_t *u;
    g1_t hs;


    /* OT Classic ElGamal */
    bn_t *x;
    g1_t *h;


    /* OT Dual-Mode */
    g1_t dm_g;
    g1_t dm_h;
    g1_t dm_w;
    g1_t dm_w_tilde;


    /* Longueur du message */
    int l;

    /*mode du protocole*/
    int mode;

    int ord;


} public_params_t;


void init_params(public_params_t *params, int l, int mode);

void free_params(public_params_t *params);


#endif
