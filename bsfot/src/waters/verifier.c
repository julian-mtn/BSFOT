
#include "waters/verifier.h"
#include "common/io.h"
#include "waters/waters.h"

int verifier_read_signature_from_file(signature_t *sig) {

    FILE *file = fopen(SIGNATURE_FILE, "rb");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return 0;
    }

    

    g1_null(sig->fm);
    g1_new(sig->fm);

    if (!read_g1(file, sig->fm)) {
        printf("Erreur fm\n");
        fclose(file);
        return 0;
    }

    g1_null(sig->sigma1);
    g1_new(sig->sigma1);

    if (!read_g1(file, sig->sigma1)) {
        printf("Erreur sigma1\n");
        fclose(file);
        return 0;
    }

    g2_null(sig->sigma2);
    g2_new(sig->sigma2);

    if (!read_g2(file, sig->sigma2)) {
        printf("Erreur sigma2\n");
        fclose(file);
        return 0;
    }


    fclose(file);
    return 1;
}

int verifier_check_signature(const signature_t *sig, public_params_t *params, const signer_public_key_t *pk, uint8_t *message)
{
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