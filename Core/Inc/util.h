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

/**
 * @brief  Converts a number into a string.
 * @param  num is the number to be converted.
 * @param  str is the string to be updated.
 * @retval None
 */
void Util_IntToStr(int num, char *str);

/* C++ detection */
#ifdef __cplusplus
}
#endif
