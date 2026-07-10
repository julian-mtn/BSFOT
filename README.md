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

## Étapes de génération de la signature

User
│
├── user_ot_init()
│      Génération des clés OT
│
├── user_write_ot_keys_to_file()
│      └── outputs/ot_keys.bin
│
▼

Signer
│
├── signer_read_ot_request_from_file()
│
├── signer_compute_response()
│
├── signer_write_response_to_file()
│      └── outputs/signer_response.bin
│
▼

User
│
├── user_read_signer_response_from_file()
│
├── user_compute_signature()
│
├── user_write_signature_to_file()
│      └── outputs/signature.bin
│
▼

Verifier
│
├── verifier_read_signature_from_file()
│
└── verifier_check_signature()
       └── Signature valide / invalide
