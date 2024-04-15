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

    SSD1306_GotoXY(x, y);
    SSD1306_Puts(str, &SSD1306_Font_11x18, 1);
}

void render_time(uint32_t timemilli) {
    char str[20];
    int_to_str(timemilli, str);

    SSD1306_GotoXY(0, 13);
    SSD1306_Puts(str, &SSD1306_Font_11x18, 1);
}

void Display_Init(struct PomodoroState pom) {
    update_dstate(&focusstate, 0);
    update_dstate(&reststate, 0);
}

void Display_Sync(struct PomodoroState pom) {

    bool focus_did_change = false;
    bool rest_did_change = false;

    uint32_t focusmilli = Util_ConvertMinToMilli(pom.focus);
    uint32_t restmilli = Util_ConvertMinToMilli(pom.rest);

    if (pom.mode == FOCUS_SELECT || pom.mode == REST_SELECT) {
        focus_did_change = update_dstate(&focusstate, focusmilli);
        rest_did_change = update_dstate(&reststate, restmilli);
    } else if (pom.mode == FOCUSING) {
        focus_did_change = update_dstate(&focusstate, focusmilli - pom.elapsed_time);
        rest_did_change = update_dstate(&reststate, restmilli);
    } else {
        focus_did_change = update_dstate(&focusstate, focusmilli);
        rest_did_change = update_dstate(&reststate, restmilli - pom.elapsed_time);
    }

    if (focus_did_change || rest_did_change) {
        SSD1306_Fill(SSD1306_COLOR_BLACK);

        render_dstate_xy(focusstate, FOCUS_X, Y);
        render_dstate_xy(reststate, REST_X, Y);

        SSD1306_UpdateScreen();
    }
}
