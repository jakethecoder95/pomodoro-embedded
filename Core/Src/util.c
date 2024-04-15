#include <stdint.h>

#include "util.h"

const uint32_t MILLI_HOUR = 3600000;
const uint32_t MILLI_MIN = 60000;
const uint32_t MILLI_SEC = 1000;

uint32_t Util_ConvertMinToMilli(int minutes) {
    uint32_t int32_min = (uint32_t)minutes;
    return int32_min * MILLI_MIN;
};
