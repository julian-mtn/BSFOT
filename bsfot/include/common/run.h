#ifndef RUN_H
#define RUN_H

#include <stdio.h>
#include <stdint.h>

#include <relic/relic.h>
#include <relic/relic_pp.h>

#include "common/params.h"

#include "classic/user.h"
#include "dual/user.h"

#include "classic/signer.h"
#include "dual/signer.h"

#include "common/keys.h"
#include "common/verifier.h"

#include "common/benchmark.h"
#include "common/timer.h"



/**
 * Contexte d'exécution du protocole.
 *
 * Contient tous les paramètres, clés, états intermédiaires
 * et objets cryptographiques utilisés pendant l'exécution
 * du protocole.
 */
typedef struct {


    /* Paramètres publics */
    public_params_t params;


    /* Message à signer */
    uint8_t * message;



    /*
     * ============================
     * OT classique
     * ============================
     */

    user_state_t user;

    ot_request_t req;

    signer_response_t resp_signer;

    signer_response_t resp_user;



    /*
     * ============================
     * Dual-Mode OT
     * ============================
     */

    dualmod_user_state_t dual_user;

    dualmod_ot_request_t dual_req;

    dualmod_signer_response_t dual_resp_signer;

    dualmod_signer_response_t dual_resp_user;



    /*
     * ============================
     * Clés du signer
     * ============================
     */

    signer_secret_key_t sk;

    signer_public_key_t pk;



    /*
     * ============================
     * Signatures
     * ============================
     */

    signature_t sig;

    signature_t sig_verif;



    /*
     * ============================
     * Benchmark
     * ============================
     */

    benchmark_timer_t timer;



    /*
     * ============================
     * Flags d'initialisation
     * ============================
     */


    /* OT classique */
    int user_init;

    int req_init;

    int resp_signer_init;

    int resp_user_init;



    /* Dual-mode OT */
    int dual_user_init;

    int dual_req_init;

    int dual_resp_signer_init;

    int dual_resp_user_init;



    /* Signature */
    int sig_init;

    int sig_verif_init;



    /*
     * Mode du protocole
     *
     * 0 : OT classique
     * 1 : Dual-Mode OT
     */
    int mode;


} protocol_ctx_t;



/**
 * Exécute le protocole complet.
 *
 * Initialise les paramètres cryptographiques,
 * exécute les phases OT, signer, utilisateur,
 * vérification, puis libère les ressources.
 *
 * @param result Résultats des benchmarks.
 * @param mode Version du protocole.
 *
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int protocol_run(benchmark_result_t *result, int mode, int MESSAGE_LENGTH);


#endif
