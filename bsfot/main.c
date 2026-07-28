#include <stdio.h>
#include <stdlib.h>

#include "benchmark.h"
#include "config.h"


int main(int argc, char **argv){

    if (argc < 2) {
        printf("Usage: %s <mode>\n", argv[0]);
        printf("Modes disponibles :\n");
        printf("  0 : Classic OT\n");
        printf("  1 : Dual-Mode OT\n");
        return -1;
    }

    protocol_mode_t mode = atoi(argv[1]);
    benchmark_result_t result = {0};

    switch (mode){

        case PROTOCOL_CLASSIC:
            printf("Mode : Classic OT\n");
            break;

        case PROTOCOL_DUALMOD:
            printf("Mode : Dual-Mode OT\n");
            break;

        default:
            printf("Mode inconnu\n");
            return -1;
    }


    if (benchmark_run(&result, mode) != 0){
        printf("Erreur protocole\n");
        return -1;
    }


    benchmark_print(&result);

    return 0;
}
