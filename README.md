# Compilation et exécution

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
│   ├── ot_keys.bin             # Clés OT générées par l'utilisateur
│   ├── signer_response.bin     # Réponse du signer
│   └── signature.bin           # Signature finale
│
├── main.c                      # main
├── makefile                    # makefile
│
├── include/                    
│   ├── benchmark.h             # Mesures de performances
│   ├── config.h                # Constantes globales
│   ├── io.h                    # Lecture / écriture des données
│   ├── keys.h                  # Génération des clés
│   ├── message.h               # Génération des messages
│   ├── params.h                # Paramètres publics
│   ├── timer.h                 # Chrono
│   ├── run.h                   # Pipeline du protocole
│   ├── signer.h                # Fonctions du signer
│   ├── user.h                  # Fonctions de user
│   ├── waters.h                # Fonction de hachage de Waters
│   └── verifier.h              # Vérification des signatures
│
└── src/                        
    ├── benchmark.c             
    ├── io.c                    
    ├── keys.c                  
    ├── message.c               
    ├── params.c                
    ├── timer.c                 
    ├── run.c                   
    ├── signer.c                
    ├── user.c                  
    ├── waters.c                
    └── verifier.c              
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
