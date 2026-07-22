#ifndef WATERS_H
#define WATERS_H

#include <stdint.h>

#include <relic/relic.h>
#include <relic/relic_pp.h>

/**
 * Calcule le hachage de Waters
 *
 * F(m) = u0 * Π(u_i^{m_i})
 *
 * @param result Résultat du hachage (F(m))
 * @param u Tableau des paramètres de Waters
 * @param l Longueur du vecteur
 * @param m Message représenté comme un vecteur d'octets
 */
void waters_hash(g1_t result, g1_t *u, int l, uint8_t *m);

#endif 