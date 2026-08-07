#include "dual/signer.h"
#include "common/io.h"
#include "common/config.h"

#include <stdio.h>
#include <stdlib.h>

int signer_dualmod_read_ot_request_from_file(dualmod_ot_request_t *req, const public_params_t *params){

    FILE *file = fopen(DUALMOD_OT_KEYS_FILE, "rb");

    if(file == NULL) {
        perror("Erreur ouverture fichier");
        return 0;
    }

    /* Lecture de l */
    if(fread(&req->l, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }

    /* Vérification de la taille */
    if(req->l != params->l) {
        fprintf(stderr, "Erreur : longueur invalide\n");
        fclose(file);
        return 0;
    }

    req->ek0  = malloc(req->l * sizeof(g1_t));
    req->tek0 = malloc(req->l * sizeof(g1_t));
    if(req->ek0 == NULL || req->tek0 == NULL) {
        fclose(file);
        return 0;
    }

    /* Lecture des clés */
    for(int i = 0; i < req->l; i++) {

        g1_null(req->ek0[i]);
        g1_new(req->ek0[i]);
        g1_null(req->tek0[i]);
        g1_new(req->tek0[i]);

        /*
         * ek_{i,0}
         * \tilde{ek}_{i,0}
         */
        if(!read_g1(file, req->ek0[i])) {
            fclose(file);
            return 0;
        }

        if(!read_g1(file, req->tek0[i])) {
            fclose(file);
            return 0;
        }
    }


    fclose(file);
    return 1;
}

void signer_dualmod_ot_request_clear(dualmod_ot_request_t *req){

    if(req == NULL) return;

    if(req->ek0 != NULL) {
        for(int i = 0; i < req->l; i++) {
            g1_free(req->ek0[i]);
        }
        free(req->ek0);
        req->ek0 = NULL;
    }

    if(req->tek0 != NULL){
        for(int i = 0; i < req->l; i++){
            g1_free(req->tek0[i]);
        }
        free(req->tek0);
        req->tek0 = NULL;
    }
    req->l = 0;
}

void signer_dualmod_compute_response(dualmod_signer_response_t *resp, const dualmod_ot_request_t *req, const public_params_t *params, const signer_secret_key_t *sk){

    resp->l = req->l;

    resp->c0_i = malloc(req->l * sizeof(g1_t));
    resp->d0_i = malloc(req->l * sizeof(g1_t));
    resp->c1_i = malloc(req->l * sizeof(g1_t));
    resp->d1_i = malloc(req->l * sizeof(g1_t));

    bn_t p,t;
    bn_null(p);
    bn_null(t);
    bn_new(p);
    bn_new(t);
    g1_get_ord(p);

    /* t <- Z_p*/
    bn_rand_mod(t,p);

    /* sigma2 = g2^t */
    g2_null(resp->sigma2);
    g2_new(resp->sigma2);
    g2_mul( resp->sigma2,params->g2, t );

    /* sigma2' = g1^t */
    g1_null(resp->sigma2_prime);
    g1_new(resp->sigma2_prime);
    g1_mul(resp->sigma2_prime, params->g1, t);

    /* --- calcul de bsk * u_0^t-- */
    g1_t target;
    g1_null(target);
    g1_new(target);

    /* target = t*u0 */
    g1_mul(target, params->u0,t);

    /*target = bsk + t*u0*/
    g1_add(target, target, sk->bsk);

    /* --- Génération des alpha_i ---*/

    g1_t *alpha;
    alpha = malloc(req->l * sizeof(g1_t));
    for(int i = 0; i < req->l; i++){
        g1_null(alpha[i]);
        g1_new(alpha[i]);
    }

    g1_t sum_alpha;
    g1_null(sum_alpha);
    g1_new(sum_alpha);

    g1_set_infty(sum_alpha);

    /*
    * alpha_i aléatoires sauf le dernier
    */
    for(int i = 0; i < req->l-1; i++){
        bn_t ai;
        bn_null(ai);
        bn_new(ai);
        bn_rand_mod(ai, p);

        g1_mul(alpha[i], params->g1, ai);
        g1_add(sum_alpha, sum_alpha, alpha[i]);


        bn_free(ai);
    }

    /* alpha_l-1 = target - somme(alpha_i) */
    g1_sub( alpha[req->l-1], target, sum_alpha );

    g1_t check;
    g1_null(check);
    g1_new(check);

    g1_set_infty(check);

    for(int i = 0; i < req->l; i++){
        g1_add(check, check, alpha[i]);
    }

    g1_free(check);

    /* Calcul des C_i,b et D_i,b */
    for(int i = 0; i < req->l; i++) {

        /*
         * ek_i,1 = ek_i,0 * w^-1
         * tek_i,1 = tek_i,0 * wt^-1
         */
        g1_t ek1;
        g1_t tek1;
        g1_null(ek1);
        g1_null(tek1);
        g1_new(ek1);
        g1_new(tek1);
        g1_sub(ek1, params->dm_w, req->ek0[i]);
        g1_sub(tek1, params->dm_w_tilde, req->tek0[i]);

        /* --- Calcul de C_i,0 et D_i,0 --- */
    
        bn_t s0;
        bn_t sp0;
        bn_null(s0);
        bn_null(sp0);
        bn_new(s0);
        bn_new(sp0);

        /* s_i,0 , s'_i,0 <- Z_p */
        bn_rand_mod(s0,p);
        bn_rand_mod(sp0,p);

        /* C_i,0 = g^s_i,0 h^s'_i,0 */
        g1_null(resp->c0_i[i]);
        g1_new(resp->c0_i[i]);
        g1_mul(resp->c0_i[i], params->dm_g, s0);

        g1_t tmp;
        g1_null(tmp);
        g1_new(tmp);
        g1_mul(tmp, params->dm_h, sp0);

        g1_add(resp->c0_i[i], resp->c0_i[i], tmp);

        /* D_i,0 = ek_i,0^s_i,0 * tek_i,0^s'_i,0 * alpha_i */

        g1_null(resp->d0_i[i]);
        g1_new(resp->d0_i[i]);
        g1_mul(resp->d0_i[i],req->ek0[i],s0);
        g1_mul(tmp, req->tek0[i], sp0);

        g1_add(resp->d0_i[i], resp->d0_i[i], tmp);

        g1_add(resp->d0_i[i], resp->d0_i[i], alpha[i]);

        bn_free(s0);
        bn_free(sp0);

        /* --- Calcul de C_i,1 et D_i,1 --- */

        bn_t s1;
        bn_t sp1;
        bn_null(s1);
        bn_null(sp1);
        bn_new(s1);
        bn_new(sp1);

        /* s_i,1 , s'_i,1 <- Z_p */
        bn_rand_mod(s1,p);
        bn_rand_mod(sp1,p);

        /*C_i,1 = g^s_i,1 * h^s'_i,1*/
        g1_null(resp->c1_i[i]);
        g1_new(resp->c1_i[i]);

        g1_mul(resp->c1_i[i], params->dm_g, s1);
        g1_mul(tmp,params->dm_h,sp1);

        g1_add(resp->c1_i[i], resp->c1_i[i],tmp);

        /* D_i,1 = ek_i,1^s_i,1 * tek_i,1^s'_i,1 * alpha_i * u_i^t */
        g1_null(resp->d1_i[i]);
        g1_new(resp->d1_i[i]);

        g1_mul( resp->d1_i[i], ek1, s1);
        g1_mul(tmp, tek1, sp1);

        g1_add(resp->d1_i[i], resp->d1_i[i], tmp);
        g1_add(resp->d1_i[i], resp->d1_i[i], alpha[i]);

        /* ajout de u_i^t*/
        g1_mul(tmp, params->u[i],t);
        g1_add(resp->d1_i[i], resp->d1_i[i],tmp);

        bn_free(s1);
        bn_free(sp1);
        g1_free(tmp);
        g1_free(ek1);
        g1_free(tek1);

    }

}

void signer_dualmod_write_response_to_file(dualmod_signer_response_t *resp, const char *filename){

    FILE *file = fopen(filename,"wb");

    if(file == NULL){
        perror("Erreur ouverture fichier");
        return;
    }
    fwrite(&resp->l,sizeof(int),1,file);

    for(int i = 0; i < resp->l; i++){

        write_g1(file,resp->c0_i[i]);

        write_g1(file,resp->d0_i[i]);

        write_g1(file,resp->c1_i[i]);

        write_g1(file,resp->d1_i[i]);
    }

    write_g2(file,resp->sigma2);

    write_g1(file,resp->sigma2_prime);

    fclose(file);

}

void signer_dualmod_response_free(dualmod_signer_response_t *resp){

    for(int i = 0; i < resp->l; i++){
        g1_free(resp->c0_i[i]);
        g1_free(resp->d0_i[i]);
        g1_free(resp->c1_i[i]);
        g1_free(resp->d1_i[i]);
    }

    free(resp->c0_i);
    free(resp->d0_i);
    free(resp->c1_i);
    free(resp->d1_i);

    g2_free(resp->sigma2);
    g1_free(resp->sigma2_prime);

}

void signer_dualmod_ot_request_free(dualmod_ot_request_t *req)
{

    for(int i = 0; i < req->l; i++)
    {
        g1_free(req->ek0[i]);
        g1_free(req->tek0[i]);
    }


    free(req->ek0);
    free(req->tek0);
}