#ifndef XBOX_AUTOFIRE_H
#define XBOX_AUTOFIRE_H

#define AUTOFIRE_PLAYERS 4
#define AUTOFIRE_BUTTONS 2
#define AUTOFIRE_A 0
#define AUTOFIRE_B 1

#ifdef __cplusplus
extern "C" {
#endif

/* Physical controller ports, independent of the game's button mapping. */
extern int g_autoFireEnabled[AUTOFIRE_PLAYERS][AUTOFIRE_BUTTONS];

/* Advance once per emulated input frame, including skipped video frames. */
void XboxAutoFireBeginFrame(void);

/* Only game button sequences may see auto fire. UI queries always see raw input. */
void XboxAutoFireSetGameInput(int enabled);

#ifdef __cplusplus
}
#endif

#endif
