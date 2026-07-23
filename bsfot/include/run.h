#ifndef RUN_H
#define RUN_H

#include <stdio.h>
#include <stdint.h>

#include <relic/relic.h>
#include <relic/relic_pp.h>

#include "params.h"
#include "user.h"
#include "signer.h"
#include "keys.h"
#include "verifier.h"
#include "benchmark.h"
#include "timer.h"


/**
 * Contexte d'exécution du protocole.
 *
 * Contient tous les paramètres, clés, états intermédiaires
 * et objets cryptographiques utilisés pendant l'exécution
 * du protocole.
 */
typedef struct {

    public_params_t params;             /** Paramètres publics du protocole */
    uint8_t message[MESSAGE_LENGTH];    /** Message signé */

    user_state_t user;             /** État de l'utilisateur */
    ot_request_t req;              /** Requête OT envoyée au signer */

    signer_secret_key_t sk;        /** Clé secrète du signer */
    signer_public_key_t pk;        /** Clé publique du signer */

    signer_response_t resp_signer; /** Réponse calculée par le signer */
    signer_response_t resp_user;   /** Réponse reçue par l'utilisateur */

    signature_t sig;               /** Signature finale générée */
    signature_t sig_verif;         /** Signature utilisée pour la vérification */

    benchmark_timer_t timer;       /** Timer utilisé pour les mesures */

    int user_init;                 /** Indique si l'état utilisateur est initialisé */
    int req_init;                  /** Indique si la requête OT est initialisée */
    int resp_signer_init;          /** Indique si la réponse du signer est initialisée */
    int resp_user_init;            /** Indique si la réponse utilisateur est initialisée */
    int sig_init;                  /** Indique si la signature est initialisée */
    int sig_verif_init;            /** Indique si la signature de vérification est initialisée */

    int mode;                      /** Version du protocole utilisée */

} protocol_ctx_t;


/**
 * @brief Exécute le protocole complet.
 *
 * Initialise les paramètres cryptographiques, exécute les différentes
 * phases du protocole (OT, génération de clés, signature et vérification),
 * puis libère les ressources utilisées.
 *
 * @param result Structure contenant les résultats des benchmarks.
 * @param mode Version du protocole à exécuter.
 *
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int protocol_run(benchmark_result_t *result, int mode);

#endif
