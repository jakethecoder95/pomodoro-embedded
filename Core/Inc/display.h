#pragma once

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "ssd1306.h"
#include "pomodoro.h"

struct DisplayState {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint32_t totalmilli;
    enum PomodoroTimer type;
};

/**
 * @brief  Initializes the display.
 * @param  None
 * @retval None
 */
void Display_Init(struct PomodoroState pom);

/**
 * @brief  Ensures the screen is displaying the correct state of the pomodoro clock.
 * @param  None
 * @retval None
 */
void Display_Sync(struct PomodoroState pom, uint32_t curmilli);

/* C++ detection */
#ifdef __cplusplus
}
#endif
