#ifndef BENCHMARK_H
#define BENCHMARK_H


/**
 * Résultats des mesures de performance du protocole.
 *
 * Contient les temps d'exécution des différentes phases
 * du protocole ainsi que les temps totaux calculés.
 */
typedef struct {

    double params_ms;      /** Temps de génération des paramètres publics (ms) */
    double ot_ms;          /** Temps de génération des paires OT (ms) */
    double keygen_ms;      /** Temps de génération des clés du signer (ms) */
    double signer_ms;      /** Temps de calcul de la réponse du signer (ms) */
    double signature_ms;   /** Temps de génération de la signature (ms) */
    double verify_ms;      /** Temps de vérification de la signature (ms) */

    double crypto_ms;      /** Temps total des opérations cryptographiques (ms) */
    double total_ms;       /** Temps total d'exécution du protocole (ms) */

} benchmark_result_t;


/**
 * Exécute le protocole et mesure ses performances.
 *
 * Lance le protocole dans le mode spécifié et enregistre
 * les différents temps d'exécution dans la structure résultat.
 *
 * @param result Structure où stocker les mesures obtenues.
 * @param mode Version du protocole à exécuter.
 *
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int benchmark_run(benchmark_result_t *result, int mode);


/**
 * Affiche les résultats du benchmark.
 *
 * Affiche les temps mesurés pour chaque étape du protocole
 * ainsi que les temps totaux.
 *
 * @param result Résultats du benchmark à afficher.
 */
void benchmark_print(const benchmark_result_t *result);


#endif
