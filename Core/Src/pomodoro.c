#include "stdbool.h"

#include "pomodoro.h"

#include "util.h"

extern struct PomodoroState pomodoro;

int POMODORO_INCREMENTOR = 5;

void Pomodoro_Init(int focus, int rest, uint32_t curmilli) {
    pomodoro.focus = focus;
    pomodoro.rest = rest;
    pomodoro.mode = FOCUS_SELECT;
    pomodoro.paused = true;
    pomodoro.elapsed_time = 0;
    pomodoro.prev_time = curmilli;

}

void Pomodoro_Reset(uint32_t curmilli) {
    Pomodoro_Init(pomodoro.focus, pomodoro.rest, curmilli);
}

void Pomodoro_IncTime() {
    if (pomodoro.mode == FOCUSING || pomodoro.mode == RESTING) {
        return;
    }

    if (pomodoro.mode == FOCUS_SELECT) {
        pomodoro.focus += POMODORO_INCREMENTOR;
    } else {
        pomodoro.rest += POMODORO_INCREMENTOR;
    }
}

void Pomodoro_DecTime() {
    if (pomodoro.mode == FOCUSING || pomodoro.mode == RESTING) {
        return;
    }

    if (pomodoro.mode == FOCUS_SELECT) {
        pomodoro.focus -= POMODORO_INCREMENTOR;
    } else {
        pomodoro.rest -= POMODORO_INCREMENTOR;
    }
}

void Pomodoro_Select(uint32_t curmilli) {
    if (pomodoro.mode == FOCUS_SELECT) {
        pomodoro.mode = REST_SELECT;
    } else if (pomodoro.mode == REST_SELECT) {
        pomodoro.mode = FOCUSING;
        pomodoro.paused = false;
        pomodoro.prev_time = curmilli;
        pomodoro.elapsed_time = 0;
    } else {
        pomodoro.paused = !pomodoro.paused;
        pomodoro.prev_time = curmilli;
    }
}

void Pomodoro_Sync(uint32_t curmilli) {

    if (!pomodoro.paused
            && (pomodoro.mode == FOCUSING || pomodoro.mode == RESTING)) {

        uint32_t target_time;
        if (pomodoro.mode == FOCUSING) {
            target_time = Util_ConvertMinToMilli(pomodoro.focus);
        } else {
            target_time = Util_ConvertMinToMilli(pomodoro.rest);
        }

        pomodoro.elapsed_time += curmilli - pomodoro.prev_time;
        pomodoro.prev_time = curmilli;

        if (pomodoro.elapsed_time >= target_time) {
            if (pomodoro.mode == FOCUSING) {
                pomodoro.mode = RESTING;
            } else {
                pomodoro.mode = FOCUSING;
            }
            pomodoro.elapsed_time = 0;
        }
    }
}
