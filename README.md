## Compilation & Exécution

Le projet peut être compilé et exécuté de deux manières selon l'utilisation voulu.

| Commande | Description |
|----------|-------------|
| `make` | Compile le projet avec la courbe RELIC sélectionnée (`bn254` par défaut). |
| `make CURVE=<courbe>` | Compile le projet avec une autre installation de RELIC (ex. `bls381`, `kss18-638`). |
| `./main <mode>` | Exécute le protocole pour le mode choisi. |
| `sudo ./run_benchmarks.sh <courbe1> [<courbe2> ...]` | Installe automatiquement les courbes RELIC demandées si elles ne sont pas déjà présentes, compile le projet pour chacune d'elles, exécute le protocole et enregistre les résultats dans le dossier `results/`. |

### Exécution du protocole

Le programme implémente un protocole de **Blind Signature** basé sur les signatures de Waters. Selon le mode choisi, l'échange entre l'utilisateur et le signer utilise soit un **Classic Oblivious Transfer**, soit un **Dual-Mode Oblivious Transfer**.

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

```bash
make CURVE=bls381
./main 1
```

### Benchmarks automatiques

Le script `run_benchmarks.sh` permet d'exécuter automatiquement le protocole sur une ou plusieurs courbes RELIC.

Pour chaque courbe passée en paramètre, le script :

1. vérifie si la bibliothèque RELIC correspondante est installée 
2. installe automatiquement la courbe si nécessaire 
3. compile le projet avec cette courbe 
4. exécute le protocole 
5. enregistre les mesures de performances dans le dossier `results/`.

Exemple :

```bash
sudo ./run_benchmarks.sh bn254 bls381 kss18-638
```

Les fichiers de résultats sont ensuite disponibles dans le dossier :

```text
results/
```

### Nettoyage

Supprimer les fichiers générés :

```bash
make clean
```

## Structure générale du projet

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
│   ├── *.log                       # Temps d'exécution par courbe
│   └── install_logs/               # Logs de RELIC
│
├── main.c                          # main
├── makefile                        # makefile
├── run_benchmarks.sh               # Compilation et benchmarks multi-courbes
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
