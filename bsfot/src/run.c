#include "run.h"
#include "timer.h"
#include "message.h"
#include "config.h"

static int protocol_init(protocol_ctx_t *ctx,  benchmark_result_t *result);
static int protocol_ot_phase(protocol_ctx_t *ctx, benchmark_result_t *result);
static int protocol_signer_phase(protocol_ctx_t *ctx, benchmark_result_t *result);
static int protocol_user_phase(protocol_ctx_t *ctx, benchmark_result_t *result);
static int protocol_verify_phase(protocol_ctx_t *ctx, benchmark_result_t *result);
static void protocol_cleanup(protocol_ctx_t *ctx);


int protocol_run(benchmark_result_t *result, int mode){

   int ret = -1;
   protocol_ctx_t ctx = {0};
   ctx.mode = mode;


   if (protocol_init(&ctx, result) != 0)
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


static int protocol_init(protocol_ctx_t *ctx, benchmark_result_t *result){

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
   init_params(&ctx->params, MESSAGE_LENGTH);
   result->params_ms = timer_stop_ms(&ctx->timer);
   printf("[OK] Paramètres publics générés -> [MESSAGE_LENGTH : %d bits]\n", MESSAGE_LENGTH);

   generate_random_message(ctx->message, MESSAGE_LENGTH);
   
   return 0;
}


static int protocol_ot_phase(protocol_ctx_t *ctx, benchmark_result_t *result){

   timer_start(&ctx->timer);
   user_ot_init(&ctx->user,&ctx->params, ctx->message);
   result->ot_ms = timer_stop_ms(&ctx->timer);

   ctx->user_init = 1;
   printf("[OK] Paires OT générées\n");


   user_write_ot_keys_to_file(&ctx->user);

   printf("[OK] Clés OT écrites dans %s\n",OT_KEYS_FILE);


   if (!signer_read_ot_request_from_file(&ctx->req,&ctx->params)) {
      printf("[ERR] Erreur lecture des clés OT\n");
      return -1;
   }

   ctx->req_init = 1;
   printf("[OK] Clés OT lues par le signer\n");


   return 0;
}

static int protocol_signer_phase(protocol_ctx_t *ctx, benchmark_result_t *result) {

   timer_start(&ctx->timer);
   keygen(&ctx->params, &ctx->sk, &ctx->pk);
   result->keygen_ms = timer_stop_ms(&ctx->timer);
   printf("[OK] Clé secrète et publique du signer générées\n");


   timer_start(&ctx->timer);
   signer_compute_response(&ctx->resp_signer, &ctx->req, &ctx->params, &ctx->sk);
   result->signer_ms = timer_stop_ms(&ctx->timer);
   ctx->resp_signer_init = 1;
   printf("[OK] Réponse du signer calculée\n");


   signer_write_response_to_file(&ctx->resp_signer);
   printf("[OK] Réponse du signer écrite dans %s\n", SIGNER_RESPONSE_FILE);


   return 0;
}


static int protocol_user_phase(protocol_ctx_t *ctx, benchmark_result_t *result){

   if (!user_read_signer_response_from_file(&ctx->resp_user, &ctx->params)) {
      printf("[ERR] Erreur lecture de la réponse du signer\n");
      return -1;
   }
   ctx->resp_user_init = 1;
   printf("[OK] Réponse du signer lue par l'user\n");


   timer_start(&ctx->timer);
   user_compute_signature(&ctx->sig, &ctx->user, &ctx->resp_user, &ctx->params);
   result->signature_ms = timer_stop_ms(&ctx->timer);
   ctx->sig_init = 1;
   printf("[OK] Signature finale calculée par l'user\n");


   user_write_signature_to_file(&ctx->sig);
   printf("[OK] Signature finale écrite dans %s\n", SIGNATURE_FILE);

   return 0;
}

static int protocol_verify_phase(protocol_ctx_t *ctx, benchmark_result_t *result){

   if (!verifier_read_signature_from_file(&ctx->sig_verif)) {
      printf("[ERR] Erreur lecture de la signature\n");
      return -1;
   }
   ctx->sig_verif_init = 1;

   timer_start(&ctx->timer);
   int valid = verifier_check_signature(&ctx->sig_verif, &ctx->params, &ctx->pk, ctx->message);
   result->verify_ms = timer_stop_ms(&ctx->timer);

   printf("\n[---Signature %s---]\n", valid ? "valide" : "invalide");

   return 0;
}


static void protocol_cleanup(protocol_ctx_t *ctx){

   if (ctx->sig_verif_init)
      user_signature_free(&ctx->sig_verif);

   if (ctx->sig_init)
      user_signature_free(&ctx->sig);

   if (ctx->req_init)
      signer_ot_request_free(&ctx->req);

   if (ctx->resp_signer_init)
      signer_ot_response_free(&ctx->resp_signer);

   if (ctx->resp_user_init)
      signer_ot_response_free(&ctx->resp_user);

   if (ctx->user_init)
      user_state_free(&ctx->user);

   free_params(&ctx->params);
   core_clean();
}
