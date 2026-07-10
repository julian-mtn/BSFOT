#ifndef PARAMS_H
#define PARAMS_H

#include <relic/relic.h>
#include <relic/relic_pp.h>

typedef struct {
    /* Générateurs */
    g1_t g1;
    g2_t g2;

    /* Paramètres de Waters */
    g1_t u0;
    g1_t *u;

    /*x_i*/
    bn_t *x;
    /* h_i = g1^xi */
    g1_t *h;

    /* hs = produit des h_i */
    g1_t hs;

    /* Longueur du vecteur */
    int l;
} public_params_t;

/**
 * Initialise les paramètres publics.
 *
 * @param params Structure contenant les paramètres publics.
 * @param l Longueur du vecteur.
 */
void init_params(public_params_t *params, int l);

/**
 * Libère la mémoire allouée aux paramètres publics.
 *
 * @param params Structure contenant les paramètres publics.
 */
void free_params(public_params_t *params);

#endif 