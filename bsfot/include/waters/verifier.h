#ifndef VERIFIER_H
#define VERIFIER_H

#include <stdio.h>
#include <stdint.h>

#include <relic/relic.h>
#include <relic/relic_pp.h>

#include "common/params.h"
#include "common/keys.h"
#include "ot_classic/user.h"


/**
 * Lit une signature depuis un fichier
 *
 * @param sig Signature à remplir
 *
 * @return 1 en cas de succès, 0 sinon
 */
int verifier_read_signature_from_file(signature_t *sig);


/**
 * Vérifie une signature Waters
 *
 * Vérifie :
 *
 * e(h_s, bvk) * e(F(M), sigma2_hat) == e(sigma1_hat, g2)
 *
 * @param sig Signature à vérifier
 * @param params Paramètres publics
 * @param pk Clé publique du signer
 * @param message Message signé
 *
 * @return 1 si la signature est valide, 0 sinon
 */
int verifier_check_signature(const signature_t *sig,
                             public_params_t *params,
                             const signer_public_key_t *pk,
                             uint8_t *message);



#endif