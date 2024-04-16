#include <stdbool.h>
#include <string.h>

#include "display.h"

#include "util.h"

struct DisplayState focusstate;
struct DisplayState reststate;

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 32;

const int Y = 9;
const int FOCUS_X = 10;
const int REST_X = (SCREEN_WIDTH / 2) + 10;

void int_to_str(int num, char *str) {
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

/**
 * @brief  Updates the provided state to match the supplied timemilli.
 * @retval A boolean representing if the state was updated or not.
 */
bool update_dstate(struct DisplayState *state, uint32_t timemilli) {

    if (state->totalmilli / MILLI_SEC == timemilli / MILLI_SEC) {
        return false;
    }

    uint32_t time = timemilli;
    state->totalmilli = timemilli;

    state->hours = time / MILLI_HOUR;
    time -= state->hours * MILLI_HOUR;
    state->minutes = time / MILLI_MIN;
    time -= state->minutes * MILLI_MIN;
    state->seconds = time / MILLI_SEC;

    return true;
}

void render_smart_line(struct DisplayState dstate, struct PomodoroState pom,
        uint32_t curmilli) {

    uint8_t color = 1;
    uint16_t y = Y + 21;
    uint16_t width = 4 * 11;
    uint16_t height = 2;

    if (dstate.type == FOCUS && (pom.mode == REST_SELECT || pom.mode == RESTING)) {
        SSD1306_DrawRectangle(FOCUS_X, y, width, height, 0);
        return;
    }
    if (dstate.type == REST && (pom.mode == FOCUS_SELECT || pom.mode == FOCUSING)) {
        SSD1306_DrawRectangle(REST_X, y, width, height, 0);
        return;
    }

    uint16_t x;
    if (pom.mode == FOCUS_SELECT || pom.mode == FOCUSING) {
        x = FOCUS_X;
    } else {
        x = REST_X;
    }
    SSD1306_DrawRectangle(x, y, width, height, 0);

    if (pom.mode == FOCUS_SELECT || pom.mode == REST_SELECT) {
        bool is_odd = ((curmilli % 1000) / 500) % 2 == 1;
        if (is_odd) {
            color = 0;
        }
    }

    if (!pom.paused && (pom.mode == FOCUSING || pom.mode == RESTING)) {
        uint16_t num = curmilli % 1000 / 25;

        if (num <= 10) {
            x += (width / 10) * num;
            width -= (width / 10) * num;
        } else if (num > 10 && num <= 20) {
            num = 10 - (num - 10);
            x += (width / 10) * num;
            width -= (width / 10) * num;
        } else if (num > 20 && num <= 30) {
            num -= 20;
            width -= (width / 10) * num;
        } else if (num > 10 && num <= 40) {
            num = 10 - (num - 30);
            width -= (width / 10) * num;
        }
    }

    SSD1306_DrawRectangle(x, y, width, height, color);
}

void render_dstate_xy(struct DisplayState dstate, int x, int y) {
    char str[5] = { 0 };
    int index = 0;

    if (dstate.hours > 0) {
        char hours[2];
        int_to_str(dstate.hours, hours);
        hours[1] = ':';
        str[index++] = hours[0];
        str[index++] = hours[1];
    } else if (dstate.minutes > 0) {
        strcat(str, "0:");
        index += 2;
    }

    if (dstate.minutes > 0) {
        char mins[2];
        int_to_str(dstate.minutes, mins);
        if (dstate.minutes < 10) {
            mins[1] = mins[0];
            mins[0] = '0';
        }
        str[index++] = mins[0];
        str[index++] = mins[1];
    } else if (dstate.hours > 0) {
        strcat(str, "00");
        index += 2;
    } else {
        // Seconds count down.
        strcat(str, "0:");
        index += 2;
        if (dstate.seconds > 0) {
            char sec[2];
            int_to_str(dstate.seconds, sec);
            if (dstate.seconds < 10) {
                sec[1] = sec[0];
                sec[0] = '0';
            }
            str[index++] = sec[0];
            str[index++] = sec[1];
        } else {
            strcat(str, "00");
            index += 2;
        }
    }

    SSD1306_DrawFilledRectangle(x, y, 11 * 4, 18, 0); // Clear existing number.
    SSD1306_GotoXY(x, y);
    SSD1306_Puts(str, &SSD1306_Font_11x18, 1);
}

void Display_Init(struct PomodoroState pom) {
    focusstate.type = FOCUS;
    reststate.type = REST;
    update_dstate(&focusstate, 0);
    update_dstate(&reststate, 0);
}

void Display_Sync(struct PomodoroState pom, uint32_t curmilli) {

    bool focus_did_change = false;
    bool rest_did_change = false;

    uint32_t focusmilli = Util_ConvertMinToMilli(pom.focus);
    uint32_t restmilli = Util_ConvertMinToMilli(pom.rest);

    if (pom.mode == FOCUS_SELECT || pom.mode == REST_SELECT) {
        focus_did_change = update_dstate(&focusstate, focusmilli);
        rest_did_change = update_dstate(&reststate, restmilli);
    } else if (pom.mode == FOCUSING) {
        focus_did_change = update_dstate(&focusstate,
                focusmilli - pom.elapsed_time);
        rest_did_change = update_dstate(&reststate, restmilli);
    } else {
        focus_did_change = update_dstate(&focusstate, focusmilli);
        rest_did_change = update_dstate(&reststate,
                restmilli - pom.elapsed_time);
    }

    render_smart_line(focusstate, pom, curmilli);
    render_smart_line(reststate, pom, curmilli);

    if (focus_did_change || rest_did_change) {
        render_dstate_xy(focusstate, FOCUS_X, Y);
        render_dstate_xy(reststate, REST_X, Y);
    }

    SSD1306_UpdateScreen();
}
