#include <stdint.h>

#include "util.h"

const uint32_t MILLI_HOUR = 3600000;
const uint32_t MILLI_MIN = 60000;
const uint32_t MILLI_SEC = 1000;

uint32_t Util_ConvertMinToMilli(int minutes) {
    uint32_t int32_min = (uint32_t)minutes;
    return int32_min * MILLI_MIN;
};

void Util_IntToStr(int num, char *str) {
    int i = 0, j, rem, len = 0, n;

    n = num;
    while (n != 0) {
        len++;
        n /= 10;
    }

    for (j = 0; j < len; j++) {
        rem = num % 10;
        num = num / 10;
        str[len - (j + 1)] = rem + '0';
    }
    str[len] = '\0';
}
