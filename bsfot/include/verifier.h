#ifndef VERIFIER_H
#define VERIFIER_H

#include <stdio.h>
#include <stdint.h>

#include <relic/relic.h>
#include <relic/relic_pp.h>

#include "params.h"
#include "keys.h"
#include "user.h"


/**
 * Lit une signature classique depuis un fichier
 *
 * Format attendu :
 * - F(M)
 * - sigma1
 * - sigma2
 *
 * @param sig Signature à remplir
 * @param filename Fichier contenant la signature
 *
 * @return 1 en cas de succès, 0 sinon
 */
int verifier_read_classic_signature_from_file(signature_t *sig, const char *filename);


/**
 * Lit une signature dual-mode depuis un fichier
 *
 * Format attendu :
 * - F(M)
 * - sigma1_hat
 * - sigma2_hat
 * - sigma2_prime_hat
 *
 * @param sig Signature à remplir
 * @param filename Fichier contenant la signature
 *
 * @return 1 en cas de succès, 0 sinon
 */
int verifier_read_dualmod_signature_from_file(signature_t *sig, const char *filename);


/**
 * Vérifie une signature Waters
 *
 * Vérifie :
 *
 * e(h_s, bvk) * e(F(M), sigma2_hat) == e(sigma1_hat, g2)
 *
 * Cette vérification est commune aux deux protocoles :
 * - classique
 * - dual-mode
 *
 * @param sig Signature à vérifier
 * @param params Paramètres publics
 * @param pk Clé publique du signer
 * @param message Message signé
 *
 * @return 1 si la signature est valide, 0 sinon
 */
int verifier_check_classic_signature( const signature_t *sig, public_params_t *params, const signer_public_key_t *pk, uint8_t *message);


int verifier_check_dualmod_signature(const signature_t *sig, public_params_t *params, const signer_public_key_t *pk, uint8_t *message);

#endif