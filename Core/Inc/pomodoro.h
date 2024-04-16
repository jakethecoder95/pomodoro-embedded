#pragma once

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

enum PomodoroMode {
    FOCUS_SELECT = 0, REST_SELECT = 1, FOCUSING = 2, RESTING = 3
};

enum PomodoroTimer {
    FOCUS, REST
};

typedef struct PomodoroState {
    int focus;
    int rest;
    enum PomodoroMode mode;

    bool paused;
    uint32_t elapsed_time;
    uint32_t prev_time;
};


/**
 * @brief  Initializes the display.
 * @param  None
 * @retval None
 */
void Pomodoro_Init(int focus, int rest, uint32_t curmilli);

/**
 * @brief  Handle Rest Pomodoro State.
 * @param  curmilli is the current millisecond that the reset occurred.
 * @retval None
 */
void Pomodoro_Reset(uint32_t curmilli);

/**
 * @brief  Handle select the current pomodoro setting for the mode that is active.
 * @param  curmilli is the current millisecond that the selected occurred.
 * @retval None
 */
void Pomodoro_Select(uint32_t curmilli);

/**
 * @brief  Increase the focus or rest time.
 * @param  None
 * @retval None
 */
void Pomodoro_IncTime();

/**
 * @brief  Decrease the focus or rest time.
 * @param  None
 * @retval None
 */
void Pomodoro_DecTime();

/**
 * @brief  Synchronizes pomodoro state every tick.
 * @param  None
 * @retval None
 */
void Pomodoro_Sync(uint32_t curmilli);

/* C++ detection */
#ifdef __cplusplus
}
#endif
