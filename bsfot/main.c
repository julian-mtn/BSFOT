#include <stdio.h>
#include <stdint.h>

#include <relic/relic.h>
#include <relic/relic_pp.h>

#include "params.h"
#include "user.h"
#include "signer.h"
#include "keys.h"
#include "verifier.h"


int main() {

   int ret = -1;
   int user_init = 0;
   int req_init = 0;
   int resp_signer_init = 0;
   int resp_user_init = 0;
   int sig_init = 0;
   int sig_verif_init = 0;


   /* init RELIC */
   if (core_init() != RLC_OK) {
      printf("Erreur initialisation RELIC\n");
      return -1;
   }

   if (pc_param_set_any() != RLC_OK) {
      printf("Erreur initialisation paramètres bilinéaires\n");
      core_clean();
      return -1;
   }

   printf("RELIC initialized\n");


   public_params_t params;
   int l = 8; // taille du message en bits
   init_params(&params, l);
   printf("Paramètres publics générés\n");


   uint8_t message[8] = {
      1,0,1,1,0,0,1,0
   };


   user_state_t user;
   user_ot_init(&user, &params, message);
   user_init = 1;
   printf("Paires OT générées\n");


   user_write_ot_keys_to_file(&user);
   printf("Clés OT écrites dans %s\n", OT_KEYS_FILE);


   ot_request_t req;
   if (!signer_read_ot_request_from_file(&req, &params)) {
      printf("Erreur lecture des clés OT\n");
      goto cleanup;
   }
   req_init = 1;
   printf("Clés OT lues par le signer\n");


   signer_secret_key_t sk;
   signer_public_key_t pk;
   keygen(&params, &sk, &pk);
   printf("Clé secrète et publique du signer générées\n");


   signer_response_t resp_signer;
   signer_compute_response(&resp_signer, &req, &params, &sk);
   resp_signer_init = 1;
   printf("Réponse du signer calculée\n");


   signer_write_response_to_file(&resp_signer);
   printf("Réponse du signer écrite dans %s\n", SIGNER_RESPONSE_FILE);


   signer_response_t resp_user;
   if (!user_read_signer_response_from_file(&resp_user, &params)) {
      printf("Erreur lecture de la réponse du signer\n");
      goto cleanup;
   }
   resp_user_init = 1;
   printf("Réponse du signer lue par l'user\n");


   signature_t sig;
   user_compute_signature(&sig, &user, &resp_user, &params);
   sig_init = 1;
   printf("Signature finale calculée par l'user\n");


   user_write_signature_to_file(&sig);
   printf("Signature finale écrite dans %s\n", SIGNATURE_FILE);



   /* Vérification de la signature */

   signature_t sig_verif;

   if (!verifier_read_signature_from_file(&sig_verif)) {
      printf("Erreur lecture de la signature\n");
      goto cleanup;
   }
   sig_verif_init = 1;


   if (verifier_check_signature(&sig_verif, &params, &pk, message)) {
      printf("[[[Signature valide]]]\n");
   }
   else {
      printf("[[[Signature invalide]]]\n");
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