/*
   Raiden II driver - Salvia/MAME2003+ playability backport

   This is a targeted backport of the later working Raiden II emulation to
   the MAME 0.78-style API used by Salvia's mame2003-plus core.

   Key additions over the old preliminary driver:
   - V30 program banking
   - Raiden II COP/protection command simulation
   - sprite protection/DMA/sort helpers
   - correct tile banks, CRTC scroll/layer state
   - correct 4-word sprite format + priority passes
   - host-independent Raiden II sprite ROM interleave/decryption
   - Seibu YM2151 + dual OKIM6295 sound path
   - correct input/system/coin layout and sane DIP defaults

   Source basis: later MAME Raiden II driver/r2crypt and current FBNeo's
   Raiden II COP implementation, adapted to this old core API.
*/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "cpu/z80/z80.h"
#include "sndhrdw/seibu.h"
#include "sound/2151intf.h"
#include "sound/adpcm.h"
#include "state.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* seibu_pending_w exists in sndhrdw/seibu.c but is not declared by the old header */
WRITE8_HANDLER( seibu_pending_w );

static struct tilemap *r2play_background_layer,*r2play_midground_layer,*r2play_foreground_layer,*r2play_text_layer;
static unsigned char *r2play_back_data,*r2play_fore_data,*r2play_mid_data;
static int r2play_bg_bank, r2play_mid_bank, r2play_fg_bank, r2play_tx_bank;
static UINT8 r2_scroll[12];
static UINT16 r2_layer_enable;
static UINT16 r2_prg_bank;
/* The old memory system uses REGION_CPU1[0..1ffff] as writable RAM.
   Preserve both switchable ROM pages before the first CPU/reset runs. */
static UINT8 *r2_bank_rom;

#include "raiden2_debug.h"

#include "raiden2_r2crypt.inc"
#include "raiden2_cop.inc"
#include "raiden2_debug.inc"
#include "raiden2_sound.inc"

/* -----------------------------------------------------------------------
 * Video
 * ----------------------------------------------------------------------- */

static UINT16 r2_spr_word(const UINT8 *p)
{
	return (UINT16)(p[0] | ((UINT16)p[1] << 8));
}

static void r2_draw_one_sprite(struct mame_bitmap *bitmap, const struct rectangle *cliprect,
                               int tile, int color, int flipx, int flipy, int sx, int sy)
{
	/* Avoid general drawgfx setup for invisible wraparound copies. */
	if (sx > cliprect->max_x || sy > cliprect->max_y ||
	    sx + 15 < cliprect->min_x || sy + 15 < cliprect->min_y) return;
#if RAIDEN2_DEBUG
	if(sx<=cliprect->max_x && sy<=cliprect->max_y && sx+16>cliprect->min_x && sy+16>cliprect->min_y &&
	   Machine->gfx[2]->pen_usage && (Machine->gfx[2]->pen_usage[tile&0xffff]&~(1U<<15))) r2_trace.tiles[4]++;
#endif
	drawgfx(bitmap, Machine->gfx[2], tile & 0xffff, color & 0x3f,
	        flipx, flipy, sx, sy, cliprect, TRANSPARENCY_PEN, 15);
}

/* Four stable priority lists, rebuilt once per VIDEO_UPDATE from the latch.
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

static void r2play_draw_sprites(struct mame_bitmap *bitmap, const struct rectangle *cliprect, int priority)
{
	/* The SEI25x reads the snapshot latched by the write to 0x68e, not
	 * the CPU's work list. Live RAM can contain a new code/size but the
	 * previous sprite's coordinates while the COP builds the next list. */
	const UINT8 *base = buffered_spriteram;
	UINT16 index;

	if (!base || spriteram_size < 8 || !Machine->gfx[2]) return;
	if (r2_video_enable() & 0x10) return;

	/* COP channel 0x14 is tilemap DMA, not a sprite-list length register.
	 * Walk the complete latched sprite RAM, as on the later MAME driver. */
	index = r2_sprite_head[priority];

	while (index != 0xffff)
	{
		const UINT8 *entry = base + (unsigned)index * 8;
		UINT16 a = r2_spr_word(entry + 0);
		int tile_number = r2_spr_word(entry + 2);
		int sx = r2_spr_word(entry + 4);
		int sy = r2_spr_word(entry + 6);
		int ytlim = ((a >> 12) & 7) + 1;
		int xtlim = ((a >> 8) & 7) + 1;
		int xflip = (a >> 15) & 1;
		int yflip = (a >> 11) & 1;
		int colr = a & 0x3f;
		int pri = (a >> 6) & 3;
		int xstep = 16, ystep = 16;
		int xt, yt;

		/* Code zero marks an empty/control entry (including the sprite-RAM
		 * header), not drawable tile zero. Match MAME's SEI25x sprite device
		 * and reject the whole entry before expanding its size fields. Drawing
		 * it can leave a block at (0,0), the lower-left corner after ROT270. */
		if (tile_number != 0 && pri == priority &&
		    r2_sprite_axis_visible(sx, xtlim * 16, cliprect->min_x, cliprect->max_x) &&
		    r2_sprite_axis_visible(sy, ytlim * 16, cliprect->min_y, cliprect->max_y))
		{
			if (xflip) { ystep = -16; sy += ytlim * 16 - 16; }
			if (yflip) { xstep = -16; sx += xtlim * 16 - 16; }

			for (xt = 0; xt < xtlim; ++xt)
			{
				for (yt = 0; yt < ytlim; ++yt)
				{
					int x = (sx + xstep * xt) & 0x1ff;
					int y = (sy + ystep * yt) & 0x1ff;
					r2_draw_one_sprite(bitmap, cliprect, tile_number, colr, yflip, xflip, x, y);
					r2_draw_one_sprite(bitmap, cliprect, tile_number, colr, yflip, xflip, x - 0x200, y);
					r2_draw_one_sprite(bitmap, cliprect, tile_number, colr, yflip, xflip, x, y - 0x200);
					r2_draw_one_sprite(bitmap, cliprect, tile_number, colr, yflip, xflip, x - 0x200, y - 0x200);
					tile_number++;
				}
			}
		}

		index = r2_sprite_next[index];
	}
}

WRITE8_HANDLER( r2play_background_w )
{
	if (r2play_back_data[offset] == (UINT8)data) return;
	r2play_back_data[offset] = data;
	tilemap_mark_tile_dirty(r2play_background_layer, offset / 2);
}

WRITE8_HANDLER( r2play_midground_w )
{
	if (r2play_mid_data[offset] == (UINT8)data) return;
	r2play_mid_data[offset] = data;
	tilemap_mark_tile_dirty(r2play_midground_layer, offset / 2);
}

WRITE8_HANDLER( r2play_foreground_w )
{
	if (r2play_fore_data[offset] == (UINT8)data) return;
	r2play_fore_data[offset] = data;
	tilemap_mark_tile_dirty(r2play_foreground_layer, offset / 2);
}

WRITE8_HANDLER( r2play_text_w )
{
	if (videoram[offset] == (UINT8)data) return;
	videoram[offset] = data;
	tilemap_mark_tile_dirty(r2play_text_layer, offset / 2);
}

static void r2play_get_back_tile_info(int tile_index)
{
	int offs = tile_index * 2;
	int v = r2play_back_data[offs] | (r2play_back_data[offs + 1] << 8);
	int color = (v >> 12) & 0x0f;
	int tile = (v & 0x0fff) | (r2play_bg_bank << 12);
	SET_TILE_INFO(1, tile, color, 0)
}

static void r2play_get_mid_tile_info(int tile_index)
{
	int offs = tile_index * 2;
	int v = r2play_mid_data[offs] | (r2play_mid_data[offs + 1] << 8);
	int color = ((v >> 12) & 0x0f) + 0x20;
	int tile = (v & 0x0fff) | (r2play_mid_bank << 12);
	SET_TILE_INFO(1, tile, color, 0)
}

static void r2play_get_fore_tile_info(int tile_index)
{
	int offs = tile_index * 2;
	int v = r2play_fore_data[offs] | (r2play_fore_data[offs + 1] << 8);
	int color = ((v >> 12) & 0x0f) + 0x10;
	int tile = (v & 0x0fff) | (r2play_fg_bank << 12);
	SET_TILE_INFO(1, tile, color, 0)
}

static void r2play_get_text_tile_info(int tile_index)
{
	int offs = tile_index * 2;
	int v = videoram[offs] | (videoram[offs + 1] << 8);
	int color = (v >> 12) & 0x0f;
	int tile = (v & 0x0fff) | (r2play_tx_bank << 12);
	SET_TILE_INFO(0, tile, color, 0)
}

VIDEO_START(r2play)
{
	/* Generic video allocates/registers the 4 KiB snapshot for save states.
	 * Do not display uninitialised data before the first hardware latch. */
	memset(buffered_spriteram, 0, spriteram_size);
	r2play_text_layer       = tilemap_create(r2play_get_text_tile_info, tilemap_scan_rows, TILEMAP_TRANSPARENT, 8, 8, 64, 32);
	r2play_background_layer = tilemap_create(r2play_get_back_tile_info, tilemap_scan_rows, TILEMAP_TRANSPARENT, 16, 16, 32, 32);
	r2play_midground_layer  = tilemap_create(r2play_get_mid_tile_info, tilemap_scan_rows, TILEMAP_TRANSPARENT, 16, 16, 32, 32);
	r2play_foreground_layer = tilemap_create(r2play_get_fore_tile_info, tilemap_scan_rows, TILEMAP_TRANSPARENT, 16, 16, 32, 32);
	if (!r2play_text_layer || !r2play_background_layer || !r2play_midground_layer || !r2play_foreground_layer) return 1;
	tilemap_set_transparent_pen(r2play_background_layer, 15);
	tilemap_set_transparent_pen(r2play_midground_layer, 15);
	tilemap_set_transparent_pen(r2play_foreground_layer, 15);
	tilemap_set_transparent_pen(r2play_text_layer, 15);
	return 0;
}

VIDEO_UPDATE(r2play)
{
	int sx, sy;
#if RAIDEN2_DEBUG
	r2_trace.tiles[4]=0;
#endif
	fillbitmap(bitmap, get_black_pen(), cliprect);

	sx = r2_scroll[0] | (r2_scroll[1] << 8); sy = r2_scroll[2] | (r2_scroll[3] << 8);
	tilemap_set_scrollx(r2play_background_layer, 0, sx & 0x1ff); tilemap_set_scrolly(r2play_background_layer, 0, sy & 0x1ff);
	sx = r2_scroll[4] | (r2_scroll[5] << 8); sy = r2_scroll[6] | (r2_scroll[7] << 8);
	tilemap_set_scrollx(r2play_midground_layer, 0, sx & 0x1ff); tilemap_set_scrolly(r2play_midground_layer, 0, sy & 0x1ff);
	sx = r2_scroll[8] | (r2_scroll[9] << 8); sy = r2_scroll[10] | (r2_scroll[11] << 8);
	tilemap_set_scrollx(r2play_foreground_layer, 0, sx & 0x1ff); tilemap_set_scrolly(r2play_foreground_layer, 0, sy & 0x1ff);

	r2_prepare_sprite_lists();

	/* Priority order follows the later working implementation. */
	r2play_draw_sprites(bitmap, cliprect, 0);
	if (!(r2_video_enable() & 0x01)) tilemap_draw(bitmap, cliprect, r2play_background_layer, 0, 0);
	r2play_draw_sprites(bitmap, cliprect, 1);
	if (!(r2_video_enable() & 0x02)) tilemap_draw(bitmap, cliprect, r2play_midground_layer, 0, 0);
	r2play_draw_sprites(bitmap, cliprect, 2);
	if (!(r2_video_enable() & 0x04)) tilemap_draw(bitmap, cliprect, r2play_foreground_layer, 0, 0);
	r2play_draw_sprites(bitmap, cliprect, 3);
	if (!(r2_video_enable() & 0x08)) tilemap_draw(bitmap, cliprect, r2play_text_layer, 0, 0);
	r2_debug_video(bitmap, cliprect);
}

/* -----------------------------------------------------------------------
 * Main CPU memory
 * ----------------------------------------------------------------------- */

static READ8_HANDLER(r2_openbus_r) { return 0xff; }

static READ8_HANDLER(r2_inputs_r)
{
	int value = 0xff;
	switch (offset) {
	case 0: value=readinputport(2); break;
	case 1: value=readinputport(3); break;
	case 4: value=readinputport(0); break;
	case 5: value=readinputport(1); break;
	case 12: value=readinputport(4); break;
	}
	r2_debug_access(0x740+offset, (value&0xff)<<((offset&1)*8), 0);
	return value;
}

static ADDRESS_MAP_START( r2play_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x003ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x00400, 0x006ff) AM_READ(r2_cop_r)
	AM_RANGE(0x00700, 0x0071f) AM_READ(r2_sound_main_r)
	AM_RANGE(0x00740, 0x0074d) AM_READ(r2_inputs_r)
	AM_RANGE(0x00762, 0x00763) AM_READ(r2_dst1_r)
	AM_RANGE(0x00000, 0x007ff) AM_READ(MRA8_RAM) /* unassigned low I/O shadow, as on the Vez map */
	AM_RANGE(0x00800, 0x0bfff) AM_READ(MRA8_RAM)
	AM_RANGE(0x0c000, 0x0cfff) AM_READ(MRA8_RAM)
	AM_RANGE(0x0d000, 0x0d7ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x0d800, 0x0dfff) AM_READ(MRA8_RAM)
	AM_RANGE(0x0e000, 0x0e7ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x0e800, 0x0f7ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x0f800, 0x0ffff) AM_READ(MRA8_RAM)
	AM_RANGE(0x10000, 0x1efff) AM_READ(MRA8_RAM)
	AM_RANGE(0x1f000, 0x1ffff) AM_READ(MRA8_RAM)
	AM_RANGE(0x20000, 0x2ffff) AM_READ(MRA8_BANK3)
	AM_RANGE(0x30000, 0x3ffff) AM_READ(MRA8_BANK4)
	AM_RANGE(0x40000, 0xfffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( r2play_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x003ff) AM_WRITE(MWA8_RAM)
	/* Specific register must precede the overlapping COP range: the old
	 * memory-map builder gives the first matching entry precedence. */
	AM_RANGE(0x0068e, 0x0068f) AM_WRITE(buffer_spriteram_w)
	AM_RANGE(0x00400, 0x006ff) AM_WRITE(r2_cop_w)
	AM_RANGE(0x00700, 0x0071f) AM_WRITE(r2_sound_main_w)
	AM_RANGE(0x00000, 0x007ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x00800, 0x0bfff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x0c000, 0x0cfff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x0d000, 0x0d7ff) AM_WRITE(r2play_background_w) AM_BASE(&r2play_back_data)
	AM_RANGE(0x0d800, 0x0dfff) AM_WRITE(r2play_foreground_w) AM_BASE(&r2play_fore_data)
	AM_RANGE(0x0e000, 0x0e7ff) AM_WRITE(r2play_midground_w) AM_BASE(&r2play_mid_data)
	AM_RANGE(0x0e800, 0x0f7ff) AM_WRITE(r2play_text_w) AM_BASE(&videoram)
	AM_RANGE(0x0f800, 0x0ffff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x10000, 0x1efff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x1f000, 0x1ffff) AM_WRITE(paletteram_xBBBBBGGGGGRRRRR_w) AM_BASE(&paletteram)
	AM_RANGE(0x20000, 0xfffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

/* -----------------------------------------------------------------------
 * Sound CPU: YM2151 + dual OKIM6295
 * ----------------------------------------------------------------------- */

static READ8_HANDLER(r2_sound_z80_r)
{
	int value;
	if(offset<2) value=seibu_soundlatch_r(offset);
	else if(offset==2) value=seibu_main_data_pending_r(0);
	else value=readinputport(5);
	r2_debug_sound_access(0x4010+offset,value,0);
	return value;
}

static WRITE8_HANDLER(r2_sound_z80_reply_w)
{
	seibu_main_data_w(offset,data);
	r2_debug_sound_access(0x4018+offset,data,1);
}

static WRITE8_HANDLER(r2_sound_z80_pending_w)
{
	seibu_pending_w(offset,data);
	r2_debug_sound_access(0x4000,data,1);
}

static ADDRESS_MAP_START( r2play_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x2000, 0x27ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x4009, 0x4009) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0x4010, 0x4013) AM_READ(r2_sound_z80_r)
	AM_RANGE(0x6000, 0x6000) AM_READ(OKIM6295_status_0_r)
	AM_RANGE(0x6002, 0x6002) AM_READ(OKIM6295_status_1_r)
	AM_RANGE(0x8000, 0xffff) AM_READ(MRA8_BANK1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( r2play_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x2000, 0x27ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x4000, 0x4000) AM_WRITE(r2_sound_z80_pending_w)
	AM_RANGE(0x4001, 0x4001) AM_WRITE(seibu_irq_clear_w)
	AM_RANGE(0x4002, 0x4002) AM_WRITE(seibu_rst10_ack_w)
	AM_RANGE(0x4003, 0x4003) AM_WRITE(seibu_rst18_ack_w)
	AM_RANGE(0x4007, 0x4007) AM_WRITE(seibu_bank_w)
	AM_RANGE(0x4008, 0x4008) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x4009, 0x4009) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x4018, 0x4019) AM_WRITE(r2_sound_z80_reply_w)
	AM_RANGE(0x401a, 0x401a) AM_WRITE(seibu_bank_w)
	AM_RANGE(0x401b, 0x401b) AM_WRITE(seibu_coin_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(OKIM6295_data_0_w)
	AM_RANGE(0x6002, 0x6002) AM_WRITE(OKIM6295_data_1_w)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END

static struct YM2151interface r2play_ym2151_interface =
{
	1,
	28636360/8,
	{ YM3012_VOL(50,MIXER_PAN_LEFT,50,MIXER_PAN_RIGHT) },
	{ seibu_ym2151_irqhandler }
};

static struct OKIM6295interface r2play_oki_interface =
{
	2,
	{ (28636360/28)/132, (28636360/28)/132 },
	{ REGION_SOUND1, REGION_SOUND2 },
	{ 40, 40 }
};

/* -----------------------------------------------------------------------
 * Inputs
 * ----------------------------------------------------------------------- */

INPUT_PORTS_START( r2play )
	PORT_START /* P1 low byte */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_PLAYER1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_PLAYER1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_PLAYER1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START /* P2 high byte of modern P1_P2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_PLAYER2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_PLAYER2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_PLAYER2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START /* DSW A */
	PORT_DIPNAME(0x07, 0x07, DEF_STR(Coin_A))
	PORT_DIPSETTING(0x01, DEF_STR(4C_1C))
	PORT_DIPSETTING(0x02, DEF_STR(3C_1C))
	PORT_DIPSETTING(0x04, DEF_STR(2C_1C))
	PORT_DIPSETTING(0x07, DEF_STR(1C_1C))
	PORT_DIPSETTING(0x06, DEF_STR(1C_2C))
	PORT_DIPSETTING(0x05, DEF_STR(1C_3C))
	PORT_DIPSETTING(0x03, DEF_STR(1C_4C))
	PORT_DIPSETTING(0x00, DEF_STR(Free_Play))
	PORT_DIPNAME(0x38, 0x38, DEF_STR(Coin_B))
	PORT_DIPSETTING(0x08, DEF_STR(4C_1C))
	PORT_DIPSETTING(0x10, DEF_STR(3C_1C))
	PORT_DIPSETTING(0x20, DEF_STR(2C_1C))
	PORT_DIPSETTING(0x38, DEF_STR(1C_1C))
	PORT_DIPSETTING(0x30, DEF_STR(1C_2C))
	PORT_DIPSETTING(0x28, DEF_STR(1C_3C))
	PORT_DIPSETTING(0x18, DEF_STR(1C_4C))
	PORT_DIPSETTING(0x00, DEF_STR(Free_Play))
	PORT_DIPNAME(0x40, 0x40, "Starting Coin")
	PORT_DIPSETTING(0x40, "Normal")
	PORT_DIPSETTING(0x00, "X 2")
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Flip_Screen))
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START /* DSW B */
	PORT_DIPNAME(0x03, 0x03, DEF_STR(Difficulty))
	PORT_DIPSETTING(0x03, "Normal")
	PORT_DIPSETTING(0x02, "Easy")
	PORT_DIPSETTING(0x01, "Hard")
	PORT_DIPSETTING(0x00, "Very Hard")
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR(Lives))
	PORT_DIPSETTING(0x00, "1")
	PORT_DIPSETTING(0x04, "4")
	PORT_DIPSETTING(0x08, "2")
	PORT_DIPSETTING(0x0c, "3")
	PORT_DIPNAME(0x30, 0x30, DEF_STR(Bonus_Life))
	PORT_DIPSETTING(0x30, "200000 500000")
	PORT_DIPSETTING(0x20, "400000 1000000")
	PORT_DIPSETTING(0x10, "1000000 3000000")
	PORT_DIPSETTING(0x00, "None")
	PORT_DIPNAME(0x40, 0x40, "Demo Sound")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "Test Mode")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START /* SYSTEM at 0x74c */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START /* coins read by sound CPU */
	PORT_BIT_IMPULSE(0x01, IP_ACTIVE_LOW, IPT_COIN1, 4)
	PORT_BIT_IMPULSE(0x02, IP_ACTIVE_LOW, IPT_COIN2, 4)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/* -----------------------------------------------------------------------
 * Graphics decode
 * ----------------------------------------------------------------------- */

static struct GfxLayout r2play_charlayout =
{
	8,8,4096,4,
	{8,12,0,4},
	{3,2,1,0,19,18,17,16},
	{0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32},
	32*8
};

static struct GfxLayout r2play_tilelayout =
{
	16,16,0x8000,4,
	{8,12,0,4},
	{3,2,1,0,19,18,17,16,3+64*8,2+64*8,1+64*8,0+64*8,19+64*8,18+64*8,17+64*8,16+64*8},
	{0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32},
	128*8
};

static struct GfxLayout r2play_spritelayout =
{
	16,16,0x10000,4,
	{0,1,2,3},
	{4,0,12,8,20,16,28,24,36,32,44,40,52,48,60,56},
	{0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64},
	16*16*4
};

/* The three layouts above consume exactly 4 bits per pixel per tile.
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

static struct GfxDecodeInfo r2play_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0x00000, &r2play_charlayout_raw,   0x700, 0x10 },
	{ REGION_GFX2, 0x00000, &r2play_tilelayout_raw,   0x400, 0x40 },
	{ REGION_GFX3, 0x00000, &r2play_spritelayout_raw, 0x000, 0x40 },
	{ -1 }
};

/* -----------------------------------------------------------------------
 * Machine
 * ----------------------------------------------------------------------- */

static INTERRUPT_GEN( r2play_interrupt )
{
	r2_debug_frame();
	cpunum_set_input_line_and_vector(cpu_getactivecpu(), 0, HOLD_LINE, 0xc0/4);
}

static void r2_state_postload(void)
{
	r2_main_bankswitch(r2_prg_bank);
	if (r2play_background_layer) tilemap_mark_all_tiles_dirty(r2play_background_layer);
	if (r2play_midground_layer) tilemap_mark_all_tiles_dirty(r2play_midground_layer);
	if (r2play_foreground_layer) tilemap_mark_all_tiles_dirty(r2play_foreground_layer);
	if (r2play_text_layer) tilemap_mark_all_tiles_dirty(r2play_text_layer);
}

static void r2_state_save_register(void)
{
	int i;
	state_save_register_int("r2play", 0, "r2play_bg_bank", &r2play_bg_bank);
	state_save_register_int("r2play", 0, "r2play_mid_bank", &r2play_mid_bank);
	state_save_register_int("r2play", 0, "r2play_fg_bank", &r2play_fg_bank);
	state_save_register_int("r2play", 0, "r2play_tx_bank", &r2play_tx_bank);
	state_save_register_UINT8("r2play", 0, "scroll", r2_scroll, 12);
	state_save_register_UINT16("r2play", 0, "layer_enable", &r2_layer_enable, 1);
	state_save_register_UINT16("r2play", 0, "prg_bank", &r2_prg_bank, 1);

	state_save_register_UINT8("r2play_cop", 0, "ram", r2_copram, sizeof(r2_copram));
	state_save_register_UINT32("r2play_cop", 0, "regs", r2_cop_regs, 8);
	state_save_register_UINT32("r2play_cop", 0, "itoa", &r2_cop_itoa, 1);
	state_save_register_UINT16("r2play_cop", 0, "status", &r2_cop_status, 1);
	state_save_register_UINT16("r2play_cop", 0, "scale", &r2_cop_scale, 1);
	state_save_register_UINT16("r2play_cop", 0, "itoa_digit_count", &r2_cop_itoa_digit_count, 1);
	state_save_register_UINT8("r2play_cop", 0, "itoa_digits", r2_cop_itoa_digits, 10);
	state_save_register_UINT16("r2play_cop", 0, "angle", &r2_cop_angle, 1);
	state_save_register_UINT16("r2play_cop", 0, "distance", &r2_cop_dist, 1);
	state_save_register_UINT16("r2play_cop", 0, "angle_target", &r2_cop_angle_target, 1);
	state_save_register_UINT16("r2play_cop", 0, "angle_step", &r2_cop_angle_step, 1);
	state_save_register_UINT16("r2play_cop", 0, "dma_mode", &r2_cop_dma_mode, 1);
	state_save_register_UINT16("r2play_cop", 0, "dma_src", r2_cop_dma_src, 0x200);
	state_save_register_UINT16("r2play_cop", 0, "dma_dst", r2_cop_dma_dst, 0x200);
	state_save_register_UINT16("r2play_cop", 0, "dma_size", r2_cop_dma_size, 0x200);
	state_save_register_UINT16("r2play_cop", 0, "dma_v1", &r2_cop_dma_v1, 1);
	state_save_register_UINT16("r2play_cop", 0, "dma_v2", &r2_cop_dma_v2, 1);
	state_save_register_UINT16("r2play_cop", 0, "dma_adr_rel", &r2_cop_dma_adr_rel, 1);
	state_save_register_UINT16("r2play_cop", 0, "sprites_start", &r2_sprites_cur_start, 1);
	state_save_register_UINT16("r2play_cop", 0, "brightness", &r2_pal_brightness_val, 1);
	state_save_register_UINT16("r2play_cop", 0, "bank", &r2_cop_bank, 1);
	state_save_register_UINT16("r2play_cop", 0, "sprite_x", &r2_sprite_prot_x, 1);
	state_save_register_UINT16("r2play_cop", 0, "sprite_y", &r2_sprite_prot_y, 1);
	state_save_register_UINT16("r2play_cop", 0, "dst1", &r2_dst1, 1);
	state_save_register_UINT16("r2play_cop", 0, "sprite_maxx", &r2_cop_spr_maxx, 1);
	state_save_register_UINT16("r2play_cop", 0, "sprite_off", &r2_cop_spr_off, 1);
	state_save_register_UINT16("r2play_cop", 0, "sprite_src", r2_sprite_prot_src_addr, 2);
	state_save_register_UINT16("r2play_cop", 0, "hit_status", &r2_cop_hit_status, 1);
	state_save_register_UINT16("r2play_cop", 0, "hit_base", &r2_cop_hit_baseadr, 1);
	state_save_register_UINT16("r2play_cop", 0, "hit_value_status", &r2_cop_hit_val_stat, 1);
	state_save_register_INT16("r2play_cop", 0, "hit_values", r2_cop_hit_val, 3);
	state_save_register_UINT32("r2play_cop", 0, "sort_ram", &r2_cop_sort_ram_addr, 1);
	state_save_register_UINT32("r2play_cop", 0, "sort_lookup", &r2_cop_sort_lookup, 1);
	state_save_register_UINT16("r2play_cop", 0, "sort_param", &r2_cop_sort_param, 1);
	for (i = 0; i < 2; ++i) {
		state_save_register_INT16("r2play_collision", i, "pos", r2_collision[i].pos, 3);
		state_save_register_INT8("r2play_collision", i, "delta", r2_collision[i].dx, 3);
		state_save_register_UINT8("r2play_collision", i, "size", r2_collision[i].size, 3);
		state_save_register_int("r2play_collision", i, "allow_swap", &r2_collision[i].allow_swap);
		state_save_register_UINT16("r2play_collision", i, "flags_swap", &r2_collision[i].flags_swap, 1);
		state_save_register_UINT32("r2play_collision", i, "sprite_address", &r2_collision[i].spradr, 1);
		state_save_register_INT16("r2play_collision", i, "minimum", r2_collision[i].min, 3);
		state_save_register_INT16("r2play_collision", i, "maximum", r2_collision[i].max, 3);
	}
	state_save_register_func_postload(r2_state_postload);
}

static MACHINE_INIT(r2play)
{
	machine_init_seibu_sound_1();
	seibu_bank_w(0, 0);

	r2play_bg_bank = 0;
	r2play_mid_bank = 1;
	r2play_fg_bank = 6;
	r2play_tx_bank = 0;
	memset(r2_scroll, 0, sizeof(r2_scroll));
	r2_layer_enable = 0;
	r2_prg_bank = 0;
	r2_reset_cop();
	if (buffered_spriteram)
		memset(buffered_spriteram, 0, spriteram_size);

	/* Working later driver starts on entries 2/3.  Use BANK3/BANK4 so the
	   sound CPU can keep BANK1 for the Seibu sound ROM. */
	r2_main_bankswitch(0);
	r2_debug_reset();
}

static MACHINE_DRIVER_START( r2play )
	MDRV_CPU_ADD(V30, 32000000/2)
	MDRV_CPU_PROGRAM_MAP(r2play_readmem, r2play_writemem)
	MDRV_CPU_VBLANK_INT(r2play_interrupt, 1)

	MDRV_CPU_ADD(Z80, 28636360/8)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
	MDRV_CPU_PROGRAM_MAP(r2play_sound_readmem, r2play_sound_writemem)

	MDRV_FRAMES_PER_SECOND(55.47)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_MACHINE_INIT(r2play)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MDRV_GFXDECODE(r2play_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(2048)
	MDRV_VIDEO_START(r2play)
	MDRV_VIDEO_UPDATE(r2play)

	MDRV_SOUND_ADD(YM2151, r2play_ym2151_interface)
	MDRV_SOUND_ADD(OKIM6295, r2play_oki_interface)
MACHINE_DRIVER_END

/* -----------------------------------------------------------------------
 * ROMs
 * ----------------------------------------------------------------------- */



static DRIVER_INIT(r2play)
{
	UINT8 *gfx = memory_region(REGION_GFX3);
	/* The ROM_RELOAD mirror is immutable and already resident: save 256 KiB. */
	r2_bank_rom = memory_region(REGION_CPU1) + 0x100000;
#if RAIDEN2_DEBUG
	r2_log_path[0] = 0;
	r2_frame_number = r2_reset_count = r2_last_draws = 0;
	r2_ever_lit = r2_was_black = 0;
	r2_old_coin = r2_old_start = 0xff;
#endif
	if (r2play_prepare_and_decrypt_sprites(gfx) != 0)
		logerror("Raiden2: sprite decryption allocation failed\n");
	r2_predecode_packed(memory_region(REGION_GFX1), &r2play_charlayout);
	r2_predecode_packed(memory_region(REGION_GFX2), &r2play_tilelayout);
	r2_predecode_packed(memory_region(REGION_GFX3), &r2play_spritelayout);
	r2_state_save_register();
	seibu_sound_state_save_register();
}

