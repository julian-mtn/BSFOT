## Benchmark results

![Benchmark results](results/images/benchmark.png)


## Compilation & Exécution

Le projet peut être compilé et exécuté de deux manières :

| Commande | Description |
|----------|-------------|
| `make` | Compile le projet avec la courbe RELIC sélectionnée (`bn254` par défaut). |
| `make CURVE=<courbe>` | Compile le projet avec une autre installation de RELIC (ex. `bls381`, `kss18-638`). |
| `./main <mode>` | Exécute le protocole pour le mode choisi. |
| `./run_benchmarks.sh` | Lance automatiquement les benchmarks sur toutes les courbes définies dans `curves.txt`. |

Le fichier `curves.txt` contient la liste des courbes RELIC utilisées pour les benchmarks.

Exemple :

```text
bn254
bls12-381
kss18-638
```

Chaque courbe correspond directement au nom du preset RELIC utilisé lors de l'installation.


## Exécution du protocole

Le programme implémente un protocole de **Blind Signature** basé sur les signatures de Waters.

Le protocole simule l'échange entre un utilisateur et un signer. Selon le mode choisi, la génération de la signature utilise soit un **Classic Oblivious Transfer**, soit un **Dual-Mode Oblivious Transfer**.

Modes disponibles :

| Mode | Protocole |
|------|-----------|
| `0` | Classic OT |
| `1` | Dual-Mode OT |

Exemples :

```bash
make
./main 0
```

Exécute le protocole avec Classic OT.


```bash
make CURVE=bls12-381
./main 1
```

Exécute le protocole avec Dual-Mode OT sur la courbe BLS12-381.


## Benchmarks automatiques

Le script `run_benchmarks.sh` permet d'automatiser l'évaluation des performances sur plusieurs courbes RELIC.

Les courbes testées sont définies dans le fichier :

```text
curves.txt
```

Pour chaque courbe, le script :

1. Vérifie si l'installation RELIC correspondante existe.
2. Installe automatiquement la courbe si nécessaire dans `/opt/`.
3. Compile le projet avec cette courbe.
4. Exécute le protocole.
5. Enregistre les temps d'exécution.
6. Génère automatiquement des graphiques de comparaison.

Lancement :

```bash
./run_benchmarks.sh
```

Le script demande les droits administrateur uniquement lorsqu'une installation RELIC dans `/opt/` est nécessaire.

Les résultats sont organisés dans :

```text
results/
├── logs/        # Logs d'exécution, compilation et installation RELIC
└── images/      # Graphiques générés automatiquement
```


## Nettoyage

Supprimer les fichiers générés par la compilation :

```bash
make clean
```


## Structure générale du projet

```text
bsfot/
├── build/                          # Fichiers objets générés
├── outputs/                        # Fichiers échangés lors des communications
│   ├── classic_ot_keys.bin         # Clés OT générées par l'utilisateur (Classic OT)
│   ├── classic_signer_response.bin # Réponse du signer (Classic OT)
│   ├── classic_signature.bin       # Signature finale (Classic OT)
│   ├── dualmod_ot_keys.bin         # Clés OT générées par l'utilisateur (Dual-Mode OT)
│   ├── dualmod_signer_response.bin # Réponse du signer (Dual-Mode OT)
│   └── dualmod_signature.bin       # Signature finale (Dual-Mode OT)
│
├── results/                        # Résultats des benchmarks
│   ├── logs/                       # Logs d'exécution, compilation et installation RELIC
│   │   ├── *.log
│   │   ├── build_logs/
│   │   └── install_logs/
│   │
│   └── images/                     # Graphiques générés par display.py
│       ├── total_time.png
│       └── breakdown.png
│
├── curves.txt                      # Liste des courbes RELIC à tester
├── display.py                      # Génération des graphiques de benchmark
├── main.c                          # main
├── makefile                        # makefile
├── run_benchmarks.sh               # Installation RELIC et benchmarks multi-courbes
├── compile_commands.json           # Base de compilation
│
├── include/
│   ├── benchmark.h                 # Mesures de performances
│   ├── config.h                    # Constantes globales
│   ├── io.h                        # Lecture / écriture des données
│   ├── keys.h                      # Génération des clés
│   ├── message.h                   # Génération des messages
│   ├── params.h                    # Paramètres publics
│   ├── timer.h                     # Chrono
│   ├── run.h                       # Pipeline du protocole
│   ├── signer.h                    # Fonctions du signer (Classic OT)
│   ├── signer_dualmode.h           # Fonctions du signer (Dual-Mode OT)
│   ├── user.h                      # Fonctions de l'utilisateur (Classic OT)
│   ├── user_dualmode.h             # Fonctions de l'utilisateur (Dual-Mode OT)
│   ├── waters.h                    # Fonction de hachage de Waters
│   └── verifier.h                  # Vérification des signatures
│
└── src/
    ├── benchmark.c
    ├── io.c
    ├── keys.c
    ├── message.c
    ├── params.c
    ├── run.c
    ├── signer.c
    ├── signer_dualmode.c
    ├── timer.c
    ├── user.c
    ├── user_dualmode.c
    ├── verifier.c
    └── waters.c
```


## Étapes de génération de la signature

```text
User
 ├─ user_ot_init()
 ├─ user_write_ot_keys_to_file()
 │
 ▼

Signer
 ├─ signer_read_ot_request_from_file()
 ├─ signer_compute_response()
 ├─ signer_write_response_to_file()
 │
 ▼

User
 ├─ user_read_signer_response_from_file()
 ├─ user_compute_signature()
 ├─ user_write_signature_to_file()
 │
 ▼

Verifier
 ├─ verifier_read_signature_from_file()
 └─ verifier_check_signature()
```