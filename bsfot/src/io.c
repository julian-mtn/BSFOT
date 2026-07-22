#include "io.h"

#include <stdlib.h>

int write_g1(FILE *file, g1_t P) {
    int size = g1_size_bin(P, 1);

    if (fwrite(&size, sizeof(int), 1, file) != 1)
        return 0;

    uint8_t *buffer = malloc(size);
    if (buffer == NULL)
        return 0;

    g1_write_bin(buffer, size, P, 1);

    int ok = (fwrite(buffer, 1, size, file) == (size_t)size);

    free(buffer);

    return ok;
}

int read_g1(FILE *file, g1_t P){

    int size;

    if (fread(&size, sizeof(int), 1, file) != 1)
        return 0;

    uint8_t *buffer = malloc(size);
    if (buffer == NULL)
        return 0;

    if (fread(buffer, 1, size, file) != (size_t)size) {
        free(buffer);
        return 0;
    }

    g1_read_bin(P, buffer, size);

    free(buffer);

    return 1;
}

int write_g2(FILE *file, g2_t P){

    int size = g2_size_bin(P, 1);

    if (fwrite(&size, sizeof(int), 1, file) != 1)
        return 0;

    uint8_t *buffer = malloc(size);
    if (buffer == NULL)
        return 0;

    g2_write_bin(buffer, size, P, 1);

    int ok = (fwrite(buffer, 1, size, file) == (size_t)size);

    free(buffer);

    return ok;
}

int read_g2(FILE *file, g2_t P){

    int size;

    if (fread(&size, sizeof(int), 1, file) != 1)
        return 0;

    uint8_t *buffer = malloc(size);
    if (buffer == NULL)
        return 0;

    if (fread(buffer, 1, size, file) != (size_t)size) {
        free(buffer);
        return 0;
    }

    g2_read_bin(P, buffer, size);

    free(buffer);

    return 1;
}

int write_bn(FILE *file, bn_t x){

    int size = bn_size_bin(x);

    if (fwrite(&size, sizeof(int), 1, file) != 1)
        return 0;

    uint8_t *buffer = malloc(size);
    if (buffer == NULL)
        return 0;

    bn_write_bin(buffer, size, x);

    int ok = (fwrite(buffer, 1, size, file) == (size_t)size);

    free(buffer);

    return ok;
}

int read_bn(FILE *file, bn_t x){
    
    int size;

    if (fread(&size, sizeof(int), 1, file) != 1)
        return 0;

    uint8_t *buffer = malloc(size);
    if (buffer == NULL)
        return 0;

    if (fread(buffer, 1, size, file) != (size_t)size) {
        free(buffer);
        return 0;
    }

    bn_read_bin(x, buffer, size);

    free(buffer);

    return 1;
}