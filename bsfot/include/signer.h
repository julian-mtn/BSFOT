#ifndef SIGNER_H
#define SIGNER_H

#include <relic/relic.h>

#include "params.h"
#include "keys.h"
#include "config.h"


/** Réponse OT envoyée par user au signer */
typedef struct {
    /** Longueur du message*/
    int l;

    /** Première composante des clés OT*/
    g1_t *ek0;

    /** Seconde composante des clés OT */
    g1_t *ek1;
} ot_request_t;


/** Réponse du signer 
 * 
 * l est la longueur du message, ct0_i et ct1_i sont les deux composantes
 * du chiffrement OT pour chaque bit du message, ct0 est g1^r et sigma2 est g2^t
 */
typedef struct {
    /** Longueur du message*/
    int l;

    /** Chiffrements ct_{i,0}*/
    g1_t *ct0_i;

    /** Chiffrements ct_{i,1}*/
    g1_t *ct1_i;

    /** ct0 = g1^r */
    g1_t ct0;

    /** sigma2 = g2^t*/
    g2_t sigma2;
} signer_response_t;


/**
 * Lit les clés OT envoyées par user dans un fichier et vérifie leur validité
 *
 * Pour chaque i, la fonction vérifie que :
 *      ek0_i * ek1_i = h_i.
 *
 * @param req Structure recevant les clés lues
 * @param params Paramètres publics
 *
 * @return 1 si les clés sont valides, 0 sinon
 */
int signer_read_ot_request_from_file(ot_request_t *req,const public_params_t *params);


/**
 * Calcule la réponse du signer
 *
 * Cette fonction génère les valeurs aléatoires du protocole et calcule
 * les composantes (ct_{i,0}, ct_{i,1}), ct0 et sigma2
 *
 * @param resp Structure recevant la réponse du signer
 * @param req Clés OT envoyées par user
 * @param params Paramètres publics
 * @param sk Clé secrète du signer
 */
void signer_compute_response(signer_response_t *resp,
                     const ot_request_t *req,
                     const public_params_t *params,
                     const signer_secret_key_t *sk);

/**
 * Écrit la réponse du signer dans un fichier.
 *
 * Format :
 *  - longueur l
 *  - (ct0_i, ct1_i) pour chaque i
 *  - ct0
 *  - sigma2
 * @param resp Structure contenant la réponse du signer
 */
void signer_write_response_to_file(signer_response_t *resp);

void signer_ot_request_free(ot_request_t *req);
void signer_ot_response_free(signer_response_t *resp);

#endif 