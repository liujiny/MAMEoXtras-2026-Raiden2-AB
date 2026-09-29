"""Run the actual autofire code on Windows with simulated controller/INI adapters.

The Xbox build uses VC 7.1; these host tests use the installed Windows VC 10.
No Xbox runtime, controller hardware or rendered menus are emulated here.
"""
from pathlib import Path
import os
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "MAMEoXtras 2026 Src"
OUT = ROOT / "build/tests"


def between(text, start, end):
    if text.count(start) != 1:
        raise ValueError(f"Expected unique start: {start}")
    return text.split(start, 1)[1].split(end, 1)[0]


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    joystick = (SRC / "MAMEoX/Sources/xbox_JoystickMouse.c").read_text(encoding="latin1")
    joyheader = (SRC / "MAMEoX/Includes/xbox_JoystickMouse.h").read_text(encoding="latin1")
    util = (SRC / "MAMEoX/Sources/MAMEoXUtil.cpp").read_text(encoding="latin1")
    menu = (SRC / "MAMEoXLauncher/Sources/OptionsScreen.cpp").read_text(encoding="latin1")
    ports = (SRC / "MAME/src/inptport.c").read_text(encoding="latin1")
    sdk = (ROOT.parent / "toolchain/XDK/xbox/include/Xbox.h").read_text(encoding="latin1")
    constants = "\n".join(line for line in sdk.splitlines()
                          if re.match(r"#define XINPUT_GAMEPAD_\w+\s+(?:0x[0-9A-Fa-f]+|[0-9]+)\s*$", line))
    joymacros = "\n".join(line for line in joyheader.splitlines()
                         if re.match(r"#define (?:JOYCODE|JOYINDEX|JT|JOYNUM)\(", line))
    enums = "typedef enum XBOXJoyType" + between(joyheader, "typedef enum XBOXJoyType", "//= P R O T O T Y P E S")
    frame = "static AutoFireState" + between(joystick, "static AutoFireState", "//= F U N C T I O N S")
    query = "int osd_is_joy_pressed" + between(joystick, "int osd_is_joy_pressed", "//---------------------------------------------------------------------")
    load = between(util, "  // Missing keys (including older INI files) mean auto fire is disabled.\n", "    //-- Lightgun calibration")
    save_section = util.split("void SaveOptions( void )", 1)[1]
    save = "  for (UINT32 player" + between(save_section, "  for (UINT32 player", '  iniFile.WriteProfileInt( "Input", "Lightgun1_Left"')
    change = "void COptionsScreen::ChangeAutoFirePage" + between(menu, "void COptionsScreen::ChangeAutoFirePage", "void DrawAutoFirePage")
    scope = "InputSeq* seq;" + between(ports, "InputSeq* seq;\n                    int pressed;", "\t\t\t\t\tif (pressed)")
    scope = scope.replace("InputSeq* seq;", "InputSeq* seq;\nint pressed;", 1)
    (OUT / "production_fragments.h").write_text(
        constants + "\n" + joymacros + "\n" + enums + "\n" + frame + "\n" + query
        + "\nvoid LoadAutoFireFixture(FakeIni &iniFile) {\n" + load + "}\n"
        + "void SaveOptions(void) {\nFakeIni &iniFile = savedIni; ++saveCalls;\n" + save + "}\n"
        + change + "\nint ReadGamePort(InputPort *in) {\n" + scope + "return pressed;\n}\n",
        encoding="ascii")
    vcvars = Path(os.environ.get("AUTOFIRE_HOST_VCVARS", r"D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"))
    if not vcvars.is_file():
        raise RuntimeError("Host tests need Windows VC 2010. Set AUTOFIRE_HOST_VCVARS to vcvarsall.bat.")
    command = OUT / "run_native.cmd"
    command.write_text(
        '@echo off\nsetlocal\n'
        f'call "{vcvars}" x86\nif errorlevel 1 exit /b %ERRORLEVEL%\n'
        f'cd /d "{OUT}"\n'
        'set CL=\nset _CL_=\n'
        f'cl /nologo /W4 /wd4127 /D_CRT_SECURE_NO_WARNINGS /EHsc /MT /Od /D_XBOX /I"{SRC / "MAMEoX/Includes"}" /I"{OUT}" '
        f'"{ROOT / "tests/test_autofire.cpp"}" /Feautofire-tests.exe\n'
        'if errorlevel 1 exit /b %ERRORLEVEL%\nautofire-tests.exe\nexit /b %ERRORLEVEL%\n',
        encoding="mbcs")
    result = subprocess.run(["cmd.exe", "/d", "/c", str(command)], cwd=ROOT,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    (OUT / "host-tests.log").write_bytes(result.stdout)
    print(result.stdout.decode("mbcs", errors="replace"))
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
