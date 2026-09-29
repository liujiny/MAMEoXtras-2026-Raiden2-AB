#ifndef AUTOFIRE_STATE_H
#define AUTOFIRE_STATE_H

/* Three frames down, three frames up: ten pulses/sec at 60 emulated FPS. */
#define AUTOFIRE_DOWN_FRAMES 3
#define AUTOFIRE_PERIOD_FRAMES 6

typedef struct AutoFireState
{
    unsigned int phase;
    int pressed;
} AutoFireState;

/* Called for every A/B button once per game input frame, even if unmapped. */
static void AutoFireStep(AutoFireState *state, int held, int enabled)
{
    if (!held || !enabled)
    {
        state->phase = 0;
        state->pressed = held != 0;
        return;
    }

    state->pressed = state->phase < AUTOFIRE_DOWN_FRAMES;
    state->phase = (state->phase + 1) % AUTOFIRE_PERIOD_FRAMES;
}

/* Reading input never advances the pulse. A new press always starts down. */
static int AutoFireRead(const AutoFireState *state, int held,
                        int enabled, int gameInput)
{
    if (!gameInput || !enabled)
        return held != 0;
    return held && state->pressed;
}

#endif
