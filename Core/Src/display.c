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

int render_count;
uint16_t prev_line_color;
uint16_t prev_line_y;
uint16_t prev_line_x;
uint16_t prev_line_width;
uint16_t prev_line_height;

/**
 * @brief  Updates the provided state to match the supplied timemilli.
 * @retval A boolean representing if the state was updated or not.
 */
bool update_dstate(struct DisplayState *state, uint32_t timemilli) {
    bool did_update = false;
    uint32_t time = timemilli;

    state->totalmilli = timemilli;

    int new_hours = time / MILLI_HOUR;
    if (new_hours != state->hours) {
        did_update = true;
    }
    state->hours = new_hours;
    time -= state->hours * MILLI_HOUR;

    int new_minutes = time / MILLI_MIN;
    if (new_minutes != state->minutes) {
        did_update = true;
    }
    state->minutes = new_minutes;
    time -= state->minutes * MILLI_MIN;

    int new_seconds = time / MILLI_SEC;
    if (new_seconds != state->seconds) {
        did_update = true;
    }
    state->seconds = time / MILLI_SEC;

    return did_update;
}

bool render_smart_line(struct DisplayState dstate, struct PomodoroState pom,
        uint32_t curmilli) {

    uint8_t color = 1;
    uint16_t y = Y + 21;
    uint16_t width = 4 * 11;
    uint16_t height = 2;

    uint16_t x;
    if (pom.mode == FOCUS_SELECT || pom.mode == FOCUSING) {
        x = FOCUS_X;
    } else {
        x = REST_X;
    }

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

    if (x == prev_line_x && y == prev_line_y && width == prev_line_width
            && height == prev_line_height
            && color == prev_line_color) {
        return false;
    }

    SSD1306_DrawRectangle(prev_line_x, prev_line_y, prev_line_width, prev_line_height, 0);

    prev_line_color = color;
    prev_line_x = x;
    prev_line_y = y;
    prev_line_width = width;
    prev_line_height = height;

    SSD1306_DrawRectangle(x, y, width, height, color);

    return true;
}

void render_dstate_xy(struct DisplayState dstate, int x, int y) {
    char str[5] = { 0 };
    int index = 0;

    if (dstate.hours > 0) {
        char hours[2];
        Util_IntToStr(dstate.hours, hours);
        hours[1] = ':';
        str[index++] = hours[0];
        str[index++] = hours[1];
    } else if (dstate.minutes > 0) {
        strcat(str, "0:");
        index += 2;
    }

    if (dstate.minutes > 0) {
        char mins[2];
        Util_IntToStr(dstate.minutes, mins);
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
            Util_IntToStr(dstate.seconds, sec);
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

    bool did_time_update = focus_did_change || rest_did_change;
    if (did_time_update) {
        render_dstate_xy(focusstate, FOCUS_X, Y);
        render_dstate_xy(reststate, REST_X, Y);
    }

    bool did_line_update = render_smart_line(focusstate, pom, curmilli);

    if (did_line_update || did_time_update) {
        render_count++;
        SSD1306_UpdateScreen();
    }
}
