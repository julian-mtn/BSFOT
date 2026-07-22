#include "message.h"

#include <stdlib.h>
#include <stdint.h>


void generate_random_message(uint8_t *message, int length)
{
    for (int i = 0; i < length; i++) {
        message[i] = rand() % 2;
    }
}
