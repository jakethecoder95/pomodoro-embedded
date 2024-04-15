#pragma once

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stdbool.h"

#include "pomodoro.h"

enum RotaryRotation {
    DEC_ROTATION = 0, INC_ROTATION = 1, NONE = 2
};

struct RotaryEncoderState {
    uint32_t position;
    uint32_t prev_position;

    bool pressed;
    uint32_t last_pressed;
    uint32_t last_released;
};

typedef void (*ClickedCallback)(uint32_t);
typedef void (*DoubleClickedCallback)(uint32_t);
typedef void (*RotatedCallback)(void);

/**
 * @brief  Initializes the encoder.
 * @param  None
 * @retval None
 */
void RotaryEncoder_Init();

/**
 * @brief  Handle button press events.
 * @param  None
 * @retval None
 */
void RotaryEncoder_ButtonPressed(uint32_t curmilli, ClickedCallback on_click,
        DoubleClickedCallback on_double_click);

/**
 * @brief  Handle encoder rotation events.
 * @param  None
 * @retval None
 */
void RotaryEncoder_Rotated(uint32_t curmilli, RotatedCallback on_inc,
        RotatedCallback on_dec);

/**
 * @brief  Handle button release events.
 * @param  None
 * @retval None
 */
void RotaryEncoder_ButtonReleased(uint32_t curmilli);

/**
 * @brief  Handle sync of encoder state every tick.
 * @param  None
 * @retval None
 */
void RotaryEncoder_Sync(uint32_t curmilli, bool pin_state);

/* C++ detection */
#ifdef __cplusplus
}
#endif
