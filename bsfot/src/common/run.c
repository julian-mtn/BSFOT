#include "common/run.h"
#include "common/timer.h"
#include "common/message.h"
#include "common/config.h"

#include "dual/signer.h"
#include "dual/user.h"


static int protocol_init(protocol_ctx_t *ctx,  benchmark_result_t *result, int mode);
static int protocol_ot_phase(protocol_ctx_t *ctx, benchmark_result_t *result);
static int protocol_signer_phase(protocol_ctx_t *ctx, benchmark_result_t *result);
static int protocol_user_phase(protocol_ctx_t *ctx, benchmark_result_t *result);
static int protocol_verify_phase(protocol_ctx_t *ctx, benchmark_result_t *result);
static void protocol_cleanup(protocol_ctx_t *ctx);


int protocol_run(benchmark_result_t *result, int mode){

   int ret = -1;
   protocol_ctx_t ctx = {0};
   ctx.mode = mode;


   if (protocol_init(&ctx, result, mode) != 0)
      goto cleanup;

   if (protocol_ot_phase(&ctx, result) != 0)
      goto cleanup;

   if (protocol_signer_phase(&ctx, result) != 0)
      goto cleanup;

   if (protocol_user_phase(&ctx, result) != 0)
      goto cleanup;

   if (protocol_verify_phase(&ctx, result) != 0)
      goto cleanup;


   ret = 0;


   cleanup: protocol_cleanup(&ctx);

   return ret;
}


static int protocol_init(protocol_ctx_t *ctx, benchmark_result_t *result, int mode){

   if (core_init() != RLC_OK) {
      printf("\n[ERR] initialisation RELIC\n");
      return -1;
   }
   if (pc_param_set_any() != RLC_OK) {
      printf("[ERR] initialisation paramètres bilinéaires\n");
      core_clean();
      return -1;
   }
   pc_param_print();
   printf("\n[OK] RELIC initialisé\n");


   timer_start(&ctx->timer);
   init_params(&ctx->params, MESSAGE_LENGTH, mode);
   result->params_ms = timer_stop_ms(&ctx->timer);
   printf("[OK] Paramètres publics générés\n");

   generate_random_message(ctx->message, MESSAGE_LENGTH);
   
   return 0;
}

static int protocol_ot_phase(protocol_ctx_t *ctx, benchmark_result_t *result) {
    timer_start(&ctx->timer);

    switch(ctx->mode){
        /*
        * OT classique
        */
        case PROTOCOL_CLASSIC:{

            user_ot_init(&ctx->user,&ctx->params, ctx->message);
            ctx->user_init = 1;


            user_write_ot_keys_to_file(&ctx->user,CLASSIC_OT_KEYS_FILE);
            printf("[OK] Clés OT classiques écrites dans %s\n", CLASSIC_OT_KEYS_FILE);



            if(!signer_read_ot_request_from_file(&ctx->req,&ctx->params,CLASSIC_OT_KEYS_FILE)){
                printf("[ERR] Lecture requête OT classique\n");
                return -1;
            }
            ctx->req_init = 1;
            printf("[OK] Requête OT classique lue\n");

            break;
        }

        /*
        * Dual-Mode OT
        */
        case PROTOCOL_DUALMOD:{

            user_dualmod_ot_init(&ctx->dual_user, &ctx->params, ctx->message);
            ctx->dual_user_init = 1;

            user_write_dualmod_ot_keys_to_file(&ctx->dual_user);
            printf("[OK] Clés Dual-Mode OT écrites dans %s\n", DUALMOD_OT_KEYS_FILE);

            if(!signer_dualmod_read_ot_request_from_file( &ctx->dual_req, &ctx->params)){

                printf("[ERR] Lecture requête Dual-Mode OT\n");
                return -1;
            }
            ctx->dual_req_init = 1;
            printf("[OK] Requête Dual-Mode OT lue\n");

            break;
        }



        default:{
            printf("[ERR] Mode OT inconnu\n");
            return -1;
        }

    }
    result->ot_ms = timer_stop_ms(&ctx->timer);
    printf("[OK] Phase OT terminée\n");

    return 0;
}


static int protocol_signer_phase(protocol_ctx_t *ctx, benchmark_result_t *result){

    /*
     * Génération des clés du signer
     */
    timer_start(&ctx->timer);
    keygen(&ctx->params, &ctx->sk, &ctx->pk);
    result->keygen_ms = timer_stop_ms(&ctx->timer);
    printf("[OK] Clé secrète et publique du signer générées\n");

    /*
     * Calcul de la réponse OT
     */
    timer_start(&ctx->timer);

    switch(ctx->mode){

        /*
         * OT classique
         */
        case 0:{

            signer_compute_response(&ctx->resp_signer, &ctx->req, &ctx->params, &ctx->sk);
            result->signer_ms = timer_stop_ms(&ctx->timer);
            ctx->resp_signer_init = 1;

            signer_write_response_to_file(&ctx->resp_signer, CLASSIC_SIGNER_RESPONSE_FILE);
            printf("[OK] Réponse signer OT classique écrite\n");

            break;
        }

        /*
         * Dual-Mode OT
         */
        case 1:{

            signer_dualmod_compute_response(&ctx->dual_resp_signer, &ctx->dual_req, &ctx->params, &ctx->sk);
            result->signer_ms = timer_stop_ms(&ctx->timer);
            ctx->dual_resp_signer_init = 1;

            signer_dualmod_write_response_to_file(&ctx->dual_resp_signer, DUALMOD_SIGNER_RESPONSE_FILE);
            printf("[OK] Réponse signer Dual-Mode écrite\n");

            break;
        }

        default:{

            printf("[ERR] Mode signer inconnu\n");
            timer_stop_ms(&ctx->timer);

            return -1;
        }

    }

    return 0;
}


static int protocol_user_phase(protocol_ctx_t *ctx, benchmark_result_t *result){

    switch (ctx->mode){

        case 0:

            if (!user_read_signer_response_from_file(&ctx->resp_user, &ctx->params)){

                printf("[ERR] Erreur lecture de la réponse du signer\n");
                return -1;
            }

            ctx->resp_user_init = 1;
            printf("[OK] Réponse du signer lue par l'utilisateur\n");

            timer_start(&ctx->timer);
            user_compute_signature(&ctx->sig, &ctx->user, &ctx->resp_user, &ctx->params);
            ctx->sig_init = 1;
            result->signature_ms = timer_stop_ms(&ctx->timer);

            break;

        case 1:

            if (!user_dualmod_read_signer_response_from_file(&ctx->dual_resp_user)) {
                printf("[ERR] Erreur lecture de la réponse du signer\n");
                return -1;
            }
            ctx->dual_resp_user_init = 1;
            printf("[OK] Réponse du signer lue par l'utilisateur\n");

            timer_start(&ctx->timer);
            user_dualmod_compute_signature(&ctx->sig, &ctx->dual_user, &ctx->dual_resp_user, &ctx->params);
            ctx->sig_init = 1;
            result->signature_ms = timer_stop_ms(&ctx->timer);

            break;

        default: return -1;
    }

    printf("[OK] Signature finale calculée par l'utilisateur\n");

    switch(ctx->mode){

        case PROTOCOL_CLASSIC:

            user_write_signature_to_file( &ctx->sig, CLASSIC_SIGNATURE_FILE);
            printf("[OK] Signature classique écrite\n");

            break;

        case PROTOCOL_DUALMOD:

            user_write_dualmod_signature_to_file(&ctx->sig, DUALMOD_SIGNATURE_FILE);
            printf("[OK] Signature Dual-Mode écrite\n");

            break;


        default: return -1;
    }

    return 0;
}



static int protocol_verify_phase(protocol_ctx_t *ctx, benchmark_result_t *result){

    int valid;

    switch (ctx->mode){

        case PROTOCOL_CLASSIC:

            if (!verifier_read_classic_signature_from_file(&ctx->sig_verif, CLASSIC_SIGNATURE_FILE)){

                printf("[ERR] Erreur lecture de la signature classique\n");
                return -1;
            }
            break;

        case PROTOCOL_DUALMOD:

            if (!verifier_read_dualmod_signature_from_file(&ctx->sig_verif, DUALMOD_SIGNATURE_FILE)){

                printf("[ERR] Erreur lecture de la signature Dual-Mode\n");
                return -1;
            }
            break;

        default: return -1;
    }
    ctx->sig_verif_init = 1;

    timer_start(&ctx->timer);
    switch (ctx->mode){

        case PROTOCOL_CLASSIC:

            valid = verifier_check_classic_signature(&ctx->sig_verif, &ctx->params, &ctx->pk, ctx->message);
            break;

        case PROTOCOL_DUALMOD:

            valid = verifier_check_dualmod_signature(&ctx->sig_verif, &ctx->params, &ctx->pk, ctx->message);
            break;

        default:
            timer_stop_ms(&ctx->timer);
            return -1;
    }

    result->verify_ms = timer_stop_ms(&ctx->timer);

    if(valid){
        printf("\n[--- Signature Valide ---]\n");
    }
    else{
        printf("\n[--- Signature Invalide ---]\n");
    }
    return 0;
}

static void protocol_cleanup(protocol_ctx_t *ctx){

    if (ctx->sig_verif_init) {
        user_signature_free(&ctx->sig_verif);
    }

    if (ctx->sig_init){
        user_signature_free(&ctx->sig);
    }

    switch (ctx->mode){

        case PROTOCOL_CLASSIC:

            if (ctx->req_init)
                signer_ot_request_free(&ctx->req);

            if (ctx->resp_signer_init)
                signer_ot_response_free(&ctx->resp_signer);

            if (ctx->resp_user_init)
                signer_ot_response_free(&ctx->resp_user);

            if (ctx->user_init)
                user_state_free(&ctx->user);

            break;

        case PROTOCOL_DUALMOD:

            if (ctx->dual_req_init)
                signer_dualmod_ot_request_free(&ctx->dual_req);

            if (ctx->dual_resp_signer_init)
                signer_dualmod_response_free(&ctx->dual_resp_signer);

            if (ctx->dual_resp_user_init)
                signer_dualmod_response_free(&ctx->dual_resp_user);

            if (ctx->dual_user_init)
                user_dualmod_state_free(&ctx->dual_user);

            break;
    }

    free_params(&ctx->params);
    core_clean();
}
