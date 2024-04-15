#pragma once

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern const uint32_t MILLI_HOUR;
extern const uint32_t MILLI_MIN;
extern const uint32_t MILLI_SEC;

/**
 * @brief  Converts minutes to milliseconds.
 * @param  None
 * @retval None
 */
uint32_t Util_ConvertMinToMilli(int minutes);

/* C++ detection */
#ifdef __cplusplus
}
#endif
