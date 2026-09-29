"""Reproduce the OG Xbox Raiden II port from the preserved AB build and upstream snapshot.

Only files under this variant are written. Original and AB builds are read-only inputs.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT.parent / "MAMEoXtras-2026-AB-Autofire"
SRC = ROOT / "MAMEoXtras 2026 Src" / "MAME" / "src"
OLD = BASE / "MAMEoXtras 2026 Src" / "MAME" / "src"
UP = ROOT / "upstream" / "snapshot" / "src"

def read(path):
    return path.read_text(encoding="latin1")

def write(path, value):
    path.write_text(value, encoding="latin1", newline="\r\n")

def once(text, old, new):
    assert text.count(old) == 1, (old[:100], text.count(old))
    return text.replace(old, new, 1)

def api(text):
    for old, new in (("READ_HANDLER", "READ8_HANDLER"), ("WRITE_HANDLER", "WRITE8_HANDLER"),
                     ("MDRV_CPU_MEMORY", "MDRV_CPU_PROGRAM_MAP"),
                     ("cpu_set_irq_line_and_vector", "cpunum_set_input_line_and_vector"),
                     ("cpu_set_irq_line", "cpunum_set_input_line"),
                     ("cpu_readmem20", "program_read_byte"), ("cpu_writemem20", "program_write_byte")):
        text = re.sub(r"\b" + old + r"\b", new, text)
    return text

def maps(text):
    def convert(match):
        direction, name, body = match.groups()
        result = f"static ADDRESS_MAP_START( {name}, ADDRESS_SPACE_PROGRAM, 8 )\n"
        for line in body.splitlines():
            entry = re.match(r"\s*\{\s*(0x[0-9a-f]+),\s*(0x[0-9a-f]+),\s*(\w+)(.*?)\s*\},?(.*)", line)
            if not entry:
                result += line + "\n"
                continue
            lo, hi, handler, rest, comment = entry.groups()
            handler = handler.replace("MRA_", "MRA8_").replace("MWA_", "MWA8_")
            result += f"\tAM_RANGE({lo}, {hi}) AM_{direction}({handler})"
            arguments = [v.strip() for v in rest.split(",") if v.strip()]
            if arguments:
                result += f" AM_BASE({arguments[0]})"
            if len(arguments) > 1:
                result += f" AM_SIZE({arguments[1]})"
            result += comment + "\n"
        return result + "ADDRESS_MAP_END"
    return re.sub(r"static MEMORY_(READ|WRITE)_START\(\s*(\w+)\s*\)\n(.*?)MEMORY_END", convert, text, flags=re.S)

def main():
    original = read(OLD / "drivers/raiden2.c")
    upstream = read(UP / "drivers/raiden2.c")
    rom = re.search(r"ROM_START\( raiden2 \).*?ROM_END", upstream, re.S).group()
    # ROM loader interleaves the four encrypted 16-bit ROMs directly. No 8 MiB copy.
    rom = rom.replace("/* Keep old-set contiguous loading. DRIVER_INIT explicitly repacks and\n\t   decrypts this region before MAME decodes the graphics. */",
                      "/* Direct word interleave saves the upstream 8 MiB temporary repack. */")
    for name, before, after in (("obj1", "0x000000", "0x000000"), ("obj2", "0x200000", "0x000002"),
                               ("obj3", "0x400000", "0x400000"), ("obj4", "0x600000", "0x400002")):
        rom = once(rom, f'ROM_LOAD("{name}", {before}', f'ROM_LOAD32_WORD("{name}", {after}')
    original = re.sub(r"ROM_START\( raiden2 \).*?ROM_END", lambda _: rom, original, count=1, flags=re.S)
    parent = re.search(r'^GAMEX\(\s*1993,\s*raiden2,.*$', original, re.M).group()
    new_parent = 'GAMEX( 1993, raiden2, 0, r2play, r2play, r2play, ROT270, "Seibu Kaihatsu", "Raiden 2 (US, OG Xbox backport)", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND)'
    original = once(original, parent, '#include "raiden2_playable.inc"\n\n' + new_parent)
    original = once(original, "Raiden 2 Preliminary Driver", "Raiden 2 legacy family driver\nThe parent raiden2 set uses raiden2_playable.inc below (2026 OG Xbox port).\nThe other sets retain their original legacy definitions.")
    write(SRC / "drivers/raiden2.c", original)

    # Keep the backport in the SAME translation unit/section 598; __FILE__ in
    # GAME remains raiden2.c, preserving MAMEoX's dynamic driver section registry.
    playable = re.sub(r"ROM_START\( raiden2 \).*?ROM_END", "", upstream, count=1, flags=re.S)
    playable = playable.split("/* This backport is intended to be playable.", 1)[0]
    playable = maps(api(playable))
    # Namespaced video state permits the unchanged legacy siblings in the same TU.
    names = ["background_layer", "midground_layer", "foreground_layer", "text_layer",
             "back_data", "fore_data", "mid_data", "bg_bank", "mid_bank", "fg_bank", "tx_bank",
             "draw_sprites", "get_back_tile_info", "get_mid_tile_info", "get_fore_tile_info", "get_text_tile_info"]
    def namespace(text):
        for name in names:
            text = re.sub(r"\b" + name + r"\b", "r2play_" + name, text)
        text = re.sub(r"\braiden2(?=\b|_)", "r2play", text)
        # Include filenames and diagnostic exports remain their upstream names.
        text = re.sub(r'"r2play_(\w+\.(?:inc|h))"', r'"raiden2_\1"', text)
        return text
    playable = namespace(playable)
    playable = once(playable,
        '\tr2_bank_rom = auto_malloc(0x40000);\n\tif (r2_bank_rom) memcpy(r2_bank_rom, memory_region(REGION_CPU1), 0x40000);\n\telse r2_bank_rom = memory_region(REGION_CPU1) + 0x100000; /* untouched ROM_RELOAD mirror */',
        '\t/* The ROM_RELOAD mirror is immutable and already resident: save 256 KiB. */\n\tr2_bank_rom = memory_region(REGION_CPU1) + 0x100000;')
    write(SRC / "drivers/raiden2_playable.inc", playable)
    for name in ("raiden2_cop.inc", "raiden2_sound.inc", "raiden2_debug.inc", "raiden2_debug.h", "raiden2_diag_api.h"):
        write(SRC / "drivers" / name, namespace(api(read(UP / "drivers" / name))))

    crypt = read(UP / "drivers/raiden2_r2crypt.inc")
    start = crypt.index("static int raiden2_prepare_and_decrypt_sprites")
    loop = crypt.index("\t/* Decrypt logical little-endian dwords explicitly. */", start)
    crypt = crypt[:start] + "static int raiden2_prepare_and_decrypt_sprites(UINT8 *rom)\n{\n\tUINT32 i;\n\n\t/* ROM_LOAD32_WORD has already interleaved the input, without a copy. */\n" + crypt[loop:]
    write(SRC / "drivers/raiden2_r2crypt.inc", namespace(crypt))

    seibu = read(OLD / "sndhrdw/seibu.c")
    seibu = once(seibu, '#include "sndhrdw/seibu.h"', '#include "sndhrdw/seibu.h"\n#include "state.h"')
    seibu = once(seibu, "static int sound_cpu;", "static int sound_cpu;\nstatic int seibu_irq1, seibu_irq2;\nstatic int seibu_bank_latch;")
    seibu = once(seibu, "\tstatic int irq1,irq2;\n", "")
    seibu = re.sub(r"\birq1\b", "seibu_irq1", seibu)
    seibu = re.sub(r"\birq2\b", "seibu_irq2", seibu)
    seibu = once(seibu, "cpu_setbank(1,rom + 0x10000 + 0x8000 * (data & 1));", "seibu_bank_latch = data & 1;\n\tcpu_setbank(1,rom + 0x10000 + 0x8000 * seibu_bank_latch);")
    state = read(UP / "sndhrdw/seibu.c").split("static void seibu_sound_state_postload", 1)[1].split("WRITE_HANDLER( seibu_coin_w )", 1)[0]
    seibu = once(seibu, "WRITE8_HANDLER( seibu_coin_w )", "static void seibu_sound_state_postload" + api(state) + "WRITE8_HANDLER( seibu_coin_w )")
    write(SRC / "sndhrdw/seibu.c", seibu)
    header = read(OLD / "sndhrdw/seibu.h")
    header = once(header, "void seibu_sound_decrypt(int cpu_region,int length);", "void seibu_sound_decrypt(int cpu_region,int length);\nvoid seibu_sound_state_save_register(void);")
    write(SRC / "sndhrdw/seibu.h", header)

    gfx = read(OLD / "drawgfx.c")
    gfx = once(gfx, "\t\tif (0 && gl->planes <= 4 && !(gfx->width & 1))\n//\t\tif (gl->planes <= 4 && !(gfx->width & 1))",
        '\t\t/* Lossless 4-bit decode for the tested Raiden II path: 12.125 MiB saved. */\n\t\tif (!strcmp(Machine->gamedrv->name, "raiden2") && gl->planes <= 4 && !(gfx->width & 1))')
    write(SRC / "drawgfx.c", gfx)
    tilemap = read(OLD / "tilemap.c")
    begin = tilemap.index("static UINT8 TRANSP(HandleTransparencyPen)(")
    end = tilemap.index("static UINT8 TRANSP(HandleTransparencyPenBit)(", begin)
    handler = tilemap[begin:end]
    handler = once(handler, "\t\t\t\t((UINT8 *)transparency_bitmap->line[y])[x] = (pen==transparent_pen)?code_transparent:code_opaque;",
        "\t\t\t\t/* Both packed pixels contribute to the whole-tile classification. */\n\t\t\t\tif (pen == transparent_pen)\n\t\t\t\t{\n\t\t\t\t\t((UINT8 *)transparency_bitmap->line[y])[x] = code_transparent;\n\t\t\t\t\tbWhollyOpaque = 0;\n\t\t\t\t}\n\t\t\t\telse\n\t\t\t\t{\n\t\t\t\t\t((UINT8 *)transparency_bitmap->line[y])[x] = code_opaque;\n\t\t\t\t\tbWhollyTransparent = 0;\n\t\t\t\t}")
    write(SRC / "tilemap.c", tilemap[:begin] + handler + tilemap[end:])
    print("Ported the two commits, with OG Xbox API, section, ROM interleave and packed graphics adaptations.")

if __name__ == "__main__":
    main()
