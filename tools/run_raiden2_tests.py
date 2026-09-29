"""Compile production fragments with Windows VC10 and compare to upstream.

No Xbox/PS3 emulator, ROM, or hardware timing claims. Generated harnesses and
compiler/test logs remain in build/raiden2-tests for inspection.
"""
from pathlib import Path
import hashlib
import importlib.util
import json
import os
import re
import subprocess
import sys
from gfx_regression import generate as graphics_harness

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "MAMEoXtras 2026 Src/MAME/src"
UP = ROOT / "upstream/snapshot"
OUT = ROOT / "build/raiden2-tests"

def read(path):
    return path.read_text(encoding="latin1")

def module(name):
    spec = importlib.util.spec_from_file_location(name, UP / "tools" / (name + ".py"))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result

def fragment(text, start, end):
    a = text.index(start)
    return text[a:text.index(end, a)]

def tilemap_harness(source):
    mod = module("test_packed_tilemap")
    handler = fragment(source, "static UINT8 TRANSP(HandleTransparencyPen)(", "static UINT8 TRANSP(HandleTransparencyPenBit)(")
    variants = []
    for name, init, get in (("ind", "const pen_t *pPalData = tile_info.pal_data", "pPalData[pen]"),
                            ("raw", "int palBase = tile_info.pal_data - Machine->remapped_colortable", "(palBase + (pen))")):
        variants.append(f"#define TRANSP(f) f ## _{name}\n#define PAL_INIT {init}\n#define PAL_GET(pen) {get}\n"
                        + handler + "\n#undef TRANSP\n#undef PAL_INIT\n#undef PAL_GET\n")
    return mod.PREAMBLE + "\n".join(variants) + mod.TESTS

SPRITE_COMPARE = r'''
static struct mame_bitmap expected_bitmap;
static struct draw_call expected_draws[4096];
static unsigned random_state = 0x91349721;
static unsigned rnd(void) { random_state ^= random_state << 13; random_state ^= random_state >> 17; random_state ^= random_state << 5; return random_state; }
static unsigned compare_cases;
static void compare(const struct rectangle *clip)
{
    unsigned priority, expected_count;
    latch();
    for (priority = 0; priority < 4; ++priority) {
        memset(&bitmap, 0x63, sizeof(bitmap));
        draw_count = 0;
        ref_draw_sprites(&bitmap, clip, priority);
        expected_count = draw_count;
        memcpy(&expected_bitmap, &bitmap, sizeof(bitmap));
        memcpy(expected_draws, draws, draw_count * sizeof(draws[0]));
        memset(&bitmap, 0x63, sizeof(bitmap));
        draw_count = 0;
        draw_sprites(&bitmap, clip, priority);
        CHECK(draw_count == expected_count);
        CHECK(!memcmp(draws, expected_draws, draw_count * sizeof(draws[0])));
        CHECK(!memcmp(&bitmap, &expected_bitmap, sizeof(bitmap)));
        ++compare_cases;
    }
}
static void compare_optimized_sprites(void)
{
    unsigned frame, entry, flip, size, x, y;
    static const unsigned positions[] = {0,1,15,16,239,240,255,256,319,320,383,384,495,496,497,511,512,65535};
    /* Boundary positions exercise partially visible and wrapped multi-tile
       sprites, all four flip combinations and all 1..8 tile extents. */
    for (flip = 0; flip < 4; ++flip)
        for (size = 0; size < 8; ++size)
            for (x = 0; x < sizeof(positions)/sizeof(positions[0]); ++x)
                for (y = 0; y < sizeof(positions)/sizeof(positions[0]); ++y) {
                    reset();
                    sprite(0, (size << 8) | (size << 12) | ((flip & 1) << 11) | ((flip & 2) << 14),
                           0xffe0, positions[x], positions[y]);
                    compare(&visible);
                }
    for (frame = 0; frame < 512; ++frame) {
        struct rectangle clip = visible;
        reset();
        for (entry = 0; entry < 32; ++entry) {
            unsigned offset = (rnd() & 511) * 8;
            sprite(offset, rnd() & 65535, (entry % 9 == 0) ? 0 : rnd() & 65535, rnd() & 65535, rnd() & 65535);
        }
        if (frame & 1) { clip.min_x = rnd() % 160; clip.max_x = 160 + rnd() % 160; clip.min_y = rnd() % 120; clip.max_y = 120 + rnd() % 120; }
        if (frame % 17 == 0) layers = 0x10;
        compare(&clip);
    }
    printf("PASS: %u optimized/reference sprite passes; draw calls, ordering, flips, clipping and raster bytes identical\n", compare_cases);
}
'''

CRYPT_TEST = r'''
#include <time.h>
static unsigned rng = 0x10437289;
static unsigned next_value(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
int main(void)
{
    unsigned i, trial;
    UINT8 *reference = (UINT8 *)malloc(0x800000);
    UINT8 *optimized = (UINT8 *)malloc(0x800000);
    CHECK(reference && optimized);
    for (i = 0; i < 1000000; ++i) {
        UINT32 a = next_value(), b = next_value(), mask = next_value();
        CHECK(r2_partial_carry_fast(a, b, mask) == ref_r2_partial_carry_sum32(a, b, mask));
    }
    puts("PASS: 1000000 random masked-carry inputs match bit-serial reference");
    for (trial = 0; trial < 2; ++trial) {
        clock_t t0, t1, t2;
        UINT32 checksum = 2166136261U;
        for (i = 0; i < 0x800000; ++i) reference[i] = trial ? (UINT8)next_value() : (UINT8)(i ^ (i >> 7) ^ (i >> 16));
        /* ROM_LOAD32_WORD: two-byte groups, skip two bytes, starting at the
           exact four offsets in the production raiden2 ROM definition. */
        for (i = 0; i < 0x100000; ++i) {
            memcpy(optimized + i*4, reference + i*2, 2);
            memcpy(optimized + i*4+2, reference + 0x200000+i*2, 2);
            memcpy(optimized + 0x400000+i*4, reference + 0x400000+i*2, 2);
            memcpy(optimized + 0x400000+i*4+2, reference + 0x600000+i*2, 2);
        }
        t0 = clock();
        CHECK(ref_raiden2_prepare_and_decrypt_sprites(reference) == 0);
        t1 = clock();
        CHECK(r2play_prepare_and_decrypt_sprites(optimized) == 0);
        t2 = clock();
        CHECK(!memcmp(reference, optimized, 0x800000));
        for (i = 0; i < 0x800000; ++i) checksum = (checksum ^ optimized[i]) * 16777619U;
        printf("PASS: 8 MiB sprite decrypt pattern %u identical (FNV1a %08x); host reference %.3fs, optimized %.3fs\n",
               trial, checksum, (double)(t1-t0)/CLOCKS_PER_SEC, (double)(t2-t1)/CLOCKS_PER_SEC);
    }
    free(reference); free(optimized);
    return 0;
}
'''

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    driver = read(SRC / "drivers/raiden2_playable.inc")
    core = read(SRC / "drivers/raiden2.c")
    map_text = fragment(driver, "static ADDRESS_MAP_START( r2play_writemem", "ADDRESS_MAP_END")
    entries = re.findall(r"AM_RANGE\((0x[0-9a-f]+), (0x[0-9a-f]+)\) AM_WRITE\((\w+)\)", map_text)
    for address in (0x68e, 0x68f):
        selected = next(handler for lo, hi, handler in entries if int(lo,16) <= address <= int(hi,16))
        assert selected == "buffer_spriteram_w", (address, selected)
    assert "VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM" in driver
    assert "\tr2_prepare_sprite_lists();" in driver
    assert "MDRV_FRAMES_PER_SECOND(55.47)" in driver
    assert "MDRV_CPU_ADD(V30, 32000000/2)" in driver
    for name, offset in (("obj1", "0x000000"), ("obj2", "0x000002"), ("obj3", "0x400000"), ("obj4", "0x400002")):
        assert f'ROM_LOAD32_WORD("{name}", {offset}' in core
    assert "ROM_RELOAD(                 0x100000, 0x80000)" in core
    assert "ROM_RELOAD(                 0x100001, 0x80000)" in core
    print("PASS: actual latch map, video buffering, original clock/timing, bank mirror and ROM interleave wiring", flush=True)

    sources = {}
    sources["graphics"] = graphics_harness(SRC)
    sources["tilemap"] = tilemap_harness(read(SRC / "tilemap.c"))
    base = ROOT.parent / "MAMEoXtras-2026-AB-Autofire/MAMEoXtras 2026 Src/MAME/src/tilemap.c"
    sources["tilemap-negative"] = tilemap_harness(read(base))
    mod = module("test_raiden2_sprites")
    sprite_code = fragment(driver, "static UINT16 r2_spr_word(", "WRITE8_HANDLER( r2play_background_w )")
    reference = fragment(read(UP / "src/drivers/raiden2.c"), "static UINT16 r2_spr_word(", "WRITE_HANDLER( raiden2_background_w )")
    reference = reference.replace("r2_spr_word", "ref_spr_word").replace("r2_draw_one_sprite", "ref_draw_one_sprite").replace("draw_sprites", "ref_draw_sprites")
    wrapper = "\nstatic void draw_sprites(struct mame_bitmap *b, const struct rectangle *c, int p) { r2_prepare_sprite_lists(); r2play_draw_sprites(b,c,p); }\n"
    test = mod.TESTS.replace("    return 0;", "    compare_optimized_sprites();\n    return 0;")
    sources["sprites"] = mod.PREAMBLE + sprite_code + wrapper + reference + SPRITE_COMPARE + test
    reference_crypt = read(UP / "src/drivers/raiden2_r2crypt.inc")
    reference_crypt = re.sub(r"\b(r2_\w+|raiden2_prepare_and_decrypt_sprites)\b", r"ref_\1", reference_crypt)
    sources["decrypt"] = "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\ntypedef unsigned char UINT8;\ntypedef unsigned short UINT16;\ntypedef unsigned int UINT32;\n#define CHECK(c) do { if (!(c)) { fprintf(stderr,\"FAIL line %d: %s\\n\",__LINE__,#c); exit(1); } } while(0)\n" + reference_crypt + read(SRC / "drivers/raiden2_r2crypt.inc") + CRYPT_TEST

    vcvars = Path(os.environ.get("RAIDEN2_HOST_VCVARS", r"D:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"))
    if not vcvars.is_file():
        raise RuntimeError("Set RAIDEN2_HOST_VCVARS to the Windows VC10 vcvarsall.bat.")
    results = {}
    for name, source in sources.items():
        harness = OUT / (name + (".c" if name == "graphics" else ".cpp"))
        harness.write_text(source, encoding="latin1")
        batch = OUT / (name + ".cmd")
        batch.write_text('@echo off\nsetlocal\n' + f'call "{vcvars}" x86\nif errorlevel 1 exit /b %ERRORLEVEL%\ncd /d "{OUT}"\nset CL=\nset _CL_=\n' +
            f'cl /nologo /W3 /EHsc /MT /O2 /D_CRT_SECURE_NO_WARNINGS "{harness}" /Fe"{name}.exe" /Fo"{name}.obj"\nif errorlevel 1 exit /b %ERRORLEVEL%\n"{name}.exe"\nexit /b %ERRORLEVEL%\n', encoding="mbcs")
        result = subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=OUT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (OUT / (name + ".log")).write_bytes(result.stdout)
        output = result.stdout.decode("mbcs", errors="replace")
        print(output, flush=True)
        if name == "tilemap-negative":
            assert result.returncode == 1 and "classification mismatch" in output, "Negative control must fail for the known high-nibble bug"
            print("PASS: original tilemap negative control detects the missing high-nibble classification", flush=True)
        elif result.returncode:
            raise RuntimeError(f"{name} failed ({result.returncode}); see {OUT / (name + '.log')}")
        results[name] = {"exit_code": result.returncode, "expected_failure": name.endswith("negative"),
                         "source_sha256": hashlib.sha256(harness.read_bytes()).hexdigest(), "log": str(OUT / (name + ".log"))}
    (OUT / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    print("All Raiden II source-level regression gates passed.", flush=True)

if __name__ == "__main__":
    main()
