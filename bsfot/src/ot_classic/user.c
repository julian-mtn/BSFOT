#include "ot_classic/user.h"
#include "common/params.h"
#include "common/io.h"
#include "waters/waters.h"

void user_ot_init(user_state_t *st, const public_params_t *params, const uint8_t *message)
{
    st->l = params->l;

    /* Copie du message */
    st->message = malloc(params->l * sizeof(uint8_t));
    memcpy(st->message, message, params->l);

    st->y   = malloc(params->l * sizeof(bn_t));
    st->ek0 = malloc(params->l * sizeof(g1_t));
    st->ek1 = malloc(params->l * sizeof(g1_t));

    bn_t order;
    bn_null(order);
    bn_new(order);
    g1_get_ord(order);

    for (int i = 0; i < params->l; i++) {

        bn_null(st->y[i]);
        bn_new(st->y[i]);

        g1_null(st->ek0[i]);
        g1_null(st->ek1[i]);

        g1_new(st->ek0[i]);
        g1_new(st->ek1[i]);

        /* y_i <- Z_p */
        bn_rand_mod(st->y[i], order);

        /* tmp = x_i - y_i mod p */
        bn_t tmp;
        bn_null(tmp);
        bn_new(tmp);

        bn_sub(tmp, params->x[i], st->y[i]);
        bn_mod(tmp, tmp, order);

        if (message[i] == 0) {

            /* ek0 = g1^{y_i} */
            g1_mul(st->ek0[i], params->g1, st->y[i]);

            /* ek1 = g1^{x_i-y_i} */
            g1_mul(st->ek1[i], params->g1, tmp);

        } else {

            /* ek0 = g1^{x_i-y_i} */
            g1_mul(st->ek0[i], params->g1, tmp);

            /* ek1 = g1^{y_i} */
            g1_mul(st->ek1[i], params->g1, st->y[i]);
        }

        bn_free(tmp);
    }

    bn_free(order);
}

void user_state_free(user_state_t *st)
{
    free(st->message);

    for (int i = 0; i < st->l; i++) {
        bn_free(st->y[i]);
        g1_free(st->ek0[i]);
        g1_free(st->ek1[i]);
    }

    free(st->y);
    free(st->ek0);
    free(st->ek1);
}

void user_write_ot_keys_to_file(const user_state_t *st){
    
    FILE *file = fopen(OT_KEYS_FILE, "wb");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    /* Écriture de la longueur */
    if (fwrite(&st->l, sizeof(int), 1, file) != 1) {
        fclose(file);
        return;
    }

    /* Écriture des paires OT */
    for (int i = 0; i < st->l; i++) {

        if (!write_g1(file, st->ek0[i]) ||
            !write_g1(file, st->ek1[i])) {
            fclose(file);
            return;
        }
    }

    fclose(file);
}

int user_read_signer_response_from_file(signer_response_t *resp,const public_params_t *params){
    
    FILE *file = fopen(SIGNER_RESPONSE_FILE, "rb");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return 0;

    }
    /*lecture de la longueur l*/
    if (fread(&resp->l, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    if (resp->l != params->l) {
        fprintf(stderr, "Erreur : longueur invalide.\n");
        fclose(file);
        return 0;
    }

    resp->ct0_i = malloc(resp->l * sizeof(g1_t));
    resp->ct1_i = malloc(resp->l * sizeof(g1_t));
    
    if (resp->ct0_i == NULL || resp->ct1_i == NULL) {
        fclose(file);
        return 0;
    }

    /*lecture des ct0_i et ct1_i*/
    for (int i = 0; i < resp->l; i++) {
        g1_null(resp->ct0_i[i]);
        g1_new(resp->ct0_i[i]);
        g1_null(resp->ct1_i[i]);
        g1_new(resp->ct1_i[i]);

        if (!read_g1(file, resp->ct0_i[i]) || !read_g1(file, resp->ct1_i[i])) {
            fclose(file);
            return 0;
        }
    }

    /*lecture de ct0*/
    g1_null(resp->ct0);
    g1_new(resp->ct0);
    if (!read_g1(file, resp->ct0)) {
        fclose(file);
        return 0;
    }

    /*lecture de sigma2*/
    g2_null(resp->sigma2);
    g2_new(resp->sigma2);
    if (!read_g2(file, resp->sigma2)) {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}


void user_compute_signature(signature_t *sig, const user_state_t *st, const signer_response_t *resp, public_params_t *params){

    g1_null(sig->fm);
    g1_null(sig->sigma1);
    g2_null(sig->sigma2);
    g1_new(sig->fm);
    g1_new(sig->sigma1);
    g2_new(sig->sigma2);

    /* Calcul de F(M) */
    waters_hash(sig->fm, params->u, params->l, st->message);

    /*sigma1 = somme des sigma1_i = somme des ct_i,M_i / ct0^{y_i}*/
    g1_set_infty(sig->sigma1);
    for(int i=0; i< params->l; i++){
        g1_t tmp;
        g1_null(tmp);
        g1_new(tmp);

        g1_t sigma1_i;
        g1_null(sigma1_i);
        g1_new(sigma1_i);

        /* sigma1_i = ct_i,M_i / ct0^{y_i} */
        g1_mul(tmp, resp->ct0, st->y[i]);

        if (st->message[i] == 0){
            g1_sub(sigma1_i, resp->ct0_i[i], tmp);
        }
        else{
            g1_sub(sigma1_i, resp->ct1_i[i], tmp);
        }
        g1_add(sig->sigma1, sig->sigma1, sigma1_i); /*sigma1 *= sigma1_i*/

        g1_free(tmp);
        g1_free(sigma1_i);
    }

    /* t' <- Z_p */
    bn_t t_prime;
    bn_null(t_prime);
    bn_new(t_prime);

    bn_t order;
    bn_null(order);
    bn_new(order);

    g1_get_ord(order);
    bn_rand_mod(t_prime, order);

    /*F(m)^t'*/
    g1_t fm_t_prime;
    g1_null(fm_t_prime);
    g1_new(fm_t_prime);
    g1_mul(fm_t_prime, sig->fm, t_prime);

    /*sigma1_hat = sigma1 * F(m)^t'*/
    g1_add(sig->sigma1, sig->sigma1, fm_t_prime);

    /*sigma2_hat = sigma2*g2^t'*/
    g2_t g2_t_prime;
    g2_null(g2_t_prime);
    g2_new(g2_t_prime);
    g2_mul(g2_t_prime, params->g2, t_prime);
    g2_add(sig->sigma2, resp->sigma2, g2_t_prime);

    g1_free(fm_t_prime);
    g2_free(g2_t_prime);
    bn_free(t_prime);
    bn_free(order);
}

void user_write_signature_to_file(signature_t *sig){
    FILE *file = fopen(SIGNATURE_FILE, "wb");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    if (!write_g1(file, sig->fm) || !write_g1(file, sig->sigma1) || !write_g2(file, sig->sigma2)) {
        fprintf(stderr, "Erreur lors de l'écriture de la signature.\n");
        fclose(file);
        return;
    }
    fclose(file);
}

void user_signature_free(signature_t *sig){
    if (sig == NULL) return;
    g1_free(sig->fm);
    g1_free(sig->sigma1);
    g2_free(sig->sigma2);
}