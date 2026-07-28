#ifndef CONFIG_H
#define CONFIG_H


/* Fichiers du protocole classique (ElGamal OT) */
#define CLASSIC_OT_KEYS_FILE           "outputs/classic_ot_keys.bin"
#define CLASSIC_SIGNER_RESPONSE_FILE   "outputs/classic_signer_response.bin"
#define CLASSIC_SIGNATURE_FILE         "outputs/classic_signature.bin"


/* Fichiers du protocole Dual-Mode OT */
#define DUALMOD_OT_KEYS_FILE           "outputs/dualmod_ot_keys.bin"
#define DUALMOD_SIGNER_RESPONSE_FILE   "outputs/dualmod_signer_response.bin"
#define DUALMOD_SIGNATURE_FILE         "outputs/dualmod_signature.bin"


/* Taille du message signé, en bits*/
#define MESSAGE_LENGTH 2048

/* structure comportant les différents mode de protocole disponibles*/
typedef enum {
    PROTOCOL_CLASSIC = 0,
    PROTOCOL_DUALMOD = 1
} protocol_mode_t;



#endif 