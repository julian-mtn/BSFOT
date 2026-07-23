#include <stdio.h>

#include "benchmark.h"


int main(void)
{
    int mode = 0;
    benchmark_result_t result = {0};

    switch (mode)
    {
        case 0:
            printf("Mode 0 : Classic OT\n");
            if (benchmark_run(&result, 0) != 0)
            {
                printf("Erreur protocole 0\n");
                return -1;
            }
            benchmark_print(&result);
            break;

        case 1:
            printf("Mode 1 : Dual-Mode OT\n");
            if (benchmark_run(&result, 1) != 0)
            {
                printf("Erreur protocole 1\n");
                return -1;
            }
            benchmark_print(&result);
            break;

        default:
            printf("Mode inconnu\n");
            break;
    }

    

    return 0;
}
