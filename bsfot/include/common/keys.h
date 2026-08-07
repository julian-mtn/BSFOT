#ifndef KEYS_H
#define KEYS_H

#include "common/params.h"


/*
 * Clé secrète du signer
 */
typedef struct {

    /*
     * Mode classique :
     * bsk = hs^a
     */
    g1_t bsk;

    /*
     * Exposant secret classique
     */
    bn_t a;



    /*
     * Mode Dual-Mode :
     * clé secrète utilisée par le signer
     */
    g1_t dm_bsk;


} signer_secret_key_t;



/*
 * Clé publique du signer
 */
typedef struct {

    /*
     * Mode classique :
     * bvk = g2^a
     */
    g2_t bvk;



    /*
     * Mode Dual-Mode
     */
    g2_t dm_bvk;


} signer_public_key_t;



/**
 * Génère les clés du signer selon le mode.
 *
 * mode 0 :
 *      bsk = hs^a
 *      bvk = g2^a
 *
 * mode 1 :
 *      génération des paramètres Dual-Mode
 *
 */
void keygen(
        public_params_t *params,
        signer_secret_key_t *sk,
        signer_public_key_t *pk
);



#endif
