#ifndef KEYS_H
#define KEYS_H

#include "params.h"

/** Structure pour la clé secrète du signer */
typedef struct {
    g1_t bsk;     /* h_s^a */
    bn_t a;       
} signer_secret_key_t;

/** Structure pour la clé publique du signer */
typedef struct {
    g2_t bvk;     /* g2^a */
} signer_public_key_t;


/**
 * Génère la paire de clés du signer
 * 
 *   - clé publique : bvk = g2^a
 * 
 *   - clé secrète : bsk = hs^a
 *
 * @param params paramètres publics.
 * @param sk Structure de la clé secrète du signer
 * @param pk Structure de la clé publique du signer
 */
void keygen(public_params_t *params, signer_secret_key_t *sk, signer_public_key_t *pk);

#endif