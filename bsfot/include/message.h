#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>

/**
 * Génère un message binaire aléatoire.
 *
 * @param message Tableau de sortie contenant les bits du message.
 * @param length Longueur du message en bits.
 */
void generate_random_message(uint8_t *message, int length);

#endif
