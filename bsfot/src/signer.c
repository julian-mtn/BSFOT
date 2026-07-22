#include "signer.h"
#include "io.h"

int signer_read_ot_request_from_file(ot_request_t *req, const public_params_t *params){

    FILE *file = fopen(OT_KEYS_FILE, "rb");

    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return 0;
    }

    /* Lecture de la longueur l */
    if (fread(&req->l, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    if (req->l != params->l) {
        fprintf(stderr, "Erreur : longueur invalide.\n");
        fclose(file);
        return 0;
    }

    /* Lecture de ek0 et ek1 */
    req->ek0 = malloc(req->l * sizeof(g1_t));
    req->ek1 = malloc(req->l * sizeof(g1_t));
    if (req->ek0 == NULL || req->ek1 == NULL) {
        fclose(file);
        return 0;
    }


    for (int i = 0; i < req->l; i++) {

        g1_null(req->ek0[i]);
        g1_null(req->ek1[i]);

        g1_new(req->ek0[i]);
        g1_new(req->ek1[i]);


        if (!read_g1(file, req->ek0[i])) {
            fclose(file);
            return 0;
        }

        if (!read_g1(file, req->ek1[i])) {
            fclose(file);
            return 0;
        }
    }

    /*Vérification des ek0 et ek1*/
    /*on regarde si eki,0 * ek1,i = g1^xi = h[i] */
    g1_t tmp;
    g1_null(tmp);
    g1_new(tmp);

    for (int i = 0; i < req->l; i++) {

        g1_add(tmp, req->ek0[i], req->ek1[i]);

        if (g1_cmp(tmp, params->h[i]) != RLC_EQ) {

            fprintf(stderr, "Erreur : clés OT invalides pour i=%d\n", i);

            g1_free(tmp);
            fclose(file);

            return 0;
        }
    }

    g1_free(tmp);

    fclose(file);

    return 1;
}

void signer_ot_request_free(ot_request_t *req)
{
    for (int i = 0; i < req->l; i++) {
        g1_free(req->ek0[i]);
        g1_free(req->ek1[i]);
    }
    free(req->ek0);
    free(req->ek1);
}

void signer_compute_response(signer_response_t *resp, const ot_request_t *req,const public_params_t *params,const signer_secret_key_t *sk){

    /*init structure reponse*/
    resp->l = req->l;
    resp->ct0_i = malloc(req->l * sizeof(g1_t));
    resp->ct1_i = malloc(req->l * sizeof(g1_t));

    /*ordre du groupe*/
    bn_t p;
    bn_null(p);
    bn_new(p);
    g1_get_ord(p);

    /*choisir alpha_i aléatoirement tel que ∏alpha_i = h_s^a*/

    g1_t *alpha = malloc(req->l * sizeof(g1_t));
    for(int i=0; i < req->l; i++){
        g1_null(alpha[i]);
        g1_new(alpha[i]);
    }
    g1_t product;
    g1_null(product);
    g1_new(product);
    g1_set_infty(product);

    for(int i=0; i < req->l-1; i++){
        bn_t ai;
        bn_null(ai);
        bn_new(ai);

        bn_rand_mod(ai, p);
        g1_mul(alpha[i], params->g1, ai);
        g1_add(product, product, alpha[i]);

        bn_free(ai);
    }
    g1_sub(alpha[req->l-1], sk->bsk, product);


    /*r,t <- Z_p*/
    bn_t r, t;
    bn_null(r);
    bn_null(t);
    bn_new(r);
    bn_new(t);

    bn_rand_mod(r, p);
    bn_rand_mod(t, p);

    /* ct_i,0 = ek0 * r * alpha_i */
    /* ct_i,1 = ek1 * r * alpha_i * (u_i)^t */

    for (int i = 0; i < req->l; i++) {

        g1_null(resp->ct0_i[i]);
        g1_null(resp->ct1_i[i]);
        g1_new(resp->ct0_i[i]);
        g1_new(resp->ct1_i[i]);

        g1_t tmp1;
        g1_t tmp2;

        g1_null(tmp1);
        g1_null(tmp2);
        g1_new(tmp1);
        g1_new(tmp2);

        /*
        * ct_i,0 = ek0_i^r * alpha_i
        */
        g1_mul(tmp1, req->ek0[i], r);
        g1_add(resp->ct0_i[i], tmp1, alpha[i]);

        /*
        * ct_i,1 = ek1_i^r * alpha_i * u_i^t
        */
        g1_mul(tmp1, req->ek1[i], r);      // ek1_i^r
        g1_mul(tmp2, params->u[i], t);     // u_i^t

        g1_add(resp->ct1_i[i], tmp1, alpha[i]);
        g1_add(resp->ct1_i[i], resp->ct1_i[i], tmp2);

        g1_free(tmp1);
        g1_free(tmp2);
    }

    /*ct0 = g1^r*/
    g1_null(resp->ct0);
    g1_new(resp->ct0);
    g1_mul(resp->ct0, params->g1, r);

    /*sigma2 = g2^t*/
    g2_null(resp->sigma2);
    g2_new(resp->sigma2);
    g2_mul(resp->sigma2, params->g2, t);


    bn_free(p);
    bn_free(r);
    bn_free(t);
    g1_free(product);
    for(int i=0; i < req->l; i++){
        g1_free(alpha[i]);
    }
    free(alpha);
}

void signer_write_response_to_file(signer_response_t *resp)
{
    FILE *file = fopen(SIGNER_RESPONSE_FILE, "wb");

    if(file == NULL){
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    /* Écriture de la longueur l */
    fwrite(&resp->l, sizeof(int), 1, file);


    /* Écriture des ct_i,0 et ct_i,1 */
    for(int i = 0; i < resp->l; i++){

        if(!write_g1(file, resp->ct0_i[i])){
            fprintf(stderr, "Erreur écriture ct0_i[%d]\n", i);
            fclose(file);
            return;
        }

        if(!write_g1(file, resp->ct1_i[i])){
            fprintf(stderr, "Erreur écriture ct1_i[%d]\n", i);
            fclose(file);
            return;
        }
    }

    /* Écriture de ct0 */
    if(!write_g1(file, resp->ct0)){
        fprintf(stderr, "Erreur écriture ct0\n");
        fclose(file);
        return;
    }

    /* Écriture de sigma2 */
    if(!write_g2(file, resp->sigma2)){
        fprintf(stderr, "Erreur écriture sigma2\n");
        fclose(file);
        return;
    }

    fclose(file);
}

void signer_ot_response_free(signer_response_t *resp)
{
    for (int i = 0; i < resp->l; i++) {
        g1_free(resp->ct0_i[i]);
        g1_free(resp->ct1_i[i]);
    }
    free(resp->ct0_i);
    free(resp->ct1_i);
    g1_free(resp->ct0);
    g2_free(resp->sigma2);
}

