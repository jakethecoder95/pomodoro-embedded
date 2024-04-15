#include <stdbool.h>
#include <string.h>

#include "display.h"

struct DisplayState focusstate;
struct DisplayState reststate;

int SCREEN_LENGTH = 128;
int SCREEN_WIDTH = 32;

int MILLI_HOUR = 3600000;
int MILLI_MIN = 60000;
int MILLI_SEC = 1000;

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

char get_display_number_from_selection(int selection) {

    char display[20];

    return display;
}

char get_display_number_from_elapsed_time() {

    char display[20];

    return display;
}

void render_header(struct PomodoroState pom) {
    char new_header[20];

    if (pom.mode == FOCUS_SELECT) {
        char focustxt[] = "Selecting Focus";
        memcpy(new_header, focustxt, sizeof(focustxt));
    } else if (pom.mode == REST_SELECT) {
        char resttxt[] = "Selecting Rest";
        memcpy(new_header, resttxt, sizeof(resttxt));
    } else if (pom.mode == FOCUSING) {
        char focusingtxt[] = "Focusing";
        memcpy(new_header, focusingtxt, sizeof(focusingtxt));
    } else if (pom.mode == FOCUSING) {
        char restingtxt[] = "Resting";
        memcpy(new_header, restingtxt, sizeof(restingtxt));
    }

    SSD1306_GotoXY(0, 0);
    SSD1306_Puts(new_header, &SSD1306_Font_7x10, 1);
}

/**
 * @brief  Updates the provided state to match the supplied timemilli.
 * @retval A boolean representing if the state was updated or not.
 */
bool update_display(struct DisplayState *state, uint32_t timemilli) {

    if (state->totalmilli == timemilli) {
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

void render_focus() {
    char str[5] = {0};
    int index = 0;

    if (focusstate.hours > 0) {
        char hours[2];
        int_to_str(focusstate.hours, hours);
        hours[1] = ':';
        str[index++] = hours[0];
        str[index++] = hours[1];
    } else {
        strcat(str, "0:");
        index += 2;
    }

    if (focusstate.minutes > 0) {
        char mins[2];
        int_to_str(focusstate.minutes, mins);
        if (focusstate.minutes < 10) {
            mins[1] = mins[0];
            mins[0] = '0';
        }
        str[index++] = mins[0];
        str[index++] = mins[1];
    } else {
        strcat(str, "00");
        index += 2;
    }

    // Without this, a random extra character is added... I'm not sure why.
    str[4] = ' ';

    SSD1306_GotoXY(0, 13);
    SSD1306_Puts(str, &SSD1306_Font_11x18, 1);
}

void render_rest() {

}

void render_time(uint32_t timemilli) {
    char str[20];
    int_to_str(timemilli, str);

    SSD1306_GotoXY(0, 13);
    SSD1306_Puts(str, &SSD1306_Font_11x18, 1);
}

void Display_Init(struct PomodoroState pom) {
    update_display(&focusstate, pom.focus * MILLI_MIN);
    update_display(&reststate, pom.rest * MILLI_MIN);
}

void Display_Sync(struct PomodoroState pom) {

    SSD1306_Fill(SSD1306_COLOR_BLACK);
    render_header(pom);

    uint32_t focusmilli = pom.focus * MILLI_MIN;
    uint32_t restmilli = pom.rest * MILLI_MIN;

    if (pom.mode == FOCUS_SELECT) {
        update_display(&focusstate, focusmilli);
    } else if (pom.mode == REST_SELECT) {
        update_display(&reststate, restmilli);
    } else if (pom.mode == FOCUSING) {
        update_display(&focusstate, focusmilli - pom.elapsed_time);
    } else {
        update_display(&reststate, restmilli - pom.elapsed_time);
    }

    render_focus();

    SSD1306_UpdateScreen();
}
