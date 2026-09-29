#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include "xbox_AutoFire.h"
#include "AutoFireState.h"

typedef unsigned int UINT32;
typedef int INT;
typedef int BOOL;
#define FALSE 0
#define ANALOG_BUTTON_AS_DIGITAL_DEADZONE 16
#define ANALOG_AS_DIGITAL_DEADZONE (32768 * 0.33f)

struct XINPUT_GAMEPAD
{
    unsigned short wButtons;
    unsigned char bAnalogButtons[8];
    short sThumbLX, sThumbLY, sThumbRX, sThumbRY;
};
static XINPUT_GAMEPAD pads[4];
static int connected[4];
const XINPUT_GAMEPAD *GetGamepadState(int player)
{
    return player >= 0 && player < 4 && connected[player] ? &pads[player] : NULL;
}
int g_autoFireEnabled[AUTOFIRE_PLAYERS][AUTOFIRE_BUTTONS] = {0};

struct FakeIni
{
    std::map<std::string, int> values;
    int GetProfileInt(const char *section, const char *key, int fallback)
    {
        std::string full = std::string(section) + "/" + key;
        return values.count(full) ? values[full] : fallback;
    }
    void WriteProfileInt(const char *section, const char *key, int value)
    {
        values[std::string(section) + "/" + key] = value;
    }
};
static FakeIni savedIni;
static int saveCalls;
void SaveOptions(void);
class COptionsScreen
{
public:
    unsigned int m_cursorPosition;
    void ChangeAutoFirePage(BOOL movingRight);
};

/* The adapter provides a single joystick sequence to the real port-scoping code. */
typedef int InputSeq;
struct InputPort { int type; InputSeq code; };
enum { IPT_BUTTON1 = 100, IPT_BUTTON10 = 109, IPT_COIN1, IPT_START1, IPT_JOYSTICK_UP };
InputSeq *input_port_seq(InputPort *in, int) { return &in->code; }
int osd_is_joy_pressed(int);
int seq_pressed(InputSeq *seq) { return osd_is_joy_pressed(*seq); }

/* Generated on every run from the delivered, compiled Xbox source files. */
#include "production_fragments.h"

static int checks;
#define CHECK(expression) do { ++checks; if (!(expression)) { \
    std::printf("FAIL line %d: %s\n", __LINE__, #expression); return 1; } } while (0)

static void Reset(void)
{
    std::memset(pads, 0, sizeof(pads));
    std::memset(g_autoFireEnabled, 0, sizeof(g_autoFireEnabled));
    for (int p = 0; p < 4; ++p) connected[p] = 1;
    XboxAutoFireBeginFrame();
}
static int GameButton(int player, int button, int type = IPT_BUTTON1)
{
    InputPort port = {type, JOYCODE(player, JT_BUTTON, button)};
    return ReadGamePort(&port);
}

int main(void)
{
    int p, f, b;
    Reset();
    FakeIni oldIni;
    LoadAutoFireFixture(oldIni);
    for (p = 0; p < 4; ++p)
        for (b = 0; b < 2; ++b) CHECK(g_autoFireEnabled[p][b] == 0);

    /* Disabled: ordinary held inputs remain held for every controller. */
    for (p = 0; p < 4; ++p)
        pads[p].bAnalogButtons[XINPUT_GAMEPAD_A] = pads[p].bAnalogButtons[XINPUT_GAMEPAD_B] = 255;
    for (f = 0; f < 30; ++f)
    {
        XboxAutoFireBeginFrame();
        for (p = 0; p < 4; ++p)
        {
            CHECK(GameButton(p, BUTTON_A) == 1);
            CHECK(GameButton(p, BUTTON_B) == 1);
        }
    }

    /* Enabled: exact frame cadence, no extra advancement when queried twice. */
    for (p = 0; p < 4; ++p)
        for (b = 0; b < 2; ++b) g_autoFireEnabled[p][b] = 1;
    for (f = 0; f < 120; ++f)
    {
        XboxAutoFireBeginFrame();
        int expected = (f % 6) < 3;
        for (p = 0; p < 4; ++p)
        {
            CHECK(GameButton(p, BUTTON_A) == expected);
            CHECK(GameButton(p, BUTTON_A) == expected);
            CHECK(GameButton(p, BUTTON_B, IPT_BUTTON10) == expected);
            CHECK(osd_is_joy_pressed(JOYCODE(p, JT_BUTTON, BUTTON_A)) == 1);
            CHECK(osd_is_joy_pressed(JOYCODE(p, JT_BUTTON, BUTTON_B)) == 1);
            CHECK(GameButton(p, BUTTON_A, IPT_COIN1) == 1);
            CHECK(GameButton(p, BUTTON_B, IPT_START1) == 1);
            CHECK(GameButton(p, BUTTON_A, IPT_JOYSTICK_UP) == 1);
            CHECK(g_autoFireGameInput == 0);
        }
    }

    /* Xbox X, Y and other controls keep their original behavior during an off pulse. */
    pads[0].bAnalogButtons[XINPUT_GAMEPAD_X] = 255;
    pads[0].bAnalogButtons[XINPUT_GAMEPAD_Y] = 255;
    CHECK(GameButton(0, BUTTON_A) == 0);
    CHECK(GameButton(0, BUTTON_X) == 1);
    CHECK(GameButton(0, BUTTON_Y) == 1);

    /* Release during off phase, then repress: first frame down, independent of B/P2. */
    pads[0].bAnalogButtons[XINPUT_GAMEPAD_A] = 0;
    XboxAutoFireBeginFrame();
    CHECK(GameButton(0, BUTTON_A) == 0);
    pads[0].bAnalogButtons[XINPUT_GAMEPAD_A] = 255;
    for (f = 0; f < 6; ++f)
    {
        XboxAutoFireBeginFrame();
        CHECK(GameButton(0, BUTTON_A) == (f < 3));
        CHECK(GameButton(0, BUTTON_B) == (((f + 1) % 6) < 3));
        CHECK(GameButton(1, BUTTON_A) == (((f + 1) % 6) < 3));
    }

    /* Disabling while held restores a normal hold, enabling starts a fresh pulse. */
    g_autoFireEnabled[0][AUTOFIRE_A] = 0;
    CHECK(GameButton(0, BUTTON_A) == 1);
    XboxAutoFireBeginFrame();
    g_autoFireEnabled[0][AUTOFIRE_A] = 1;
    XboxAutoFireBeginFrame();
    CHECK(GameButton(0, BUTTON_A) == 1);

    /* Independent switches: only P1 A and P4 B auto fire. */
    Reset();
    for (p = 0; p < 4; ++p)
        pads[p].bAnalogButtons[XINPUT_GAMEPAD_A] = pads[p].bAnalogButtons[XINPUT_GAMEPAD_B] = 255;
    g_autoFireEnabled[0][AUTOFIRE_A] = g_autoFireEnabled[3][AUTOFIRE_B] = 1;
    for (f = 0; f < 4; ++f) XboxAutoFireBeginFrame();
    CHECK(GameButton(0, BUTTON_A) == 0);
    CHECK(GameButton(0, BUTTON_B) == 1);
    CHECK(GameButton(3, BUTTON_A) == 1);
    CHECK(GameButton(3, BUTTON_B) == 0);
    CHECK(GameButton(1, BUTTON_A) == 1);
    CHECK(GameButton(2, BUTTON_B) == 1);

    /* Deadzone, disconnect/reconnect and invalid controller numbers. */
    pads[0].bAnalogButtons[XINPUT_GAMEPAD_A] = 16;
    XboxAutoFireBeginFrame();
    CHECK(GameButton(0, BUTTON_A) == 0);
    pads[0].bAnalogButtons[XINPUT_GAMEPAD_A] = 17;
    XboxAutoFireBeginFrame();
    CHECK(GameButton(0, BUTTON_A) == 1);
    connected[0] = 0;
    XboxAutoFireBeginFrame();
    CHECK(GameButton(0, BUTTON_A) == 0);
    connected[0] = 1;
    XboxAutoFireBeginFrame();
    CHECK(GameButton(0, BUTTON_A) == 1);
    CHECK(GameButton(4, BUTTON_A) == 0);
    CHECK(GameButton(15, BUTTON_B) == 0);

    /* A stale filtered result cannot keep a physically released button down. */
    pads[0].bAnalogButtons[XINPUT_GAMEPAD_A] = 0;
    CHECK(GameButton(0, BUTTON_A) == 0);

    /* Actual option callback and INI loops: eight rows round-trip independently. */
    Reset();
    savedIni.values.clear();
    savedIni.values["General/CheatsEnabled"] = 1;
    saveCalls = 0;
    COptionsScreen screen;
    for (unsigned int row = 0; row < 8; ++row)
    {
        screen.m_cursorPosition = row;
        screen.ChangeAutoFirePage(1);
        CHECK(g_autoFireEnabled[row / 2][row % 2] == 1);
        screen.ChangeAutoFirePage(1);
        CHECK(saveCalls == (int)row + 1);
    }
    CHECK(savedIni.values.size() == 9);
    CHECK(savedIni.values["Input/AutoFireP1A"] == 1);
    CHECK(savedIni.values["Input/AutoFireP4B"] == 1);
    CHECK(savedIni.values["General/CheatsEnabled"] == 1);
    for (unsigned int offRow = 0; offRow < 8; offRow += 2)
    {
        screen.m_cursorPosition = offRow;
        screen.ChangeAutoFirePage(0);
    }
    std::memset(g_autoFireEnabled, 0, sizeof(g_autoFireEnabled));
    LoadAutoFireFixture(savedIni);
    for (p = 0; p < 4; ++p)
    {
        CHECK(g_autoFireEnabled[p][AUTOFIRE_A] == 0);
        CHECK(g_autoFireEnabled[p][AUTOFIRE_B] == 1);
    }
    screen.m_cursorPosition = 8;
    screen.ChangeAutoFirePage(1);
    CHECK(saveCalls == 12);

    std::printf("PASS: %d assertions; cadence, gameplay scope, 4 ports, release, switches, INI round-trip.\n", checks);
    return 0;
}
