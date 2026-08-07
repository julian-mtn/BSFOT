#include "user_dualmode.h"
#include "params.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "io.h"

void user_dualmod_ot_init(dualmod_user_state_t *st, const public_params_t *params, const uint8_t *message){

    st->l = params->l;
    st->message = malloc(params->l * sizeof(uint8_t));
    memcpy(st->message, message, params->l);

    st->y = malloc(params->l * sizeof(bn_t));
    st->ek0  = malloc(params->l * sizeof(g1_t));
    st->ek1  = malloc(params->l * sizeof(g1_t));
    st->tek0 = malloc(params->l * sizeof(g1_t));
    st->tek1 = malloc(params->l * sizeof(g1_t));

    bn_t order;
    bn_null(order);
    bn_new(order);
    g1_get_ord(order);

    for(int i = 0; i < params->l; i++){

        bn_null(st->y[i]); 
        bn_new(st->y[i]);
        g1_null(st->ek0[i]);
        g1_null(st->ek1[i]);
        g1_null(st->tek0[i]);
        g1_null(st->tek1[i]);
        g1_new(st->ek0[i]);
        g1_new(st->ek1[i]);
        g1_new(st->tek0[i]);
        g1_new(st->tek1[i]);

        /* y_i <- Zp*/
        bn_rand_mod(st->y[i], order);

        g1_t ek_real;
        g1_t tek_real;
        g1_t inv_ek;
        g1_t inv_tek;
        g1_null(ek_real);
        g1_null(tek_real);
        g1_null(inv_ek);
        g1_null(inv_tek);
        g1_new(ek_real);
        g1_new(tek_real);
        g1_new(inv_ek);
        g1_new(inv_tek);

        /* Branche DH :
        * ek = dm_g^y
        * tek = dm_h^y
        */
        g1_mul(ek_real,params->dm_g,st->y[i]);
        g1_mul(tek_real,params->dm_h,st->y[i]);

        /* Inverse dans G1 :
        * ek^{-1} = -ek
        * tek^{-1} = -tek
        */
        g1_neg(inv_ek,ek_real);
        g1_neg(inv_tek,tek_real);

        if(message[i] == 0){

            /* Branche DH pour le bit 0 */
            g1_copy(st->ek0[i],ek_real);
            g1_copy(st->tek0[i],tek_real);

            /* Branche messy pour le bit 1 :
             * w * ek^-1
             * w_tilde * tek^-1
             */
            g1_add(st->ek1[i], params->dm_w, inv_ek);
            g1_add(st->tek1[i], params->dm_w_tilde, inv_tek);

        }
        else {

            /* Branche DH pour le bit 1*/
            g1_copy(st->ek1[i], ek_real);
            g1_copy(st->tek1[i], tek_real);

            /* Branche messy pour le bit 0 */
            g1_add(st->ek0[i], params->dm_w, inv_ek);
            g1_add(st->tek0[i], params->dm_w_tilde, inv_tek);

        }

        g1_free(ek_real);
        g1_free(tek_real);
        g1_free(inv_ek);
        g1_free(inv_tek);

    }

    bn_free(order);
}


void user_dualmod_state_free(dualmod_user_state_t *st){

    free(st->message);

    for(int i = 0; i < st->l; i++){
        bn_free(st->y[i]);
        g1_free(st->ek0[i]);
        g1_free(st->ek1[i]);
        g1_free(st->tek0[i]);
        g1_free(st->tek1[i]);
    }

    free(st->y);
    free(st->ek0);
    free(st->ek1);
    free(st->tek0);
    free(st->tek1);
}

void user_write_dualmod_ot_keys_to_file(const dualmod_user_state_t *st){

    FILE *file = fopen(DUALMOD_OT_KEYS_FILE, "wb");

    if(file == NULL){
        perror("Erreur ouverture fichier");
        return;
    }

    fwrite(&st->l,sizeof(int),1,file);

    for(int i = 0; i < st->l; i++){

        write_g1(file, st->ek0[i]);
        write_g1(file, st->tek0[i]);

        //write_g1(file, st->ek1[i]);
        //write_g1(file, st->tek1[i]);

    }

    fclose(file);
}


void user_dualmod_compute_signature(signature_t *sig, const dualmod_user_state_t *st, const dualmod_signer_response_t *resp, public_params_t *params){

    g1_null(sig->fm);
    g1_null(sig->sigma1);
    g2_null(sig->sigma2);
    g1_new(sig->fm);
    g1_new(sig->sigma1);
    g2_new(sig->sigma2);

    /* F(M) */
    waters_hash_dualmode(sig->fm, params->u, params->l, st->message,params); /* F(M) = u0 * prod(u_i^{M_i}) */

    /* sigma1 = produit des alpha_i */
    g1_set_infty(sig->sigma1);

    for(int i = 0; i < st->l; i++){

        g1_t ct;
        g1_t mask;
        g1_null(ct);
        g1_null(mask);
        g1_new(ct);
        g1_new(mask);

        /* branche correspondant au bit */
        if(st->message[i] == 0){

            g1_copy(ct,resp->d0_i[i]);

            /* masque = C_i,0^{y_i} */
            g1_mul(mask, resp->c0_i[i], st->y[i]);

        }
        else {

            g1_copy(ct,resp->d1_i[i]);
            /* masque = C_i,1^{y_i} */
            g1_mul(mask, resp->c1_i[i], st->y[i]);
        }

        /*
         * ct * masque ^ -1
         */
        g1_neg(mask, mask);
        g1_add(ct, ct, mask);

        /*
         * produit sigma1_i
         */
        g1_add(sig->sigma1, sig->sigma1, ct);


        g1_free(ct);
        g1_free(mask);
    }

    /* calcul de sigma1_hat */
    bn_t ord;
    bn_null(ord);
    bn_new(ord);
    g1_get_ord(ord);

    bn_t t_prime;
    bn_null(t_prime);
    bn_new(t_prime);
    bn_rand_mod(t_prime, ord);

    /* sigma1 *= t' * F(M) */
    g1_t tmp1;
    g1_null(tmp1);
    g1_new(tmp1);
    g1_mul(tmp1, sig->fm, t_prime);
    g1_add(sig->sigma1, sig->sigma1, tmp1);

    /* calcul de sigma2_hat */

    g2_copy(sig->sigma2, resp->sigma2);

    g2_t tmp2;
    g2_null(tmp2);
    g2_new(tmp2);

    g2_mul(tmp2, params->g2, t_prime);

    g2_add(sig->sigma2, sig->sigma2, tmp2); 

    /* calcul de sigma2_hat_prime */

    /*sigma2' reçu du signer */
    g1_null(sig->sigma2_prime);
    g1_new(sig->sigma2_prime);
    g1_copy(sig->sigma2_prime, resp->sigma2_prime);

    /* tmp3 = g1^t' */
    g1_t tmp3;
    g1_null(tmp3);
    g1_new(tmp3);
    g1_mul(tmp3, params->g1, t_prime);

    /* sigma2_hat_prime = sigma2' * g1^t' */
    g1_add(sig->sigma2_prime, sig->sigma2_prime, tmp3);

    g1_free(tmp1);
    g2_free(tmp2);
    g1_free(tmp3);
    bn_free(t_prime);

}

int user_dualmod_read_signer_response_from_file(dualmod_signer_response_t *resp){

    FILE *file = fopen(DUALMOD_SIGNER_RESPONSE_FILE, "rb");

    if (file == NULL){
        perror("Erreur ouverture réponse Dual-Mode");
        return 0;
    }

    /* Lecture de la longueur */
    if (fread(&resp->l, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }

    resp->c0_i = malloc(resp->l * sizeof(g1_t));
    resp->d0_i = malloc(resp->l * sizeof(g1_t));
    resp->c1_i = malloc(resp->l * sizeof(g1_t));
    resp->d1_i = malloc(resp->l * sizeof(g1_t));

    for(int i = 0; i < resp->l; i++){

        g1_null(resp->c0_i[i]);
        g1_null(resp->d0_i[i]);
        g1_null(resp->c1_i[i]);
        g1_null(resp->d1_i[i]);

        g1_new(resp->c0_i[i]);
        g1_new(resp->d0_i[i]);
        g1_new(resp->c1_i[i]);
        g1_new(resp->d1_i[i]);


        if(!read_g1(file, resp->c0_i[i])){
            fclose(file);
            return 0;
        }

        if(!read_g1(file, resp->d0_i[i])){
            fclose(file);
            return 0;
        }

        if(!read_g1(file, resp->c1_i[i])){
            fclose(file);
            return 0;
        }

        if(!read_g1(file, resp->d1_i[i])){
            fclose(file);
            return 0;
        }
    }


    /* Lecture sigma2 */
    g2_null(resp->sigma2);
    g2_new(resp->sigma2);

    if(!read_g2(file, resp->sigma2)) {
        fclose(file);
        return 0;
    }


    /* Lecture sigma2' */
    g1_null(resp->sigma2_prime);
    g1_new(resp->sigma2_prime);

    if (!read_g1(file, resp->sigma2_prime)) {
        fclose(file);
        return 0;
    }

    fclose(file);

    return 1;
}

int user_write_dualmod_signature_to_file(const signature_t *sig, const char *filename){
    FILE *file = fopen(filename, "wb");

    if(file == NULL) return 0;

    if (!write_g1(file, sig->fm) ||
        !write_g1(file, sig->sigma1) ||
        !write_g2(file, sig->sigma2) ||
        !write_g1(file, sig->sigma2_prime)) {

        fclose(file);
        return 0;
    }

    fclose(file);

    return 1;
}