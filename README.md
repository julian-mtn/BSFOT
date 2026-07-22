## Compilation et exécution

Compiler le projet :

```bash
make
```

Exécuter le programme :

```bash
./main
```

Supprimer les fichiers générés :

```bash
make clean
```
## Structure générale du projet

```text
bsfot/
├── build/                      # Fichiers objets générés
├── outputs/                    # Fichiers échangés lors des communications
│
├── main.c                      # main
├── makefile
│
├── include/
│   ├── common/
│   │   ├── benchmark.h         # Mesures de performances
│   │   ├── config.h            # Constantes globales
│   │   ├── io.h                # Lecture / écriture à travers les fichiers 
│   │   ├── keys.h              # Génération des clés 
│   │   ├── params.h            # Paramètres publics
│   │   └── timer.h             # Chrono
│   │
│   ├── waters/
│   │   ├── verifier.h          # Vérification des signatures
│   │   └── waters.h            # Fonction de hachage de Waters
│   │
│   ├── ot_classic/
│   │   ├── run.h               # Pipeline OT classique
│   │   ├── signer.h            # Côté signer
│   │   └── user.h              # Côté user
│   │
│   └── ot_dual/
│       ├── run.h               # Pipeline Dual-Mode OT
│       ├── signer.h            # Côté signer
│       └── user.h              # Côté user
│
└── src/
    ├── common/
    │   ├── benchmark.c
    │   ├── io.c
    │   ├── keys.c
    │   ├── params.c
    │   └── timer.c
    │
    ├── waters/
    │   ├── verifier.c
    │   └── waters.c
    │
    ├── ot_classic/
    │   ├── run.c
    │   ├── signer.c
    │   └── user.c
    │
    └── ot_dual/
        ├── run.c
        ├── signer.c
        └── user.c
```

## Étapes de génération de la signature

```text
User
 ├─ user_ot_init()
 │    └─ Génération des clés OT
 ├─ user_write_ot_keys_to_file()
 │    └─ outputs/ot_keys.bin
 │
 ▼

Signer
 ├─ signer_read_ot_request_from_file()
 ├─ signer_compute_response()
 ├─ signer_write_response_to_file()
 │    └─ outputs/signer_response.bin
 │
 ▼

User
 ├─ user_read_signer_response_from_file()
 ├─ user_compute_signature()
 ├─ user_write_signature_to_file()
 │    └─ outputs/signature.bin
 │
 ▼

Verifier
 ├─ verifier_read_signature_from_file()
 └─ verifier_check_signature()
      └─ Signature valide / invalide
```
