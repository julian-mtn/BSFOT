
#include "verifier.h"
#include "io.h"
#include "waters.h"

int verifier_read_classic_signature_from_file(signature_t *sig, const char *filename){

    FILE *file = fopen(filename, "rb");

    if (file == NULL){
        perror("Erreur lors de l'ouverture du fichier");
        return 0;
    }


    g1_null(sig->fm);
    g1_new(sig->fm);

    if (!read_g1(file, sig->fm)){
        printf("Erreur fm\n");
        fclose(file);
        return 0;
    }


    g1_null(sig->sigma1);
    g1_new(sig->sigma1);

    if (!read_g1(file, sig->sigma1)){
        printf("Erreur sigma1\n");
        fclose(file);
        return 0;
    }


    g2_null(sig->sigma2);
    g2_new(sig->sigma2);

    if (!read_g2(file, sig->sigma2)){
        printf("Erreur sigma2\n");
        fclose(file);
        return 0;
    }


    fclose(file);

    return 1;
}

int verifier_read_dualmod_signature_from_file(signature_t *sig, const char *filename){

    FILE *file = fopen(filename, "rb");

    if (file == NULL){
        perror("Erreur lors de l'ouverture du fichier");
        return 0;
    }


    g1_null(sig->fm);
    g1_new(sig->fm);

    if (!read_g1(file, sig->fm)){
        printf("Erreur fm\n");
        fclose(file);
        return 0;
    }


    g1_null(sig->sigma1);
    g1_new(sig->sigma1);

    if (!read_g1(file, sig->sigma1)){
        printf("Erreur sigma1\n");
        fclose(file);
        return 0;
    }


    g2_null(sig->sigma2);
    g2_new(sig->sigma2);

    if (!read_g2(file, sig->sigma2)){
        printf("Erreur sigma2\n");
        fclose(file);
        return 0;
    }


    g1_null(sig->sigma2_prime);
    g1_new(sig->sigma2_prime);

    if (!read_g1(file, sig->sigma2_prime)){
        printf("Erreur sigma2_prime\n");
        fclose(file);
        return 0;
    }


    fclose(file);

    return 1;
}


int verifier_check_classic_signature(const signature_t *sig, public_params_t *params, const signer_public_key_t *pk, uint8_t *message){
    
    /*F(M) */
    g1_t fm;
    g1_null(fm);
    g1_new(fm);

    waters_hash(fm, params->u, params->l, message);


    /* gauche : e(h_s, bvk) * e(F(M), sigma2)*/
    gt_t left;
    gt_null(left);
    gt_new(left);

    gt_t tmp;
    gt_null(tmp);
    gt_new(tmp);


    pc_map(left, params->hs, pk->bvk); //e(h_s, bvk)

    pc_map(tmp, fm, sig->sigma2); //e(F(M), sigma2)
    gt_mul(left, left, tmp);


    /* droite : e(sigma1, g2) */
    gt_t right;
    gt_null(right);
    gt_new(right);

    pc_map(right, sig->sigma1, params->g2); //e(sigma1, g2)

  
    int valid = 0;

    /*
    printf("left: ");
    gt_print(left);
    printf("right: ");
    gt_print(right);
    */
   
    if (gt_cmp(left, right) == RLC_EQ) {
        valid = 1;
    }



    g1_free(fm);
    gt_free(left);
    gt_free(tmp);
    gt_free(right);


    return valid;
}

int verifier_check_dualmod_signature(const signature_t *sig, public_params_t *params, const signer_public_key_t *pk, uint8_t *message){

    /* F(M) */
    g1_t fm;
    g1_null(fm);
    g1_new(fm);

    waters_hash(fm, params->u, params->l, message);


    /* e(sigma2_prime, g2) =? e(g1, sigma2) */

    gt_t left1;
    gt_t right1;
    gt_null(left1);
    gt_null(right1);
    gt_new(left1);
    gt_new(right1);

    pc_map(left1, sig->sigma2_prime, params->g2);
    pc_map(right1, params->g1, sig->sigma2);


    printf("\n[DEBUG] Vérification sigma2_prime\n");

    if(gt_cmp(left1, right1) != RLC_EQ){

        printf("[DEBUG] FAIL : e(sigma2_prime, g2) != e(g1, sigma2)\n");

        g1_free(fm);
        gt_free(left1);
        gt_free(right1);

        return 0;
    }

    else {

        printf("[DEBUG] OK : e(sigma2_prime, g2) == e(g1, sigma2)\n");

    }


    /* e(sigma1,g2) =? e(hs,bvk) * e(F(M),sigma2) */


    gt_t left2;
    gt_t right2;
    gt_t tmp;

    gt_null(left2);
    gt_null(right2);
    gt_null(tmp);

    gt_new(left2);
    gt_new(right2);
    gt_new(tmp);


    pc_map(left2, sig->sigma1, params->g2);

    pc_map(right2, params->hs, pk->bvk);

    pc_map(tmp, fm, sig->sigma2);

    gt_mul(right2, right2, tmp);


    printf("[DEBUG] Vérification sigma1\n");


    int valid = 0;

    if(gt_cmp(left2, right2) == RLC_EQ){

        printf("[DEBUG] OK : e(sigma1,g2) == e(hs,bvk)*e(F(M),sigma2)\n");

        valid = 1;
    }

    else {

        printf("[DEBUG] FAIL : e(sigma1,g2) != e(hs,bvk)*e(F(M),sigma2)\n");

    }


    g1_free(fm);

    gt_free(left1);
    gt_free(right1);

    gt_free(left2);
    gt_free(right2);
    gt_free(tmp);


    return valid;
}