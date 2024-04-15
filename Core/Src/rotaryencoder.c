#include "stdbool.h"

#include "rotaryencoder.h"

struct RotaryEncoderState encoder;

void RotaryEncoder_Init() {
    encoder.position = 0;
    encoder.prev_position = 0;
    encoder.last_pressed = 0;
    encoder.last_released = 0;
    encoder.pressed = false;
}

bool button_debounce(uint32_t curmilli) {
    uint8_t debounce_time = 50;
    return curmilli - encoder.last_pressed > debounce_time
            && curmilli - encoder.last_released > debounce_time;
}

enum RotaryRotation get_rotate_dir(struct RotaryEncoderState encoder) {
    if (encoder.prev_position == 0 && encoder.position == 65532) {
        return DEC_ROTATION;
    }
    if (encoder.prev_position == 65532 && encoder.position == 0) {
        return INC_ROTATION;
    }
    if (encoder.position > encoder.prev_position) {
        return INC_ROTATION;
    }
    if (encoder.position < encoder.prev_position) {
        return DEC_ROTATION;
    }
    return NONE;
}

void RotaryEncoder_Rotated(uint32_t position, RotatedCallback on_inc,
        RotatedCallback on_dec) {

    encoder.position = position;
    enum RotaryRotation dir = get_rotate_dir(encoder);
    encoder.prev_position = position;

    if (dir == INC_ROTATION) {
        on_inc();
    } else if (dir == DEC_ROTATION) {
        on_dec();
    }
}

void RotaryEncoder_ButtonPressed(uint32_t curmilli, ClickedCallback on_click,
        DoubleClickedCallback on_double_click) {

    if (encoder.pressed || !button_debounce(curmilli)) {
        return;
    }

    if (curmilli - encoder.last_pressed < 300) {
        on_double_click(curmilli);
    } else {
        on_click(curmilli);
    }

    encoder.last_pressed = curmilli;
    encoder.pressed = true;
}

void RotaryEncoder_ButtonReleased(uint32_t curmilli) {
    encoder.last_released = curmilli;
    encoder.pressed = false;
}

void RotaryEncoder_Sync(uint32_t curmilli, bool button_set) {
    if (encoder.pressed && button_set && button_debounce(curmilli)) {
        RotaryEncoder_ButtonReleased(curmilli);
    }
}
