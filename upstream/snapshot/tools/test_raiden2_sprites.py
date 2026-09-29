#!/usr/bin/env python3
"""Compile the driver's actual sprite walker against a tiny raster test bed.

No ROM or PS3 SDK is needed. Pass --source - to check an older driver from
git show and verify that the corner-artifact regression fails before the fix.
"""

import argparse
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile


PREAMBLE = r"""
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
#define RAIDEN2_DEBUG 0
#define TRANSPARENCY_PEN 1
#define WIDTH 320
#define HEIGHT 240
#define BACKGROUND 0x3456
struct rectangle { int min_x, max_x, min_y, max_y; };
struct mame_bitmap { UINT16 pixels[HEIGHT][WIDTH]; };
struct GfxElement { int unused; };
struct MachineTest { struct GfxElement *gfx[3]; };
static struct GfxElement sprite_gfx;
static struct MachineTest machine = {{ NULL, NULL, &sprite_gfx }};
static struct MachineTest *Machine = &machine;
static UINT8 ram[0x1000], snapshot[0x1000];
static UINT8 *spriteram = ram, *buffered_spriteram = snapshot;
static size_t spriteram_size = sizeof(ram);
static UINT16 layers;
static int r2_video_enable(void) { return layers; }
static struct mame_bitmap bitmap;
static const struct rectangle visible = { 0, WIDTH - 1, 0, HEIGHT - 1 };
struct draw_call { int tile, color, flipx, flipy, x, y; };
static struct draw_call draws[4096];
static unsigned draw_count;
static void draw_sprites(struct mame_bitmap *, const struct rectangle *, int);

/* Same operation as generic video's buffer_spriteram_w handler. */
static void latch(void)
{
    memcpy(buffered_spriteram, spriteram, spriteram_size);
}

static void render(int priority)
{
    latch();
    draw_sprites(&bitmap, &visible, priority);
}

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); \
    exit(1); } } while (0)

/* Use a nontransparent black tile 0. An empty sprite descriptor is not the
 * same thing as a decoded tile whose pixels all happen to be pen 15. */
static void drawgfx(struct mame_bitmap *dest, const struct GfxElement *gfx,
        unsigned code, unsigned color, int flipx, int flipy, int sx, int sy,
        const struct rectangle *cliprect, int transparency, int pen)
{
    int x, y;
    struct draw_call *call;
    CHECK(gfx == &sprite_gfx);
    CHECK(transparency == TRANSPARENCY_PEN && pen == 15);
    CHECK(draw_count < sizeof(draws) / sizeof(draws[0]));
    call = &draws[draw_count++];
    call->tile = code; call->color = color;
    call->flipx = flipx; call->flipy = flipy;
    call->x = sx; call->y = sy;
    for (y = sy; y < sy + 16; ++y)
        for (x = sx; x < sx + 16; ++x)
            if (x >= cliprect->min_x && x <= cliprect->max_x &&
                y >= cliprect->min_y && y <= cliprect->max_y)
                dest->pixels[y][x] = code ? (UINT16)(code | 1) : 0;
}

static void write_word(unsigned offset, unsigned value)
{
    ram[offset] = value & 0xff;
    ram[offset + 1] = (value >> 8) & 0xff;
}

static void sprite(unsigned offset, unsigned attr, unsigned code,
        unsigned x, unsigned y)
{
    write_word(offset, attr);
    write_word(offset + 2, code);
    write_word(offset + 4, x);
    write_word(offset + 6, y);
}

static void reset(void)
{
    int x, y;
    memset(ram, 0, sizeof(ram));
    memset(snapshot, 0, sizeof(snapshot));
    spriteram_size = sizeof(ram);
    draw_count = 0;
    layers = 0;
    for (y = 0; y < HEIGHT; ++y)
        for (x = 0; x < WIDTH; ++x)
            bitmap.pixels[y][x] = BACKGROUND;
}

/* ROT270: source (x,y) -> display (y, WIDTH - 1 - x). */
static UINT16 displayed_pixel(unsigned x, unsigned y)
{
    return bitmap.pixels[x][WIDTH - 1 - y];
}
"""

TESTS = r"""
int main(void)
{
    unsigned p, x, y;

    /* Empty/control entries may contain nonzero size, priority and colour.
     * Check all passes, including a stale entry beyond the first slot. */
    for (p = 0; p < 4; ++p)
    {
        reset();
        sprite(0, 0x7700 | (p << 6) | 0x38, 0, 0, 0);
        sprite(8, 0x7700 | (p << 6) | 0x38, 0, 128, 64);
        render(p);
        CHECK(draw_count == 0);
        for (y = 0; y < HEIGHT; ++y)
            for (x = 0; x < WIDTH; ++x)
                CHECK(bitmap.pixels[y][x] == BACKGROUND);
        CHECK(displayed_pixel(0, WIDTH - 1) == BACKGROUND);
    }
    puts("PASS: code-zero control/empty entries never draw a rotated corner block");

    /* Do not hide real sprites at the same location, palette or slot. */
    for (p = 0; p < 4; ++p)
    {
        reset();
        sprite(0, p << 6, 0x1234, 0, 0);
        render((p + 1) & 3);
        CHECK(draw_count == 0);
        draw_sprites(&bitmap, &visible, p);
        CHECK(draw_count == 1 && draws[0].tile == 0x1234);
        CHECK(draws[0].color == 0);
        CHECK(displayed_pixel(0, WIDTH - 1) != BACKGROUND);
    }
    puts("PASS: real sprites remain visible at the corner in all four priorities");

    reset();
    sprite(0, 0, 1, 0, 0);
    sprite(8, 0, 2, 0, 0);
    render(0);
    CHECK(draw_count == 2 && draws[0].tile == 2 && draws[1].tile == 1);

    reset();
    sprite(0, 0x1100, 0x1234, 64, 80);
    render(0);
    CHECK(draw_count == 4);
    CHECK(draws[0].tile == 0x1234 && draws[0].x == 64 && draws[0].y == 80);
    CHECK(draws[1].tile == 0x1235 && draws[1].x == 64 && draws[1].y == 96);
    CHECK(draws[2].tile == 0x1236 && draws[2].x == 80 && draws[2].y == 80);

    reset();
    sprite(0, 0x9900, 0x1234, 64, 80);
    render(0);
    CHECK(draw_count == 4 && draws[0].flipx && draws[0].flipy);
    CHECK(draws[0].x == 80 && draws[0].y == 96);
    CHECK(draws[3].x == 64 && draws[3].y == 80);

    /* Only the descriptor's initial code is a sentinel. A valid group that
     * wraps the 16-bit tile index must still expand both tiles. */
    reset();
    sprite(0, 0x0100, 0xffff, 32, 32);
    render(0);
    CHECK(draw_count == 2 && draws[0].tile == 0xffff && draws[1].tile == 0);
    puts("PASS: sprite order, multi-tile expansion, flips and tile-index wrap");

    reset();
    sprite(0, 0, 7, 0x1f8, 0x1f8);
    render(0);
    CHECK(draw_count == 1 && draws[0].x == -8 && draws[0].y == -8);
    CHECK(bitmap.pixels[0][0] == 7 && bitmap.pixels[8][8] == BACKGROUND);

    reset();
    sprite(0xff8, 0, 9, WIDTH - 1, HEIGHT - 1);
    render(0);
    CHECK(draw_count == 1 && bitmap.pixels[HEIGHT - 1][WIDTH - 1] == 9);

    reset();
    sprite(0, 0, 7, 0, 0);
    layers = 0x10;
    render(0);
    CHECK(draw_count == 0);
    puts("PASS: clipping, coordinate wrap, sprite-RAM bounds and layer disable");

    /* The CPU/COP may rewrite just half of a descriptor before display.
     * Neither this mixed record nor a complete next-frame record may leak
     * through without a write to the sprite-latch register. */
    reset();
    sprite(0x100, 0x00c0, 0x1234, 160, 0);
    draw_sprites(&bitmap, &visible, 3);
    CHECK(draw_count == 0); /* no latch yet */
    latch();
    write_word(0x102, 0x52ba); /* new opaque code, old edge coordinates */
    draw_sprites(&bitmap, &visible, 3);
    CHECK(draw_count == 1 && draws[0].tile == 0x1234);
    sprite(0x100, 0x00c0, 0x5678, 80, 80);
    draw_count = 0;
    draw_sprites(&bitmap, &visible, 3);
    CHECK(draw_count == 1 && draws[0].tile == 0x1234 && draws[0].y == 0);
    latch();
    draw_count = 0;
    draw_sprites(&bitmap, &visible, 3);
    CHECK(draw_count == 1 && draws[0].tile == 0x5678 && draws[0].y == 80);
    memset(ram, 0, sizeof(ram));
    latch();
    draw_count = 0;
    draw_sprites(&bitmap, &visible, 3);
    CHECK(draw_count == 0); /* the next latch retires old sprites */
    puts("PASS: partial/live sprite writes are invisible until the hardware latch");

    reset();
    sprite(0xff8, 0, 11, 16, 32);
    render(0);
    CHECK(draw_count == 1 && draws[0].tile == 11);
    spriteram_size = 7;
    draw_count = 0;
    draw_sprites(&bitmap, &visible, 0);
    CHECK(draw_count == 0);
    puts("PASS: complete latched list, including the final slot, and short-buffer guard");
    return 0;
}
"""


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", default=str(
        Path(__file__).resolve().parents[1] / "src/drivers/raiden2.c"))
    parser.add_argument("--cc", default="cc")
    parser.add_argument("--sanitize", action="store_true")
    args = parser.parse_args()
    source = sys.stdin.read() if args.source == "-" else Path(args.source).read_text()
    # Validate real routing, not just a stand-in latch in the raster harness.
    map_text = source.split("static MEMORY_WRITE_START( raiden2_writemem )", 1)[1].split("MEMORY_END", 1)[0]
    entries = re.findall(r"\{\s*(0x[0-9a-f]+),\s*(0x[0-9a-f]+),\s*(\w+)", map_text)
    for address in (0x68e, 0x68f):
        handler = next(h for lo, hi, h in entries if int(lo, 16) <= address <= int(hi, 16))
        assert handler == "buffer_spriteram_w", (hex(address), handler)
    assert "VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM" in source
    print("PASS: both sprite-latch byte lanes precede COP routing; generic save-state buffer enabled", flush=True)
    start = source.index("static UINT16 r2_spr_word(")
    end = source.index("WRITE_HANDLER( raiden2_background_w )", start)
    with tempfile.TemporaryDirectory(prefix="raiden2-sprites-") as tmp:
        harness = Path(tmp) / "sprites.c"
        program = Path(tmp) / "sprites"
        harness.write_text(PREAMBLE + source[start:end] + TESTS)
        command = shlex.split(args.cc) + ["-std=c99", "-O2", "-Wall", "-Wextra", "-Werror"]
        if args.sanitize:
            command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        subprocess.run(command + [str(harness), "-o", str(program)], check=True)
        return subprocess.run([str(program)]).returncode


if __name__ == "__main__":
    sys.exit(main())
