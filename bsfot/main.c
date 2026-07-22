#include <stdio.h>

#include "benchmark.h"


int main(void)
{
    benchmark_result_t result = {0};


    if (benchmark_run(&result) != 0)
    {
        printf("Erreur protocole\n");
        return -1;
    }


    benchmark_print(&result);


    return 0;
}