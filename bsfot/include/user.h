#ifndef USER_H
#define USER_H

#include <stdint.h>
#include <stdio.h>
#include <relic/relic.h>

#include "params.h"
#include "config.h"
#include "signer.h"

/**
 * État local de l'user durant le protocole de signature aveugle
 *
 * Cette structure contient les informations nécessaires à la première phase
 * du protocole (création des paires OT) ainsi que les données qui seront
 * réutilisées lors de la dérivation finale de la signature
 */
typedef struct {

    /** Longueur du message (en bits) */
    int l;

    /** Message binaire à signer */
    uint8_t *message;

    /** Exposants aléatoires y_i choisis par l'user */
    bn_t *y;

    /** Première composante des clés d'évaluation OT */
    g1_t *ek0;

    /** Seconde composante des clés d'évaluation OT */
    g1_t *ek1;

} user_state_t;

/**
 * Signature finale du schéma calculé par user
 *
 * sigma = (F(M), sigma1_hat, sigma2_hat)
 */
typedef struct {

    /** Hachage Waters du message F(M) */
    g1_t fm;

    /** Composante sigma1 chiffrée/randomisée*/
    g1_t sigma1;

    /** Composante sigma2 chiffrée/randomisée */
    g2_t sigma2;

    /** Composante \hat{\sigma}'_2 pour le dualmode */
    g1_t sigma2_prime;

} signature_t;

/**
 * Initialise l'état de l'user et génère les paires OT
 *
 * Pour chaque bit M_i du message :
 *   - choisit un exposant aléatoire y_i ;
 *   - calcule les deux clés d'évaluation
 *       ek_{i,M_i} = g1^{y_i}
 *       ek_{i,1-M_i} = g1^{x_i-y_i}
 *
 *
 * @param st      État user à initialiser
 * @param params  Paramètres publics du protocole
 * @param message Message binaire à signer (tableau de longueur params->l)
 */
void user_ot_init(user_state_t *st,const public_params_t *params,const uint8_t *message);



/**
 * Écrit les clés d'évaluation OT dans un fichier
 *
 *
 * @param st État user contenant les clés OT
 */
void user_write_ot_keys_to_file(const user_state_t *st, const char *filename);

/** 
 * Lit la réponse du signer depuis le fichier SIGNER_RESPONSE_FILE et la stocke dans la structure resp
 *
 * 
 * @param resp Structure recevant la réponse lue   
 * @param params Paramètres publics du protocole
 * @return 1 si la lecture est réussie, 0 sinon
 */
int user_read_signer_response_from_file(signer_response_t *resp,const public_params_t *params);


/**
 * Calcule la signature finale à partir de la réponse du signer
 *
 * sigma1 = Π_i (ct_i,M_i / ct0^{y_i})
 * sigma1_hat = sigma1 * F(M)^{t'}
 * sigma2_hat = sigma2 * g2^{t'}
 * @param sig Structure recevant la signature finale
 * @param st État user contenant les clés OT et le message
 * @param resp Réponse du signer contenant les chiffrés ct_i,M_i et ct0, sigma2
 * @param params Paramètres publics du protocole
 */
void user_compute_signature(signature_t *sig, const user_state_t *st, const signer_response_t *resp, public_params_t *params);


/** 
 * Écrit la signature dans un fichier
 *
 * @param sig Signature à écrire
 */
void user_write_signature_to_file(signature_t *sig, const char *filename);

/**
 * Libère une signature.
 */
void user_signature_free(signature_t *sig);

/**
 * Libère toute la mémoire associée à un état user
 *
 * @param st État user à libérer
 */
void user_state_free(user_state_t *st);

#endif