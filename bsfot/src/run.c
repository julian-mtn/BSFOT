#include "run.h"
#include "timer.h"
#include "message.h"
#include "config.h"

int protocol_run(benchmark_result_t *result)
{
   int ret = -1;
   int user_init = 0;
   int req_init = 0;
   int resp_signer_init = 0;
   int resp_user_init = 0;
   int sig_init = 0;
   int sig_verif_init = 0;

   benchmark_timer_t timer;


   /* init RELIC */
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


   public_params_t params;
                                                   timer_start(&timer);
   init_params(&params, MESSAGE_LENGTH);
                                                   result->params_ms = timer_stop_ms(&timer);
   printf("[OK] Paramètres publics générés\n");


   uint8_t message[MESSAGE_LENGTH];
   generate_random_message(message,MESSAGE_LENGTH);


   user_state_t user;
                                                   timer_start(&timer);
   user_ot_init(&user, &params, message);
                                                   result->ot_ms = timer_stop_ms(&timer);
   user_init = 1;
   printf("[OK] Paires OT générées\n");


   user_write_ot_keys_to_file(&user);
   printf("[OK] Clés OT écrites dans %s\n", OT_KEYS_FILE);


   ot_request_t req;
   if (!signer_read_ot_request_from_file(&req, &params)) {
      printf("[ERR] Erreur lecture des clés OT\n");
      goto cleanup;
   }
   req_init = 1;
   printf("[OK] Clés OT lues par le signer\n");


   signer_secret_key_t sk;
   signer_public_key_t pk;
                                                                     timer_start(&timer);
   keygen(&params, &sk, &pk);
                                                                     result->keygen_ms = timer_stop_ms(&timer);
   printf("[OK] Clé secrète et publique du signer générées\n");


   signer_response_t resp_signer;
                                                                     timer_start(&timer);
   signer_compute_response(&resp_signer, &req, &params, &sk);
                                                                     result->signer_ms = timer_stop_ms(&timer);
   resp_signer_init = 1;
   printf("[OK] Réponse du signer calculée\n");


   signer_write_response_to_file(&resp_signer);
   printf("[OK] Réponse du signer écrite dans %s\n", SIGNER_RESPONSE_FILE);


   signer_response_t resp_user;
   if (!user_read_signer_response_from_file(&resp_user, &params)) {
      printf("[ERR] Erreur lecture de la réponse du signer\n");
      goto cleanup;
   }
   resp_user_init = 1;
   printf("[OK] Réponse du signer lue par l'user\n");


   signature_t sig;
                                                                        timer_start(&timer);
   user_compute_signature(&sig, &user, &resp_user, &params);
                                                                        result->signature_ms = timer_stop_ms(&timer);
   sig_init = 1;
   printf("[OK] Signature finale calculée par l'user\n");


   user_write_signature_to_file(&sig);
   printf("[OK] Signature finale écrite dans %s\n", SIGNATURE_FILE);



   /* Vérification de la signature */

   signature_t sig_verif;

   if (!verifier_read_signature_from_file(&sig_verif)) {
      printf("[ERR] Erreur lecture de la signature\n");
      goto cleanup;
   }
   sig_verif_init = 1;


                                             timer_start(&timer);
   int valid = verifier_check_signature(
      &sig_verif,
      &params,
      &pk,
      message
   );
                                             result->verify_ms = timer_stop_ms(&timer);

   if (valid){
      printf("\n[---Signature valide---]\n");
   }
   else{
      printf("\n[---Signature invalide---]\n");
   }
   
   ret = 0;


cleanup:

   if (sig_verif_init)
      user_signature_free(&sig_verif);

   if (sig_init)
      user_signature_free(&sig);

   if (req_init)
      signer_ot_request_free(&req);

   if (resp_signer_init)
      signer_ot_response_free(&resp_signer);

   if (resp_user_init)
      signer_ot_response_free(&resp_user);

   if (user_init)
      user_state_free(&user);

   free_params(&params);

   core_clean();

   return ret;
}