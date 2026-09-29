"""Apply output-preserving OG Xbox optimizations after apply_raiden2_port.py."""
from pathlib import Path
import re
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from apply_raiden2_port import SRC, OLD, once, read, write

def main():
    path = SRC / "drivers/raiden2_playable.inc"
    code = read(path)
    start = code.index("static void r2play_draw_sprites(")
    code = code[:start] + '''/* Four stable priority lists, rebuilt once per VIDEO_UPDATE from the latch.
 * The old walker inspected all 512 descriptors on each of four passes.
 * 1,032 bytes of scratch state replaces the repeated scans; no allocation.
 */
static UINT16 r2_sprite_next[512], r2_sprite_head[4];

static void r2_prepare_sprite_lists(void)
{
	unsigned i, count;
	for (i = 0; i < 4; ++i) r2_sprite_head[i] = 0xffff;
	if (!buffered_spriteram || spriteram_size < 8) return;
	count = (unsigned)(spriteram_size / 8);
	if (count > 512) count = 512;
	for (i = 0; i < count; ++i)
	{
		const UINT8 *p = buffered_spriteram + i * 8;
		if (r2_spr_word(p + 2) != 0)
		{
			unsigned priority = (r2_spr_word(p) >> 6) & 3;
			r2_sprite_next[i] = r2_sprite_head[priority];
			r2_sprite_head[priority] = (UINT16)i;
		}
	}
}

/* A multi-tile object spans at most 128 pixels on a 512-pixel torus.
 * Reject it only if neither the original nor wrapped extent can be visible.
 */
static int r2_sprite_axis_visible(int position, int length, int lo, int hi)
{
	position &= 0x1ff;
	return (position <= hi && position + length - 1 >= lo) ||
	       (position - 0x200 <= hi && position + length - 1 - 0x200 >= lo);
}

''' + code[start:]
    code = once(code, '\tconst UINT8 *source;\n', '\tconst UINT8 *source;\n\tUINT16 index;\n')
    code = once(code, '\tsource = base + ((spriteram_size - 8) & ~(size_t)7);\n\n\twhile (source >= base)\n\t{',
        '\tindex = r2_sprite_head[priority];\n\n\twhile (index != 0xffff)\n\t{\n\t\tsource = base + (unsigned)index * 8;')
    # C89/VC 7.1 declarations must precede the source assignment.
    code = once(code, '\t\tsource = base + (unsigned)index * 8;\n\t\tUINT16 a =',
        '\t\tconst UINT8 *entry = base + (unsigned)index * 8;\n\t\tUINT16 a =')
    a = code.index("static void r2play_draw_sprites(")
    b = code.index("WRITE8_HANDLER( r2play_background_w )", a)
    walker = code[a:b].replace("\tconst UINT8 *source;\n", "").replace("r2_spr_word(source", "r2_spr_word(entry")
    walker = once(walker, '\t\tif (tile_number != 0 && pri == priority)',
        '\t\tif (tile_number != 0 && pri == priority &&\n\t\t    r2_sprite_axis_visible(sx, xtlim * 16, cliprect->min_x, cliprect->max_x) &&\n\t\t    r2_sprite_axis_visible(sy, ytlim * 16, cliprect->min_y, cliprect->max_y))')
    walker = once(walker, '\t\tif (source == base) break;\n\t\tsource -= 8;', '\t\tindex = r2_sprite_next[index];')
    code = code[:a] + walker + code[b:]
    code = once(code, '\t/* Priority order follows the later working implementation. */',
        '\tr2_prepare_sprite_lists();\n\n\t/* Priority order follows the later working implementation. */')
    # Decode each tile through the REAL core decoder into a 128-byte scratch
    # tile, then reuse its ROM bytes as the packed pixel store (GFX_RAW).
    marker = "static struct GfxDecodeInfo r2play_gfxdecodeinfo[]"
    raw_code = '''/* The three layouts above consume exactly 4 bits per pixel per tile.
 * Decode in place with a 128-byte tile buffer: retain only ONE 12.125 MiB
 * graphics store instead of ROM plus a second complete decoded allocation.
 * GFX_RAW below has the same line/character stride as normal packed decode.
 */
static void r2_predecode_packed(UINT8 *rom, const struct GfxLayout *layout)
{
	struct GfxElement gfx;
	UINT8 tile[128];
	UINT32 i;
	memset(&gfx, 0, sizeof(gfx));
	gfx.width = layout->width;
	gfx.height = layout->height;
	gfx.flags = GFX_PACKED;
	gfx.line_modulo = layout->width / 2;
	gfx.char_modulo = layout->width * layout->height / 2;
	gfx.gfxdata = tile;
	for (i = 0; i < layout->total; ++i)
	{
		UINT8 *source = rom + i * gfx.char_modulo;
		decodechar(&gfx, 0, source, layout);
		memcpy(source, tile, gfx.char_modulo);
	}
}

static struct GfxLayout r2play_charlayout_raw =
{
	8,8,4096,4, { GFX_RAW }, { 0 }, { 8*4 }, 8*8*4
};
static struct GfxLayout r2play_tilelayout_raw =
{
	16,16,0x8000,4, { GFX_RAW }, { 0 }, { 16*4 }, 16*16*4
};
static struct GfxLayout r2play_spritelayout_raw =
{
	16,16,0x10000,4, { GFX_RAW }, { 0 }, { 16*4 }, 16*16*4
};

'''
    code = once(code, marker, raw_code + marker)
    for name in ("charlayout", "tilelayout", "spritelayout"):
        code = once(code, "&r2play_" + name + ",", "&r2play_" + name + "_raw,")
    code = once(code, '\tr2_state_save_register();',
        '\tr2_predecode_packed(memory_region(REGION_GFX1), &r2play_charlayout);\n\tr2_predecode_packed(memory_region(REGION_GFX2), &r2play_tilelayout);\n\tr2_predecode_packed(memory_region(REGION_GFX3), &r2play_spritelayout);\n\tr2_state_save_register();')
    write(path, code)
    parent_path = SRC / "drivers/raiden2.c"
    parent = read(parent_path)
    a = parent.index("ROM_START( raiden2 )")
    b = parent.index("ROM_END", a)
    rom = parent[a:b].replace("ROMREGION_DISPOSE", "0 /* retained packed pixel store */")
    write(parent_path, parent[:a] + rom + parent[b:])
    # The raw layouts select packed storage without changing the decode policy
    # for any other game. Keep the shared drawgfx.c exactly as in the AB build.
    write(SRC / "drawgfx.c", read(OLD / "drawgfx.c"))

    path = SRC / "drivers/raiden2_r2crypt.inc"
    code = read(path)
    start = code.index("static int r2play_prepare_and_decrypt_sprites(")
    code = code[:start] + '''/* Carry propagation terminates at every zero bit of carry_mask.  This
 * parallel form is exactly the bit-serial operation above, including its
 * final bit-31 wrap XOR. The fixed Raiden II mask has only short carry runs.
 */
static UINT32 r2_partial_carry_fast(UINT32 a, UINT32 b, UINT32 mask)
{
	UINT32 result = a ^ b;
	UINT32 carry = a & b;
	UINT32 wrap = 0;
	while (carry & mask)
	{
		carry &= mask;
		wrap ^= carry >> 31;
		carry <<= 1;
		b = result & carry;
		result ^= carry;
		carry = b;
	}
	return result ^ wrap;
}

static int r2play_prepare_and_decrypt_sprites(UINT8 *rom)
{
	/* Five KiB of load-time stack tables replaces per-word bit-walking.
	 * No persistent cache and no extra copy of the 8 MiB sprite region. */
	UINT32 swap32[4][256];
	UINT16 swap16[2][256];
	UINT16 gm[16];
	UINT32 i, byte, lane;
	for (lane = 0; lane < 4; ++lane)
		for (byte = 0; byte < 256; ++byte)
			swap32[lane][byte] = r2_bitswap32(byte << (lane * 8));
	for (lane = 0; lane < 2; ++lane)
		for (byte = 0; byte < 256; ++byte)
			swap16[lane][byte] = r2_bitswap16((UINT16)(byte << (lane * 8)));
	for (i = 0; i < 16; ++i) gm[i] = r2_gm(i);
	for (i = 0; i < 0x800000U / 4U; ++i)
	{
		UINT32 i2 = (i & 0xff) ^ ((i >> 15) & 1);
		UINT32 i1 = i2 ^ (((i >> 20) & 1) << 8);
		UINT32 v = r2_yrot(r2_read32le_buf(rom + i * 4), r2_rotate[i1]);
		UINT16 low = (UINT16)((r2_x5[i2] << 11) ^ r2_x11[(i >> 8) & 0xff] ^ gm[(i >> 16) & 15]);
		UINT16 high = (UINT16)(swap16[0][low & 255] | swap16[1][low >> 8]);
		UINT32 key = ((UINT32)low | ((UINT32)high << 16)) ^ 0x60860000U;
		v = swap32[0][v & 255] | swap32[1][(v >> 8) & 255] |
		    swap32[2][(v >> 16) & 255] | swap32[3][v >> 24];
		r2_write32le_buf(rom + i * 4, r2_partial_carry_fast(v, key, 0x176c91a8U) ^ 0x0f488000U);
	}
	return 0;
}
'''
    write(path, code)
    project = read(OLD.parent / "MAME.vcproj")
    pattern = r'(RelativePath="src\\drivers\\raiden2\.c">.*?Name="Release\|Xbox">\s*<Tool\s*Name="VCCLCompilerTool")'
    project, count = re.subn(pattern, r'\1 Optimization="2" InlineFunctionExpansion="2" EnableFunctionLevelLinking="FALSE"', project, count=1, flags=re.S)
    assert count == 1
    write(SRC.parent / "MAME.vcproj", project)
    print("Applied exact-result sprite priority/culling, decrypt, and in-place packed graphics optimizations.")

if __name__ == "__main__":
    main()
