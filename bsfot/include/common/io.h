#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <relic/relic.h>

/**
 * Écrit un élément de G1 dans un fichier
 *
 * Format :
 *   - taille (int)
 *   - représentation binaire compressée
 *
 * @param file Fichier de destination
 * @param P Point à écrire
 *
 * @return 1 en cas de succès, 0 sinon
 */
int write_g1(FILE *file, const g1_t P);

/**
 * Lit un élément de G1 depuis un fichier
 *
 * @param file Fichier source
 * @param P Point déjà initialisé à remplir
 *
 * @return 1 en cas de succès, 0 sinon
 */
int read_g1(FILE *file, g1_t P);

/**
 * Écrit un élément de G2 dans un fichier
 *
 * @param file Fichier de destination
 * @param P Point à écrire
 *
 * @return 1 en cas de succès, 0 sinon
 */
int write_g2(FILE *file, const g2_t P);

/**
 * Lit un élément de G2 depuis un fichier
 *
 * @param file Fichier source
 * @param P Point déjà initialisé à remplir
 *
 * @return 1 en cas de succès, 0 sinon
 */
int read_g2(FILE *file, g2_t P);

/**
 * Écrit un entier RELIC dans un fichier
 *
 * @param file Fichier de destination
 * @param x Entier à écrire
 *
 * @return 1 en cas de succès, 0 sinon
 */
int write_bn(FILE *file, const bn_t x);

/**
 * Lit un entier RELIC depuis un fichier
 *
 * @param file Fichier source
 * @param x Entier déjà initialisé à remplir
 *
 * @return 1 en cas de succès, 0 sinon
 */
int read_bn(FILE *file, bn_t x);

#endif