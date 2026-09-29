#pragma code_seg("C801")
#pragma data_seg("D801")
#pragma bss_seg("B801")
#pragma const_seg("K801")
#pragma comment(linker, "/merge:D801=801")
#pragma comment(linker, "/merge:C801=801")
#pragma comment(linker, "/merge:B801=801")
#pragma comment(linker, "/merge:K801=801")
/********************************************************************

 Vampire 1/2 and other Hyperstone-based games

 ***VERY WIP***

 To be used only for testing Hyperstone CPU core, probably the only correct
 thing in the driver so far is the ROM loading and graphics decoding.

 These will be split into separate drivers later.

 CHANGELOG:

 MooglyGuy - 10/25/03
    - Changed prelim driver to only load the ROM in the upper part of mem,
      loading the ROM at 0x00000000 and setting the bank to point there was
      completely wrong since apparently there's RAM at 0x00000000.

*********************************************************************/
#include "driver.h"
#include "machine/eeprom.h"

static data32_t hyperstone_iram[0x1000];
static data32_t *tiles, *wram;
static UINT32 mux_data;

static WRITE32_HANDLER( hyperstone_iram_w )
{
	COMBINE_DATA(&hyperstone_iram[offset&0xfff]);
}

static READ32_HANDLER( hyperstone_iram_r )
{
	return hyperstone_iram[offset&0xfff];
}

static WRITE32_HANDLER( paletteram32_wordx2_w )
{
	int r,g,b;
	COMBINE_DATA(&paletteram32[offset]);

	r = ((paletteram32[offset] & 0x00007c00) >> 10);
	g = ((paletteram32[offset] & 0x000003e0) >> 5);
	b = ((paletteram32[offset] & 0x0000001f) >> 0);

	palette_set_color((offset*2)+1,r<<3,g<<3,b<<3);

	r = ((paletteram32[offset] & 0x7c000000) >> 26);
	g = ((paletteram32[offset] & 0x03e00000) >> 21);
	b = ((paletteram32[offset] & 0x001f0000) >> 16);

	palette_set_color(offset*2,r<<3,g<<3,b<<3);
}

static READ32_HANDLER( oki_r )
{
	return OKIM6295_status_0_r(0);
}

static WRITE32_HANDLER( oki_w )
{
	OKIM6295_data_0_w(0, data);
}

static READ32_HANDLER( ym2151_r )
{
	return YM2151_status_port_0_r(0);
}

static WRITE32_HANDLER( ym2151_w )
{
	static int which = 0;

	if(which)
		YM2151_data_port_0_w(0, data);
	else
		YM2151_register_port_0_w(0,data);

	which ^= 1;
}

static READ32_HANDLER( vamphalf_inputs_r )
{
#define E132XS_CL0 33
	mux_data = activecpu_get_reg(E132XS_CL0);
	mux_data &= 0xf000;
	mux_data >>= 12;
	/*This uses a multiplexer with CL0 register or this is a CPU core bug...*/
	switch(mux_data)
	{
		case 0:	return 0xffff0000 | readinputport(1);
		case 2: return 0xffff0000 | readinputport(0);
		default: return 0xffffffff;
	}
}

static READ32_HANDLER( vamphalf_eeprom_r )
{
	return 0;
	return EEPROM_read_bit();
}

static WRITE32_HANDLER( vamphalf_eeprom_w )
{

}

static ADDRESS_MAP_START( common_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_BASE(&wram)
	AM_RANGE(0x40000000, 0x4003ffff) AM_RAM AM_BASE(&tiles)
	AM_RANGE(0x80000000, 0x8000ffff) AM_READ(MRA32_RAM) AM_WRITE(paletteram32_wordx2_w) AM_BASE(&paletteram32)
	AM_RANGE(0xc0000000, 0xdfffffff) AM_READ(hyperstone_iram_r) AM_WRITE(hyperstone_iram_w)
	AM_RANGE(0xfff80000, 0xffffffff) AM_READ(MRA32_BANK1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vamphalf_io, ADDRESS_SPACE_IO, 32 )
	AM_RANGE(0x030, 0x033) AM_READWRITE(oki_r, oki_w)
	AM_RANGE(0x050, 0x053) AM_READWRITE(ym2151_r, ym2151_w)
	AM_RANGE(0x070, 0x073) AM_READ(vamphalf_eeprom_r)
	AM_RANGE(0x180, 0x183) AM_READWRITE(vamphalf_inputs_r, vamphalf_eeprom_w)
ADDRESS_MAP_END


/*
Vamp 1/2 preliminary sprite list:
Offset+0
-------- -------- xxxxxxxx xxxxxxxx Data
-------- xxxxxxxx -------- -------- Y offs
x------- -------- -------- -------- Flip X (trusted)
-x------ -------- -------- -------- Flip Y
Offset+1
xxxxxxxx xxxxxxxx -------- -------- Color
-------- -------- -------x xxxxxxxx X offs

-------- -------- -------- --------
-------- -------- -------- --------
*/

static void draw_sprites(struct mame_bitmap *bitmap, const struct rectangle *cliprect)
{
	const struct GfxElement *gfx = Machine->gfx[0];
	UINT32 cnt;
	int block, offs;
	int code,color,x,y,fx,fy;
	struct rectangle clip;

	clip.min_x = Machine->visible_area.min_x;
	clip.max_x = Machine->visible_area.max_x;
	clip.min_y = Machine->visible_area.min_y;
	clip.max_y = Machine->visible_area.max_y;

	for (block=0; block<0x8000; block+=0x800)
	{
		clip.min_y= (16-(block/0x800))*16;
		clip.max_y= ((16-(block/0x800))*16)+15;

		for (cnt=0; cnt<0x800; cnt+=8)
		{
			offs = (block + cnt) / 4;
			if(tiles[offs] & 0x01000000) continue;

			code  = tiles[offs] & 0x0000ffff;
			color = (tiles[offs+1] & 0x00ff0000) >> 16;

			x = tiles[offs+1] & 0x000001ff;
			y = 0x100 - ((tiles[offs] & 0x00ff0000) >> 16);

			fx = (tiles[offs] & 0x80000000) >> 31;
			fy = (tiles[offs] & 0x40000000) >> 30;

			drawgfx(bitmap,gfx,code,color,fx,fy,x,y,&clip,TRANSPARENCY_PEN,0);
		}
	}
}

VIDEO_UPDATE( common )
{
	fillbitmap(bitmap,Machine->pens[0],cliprect);
	draw_sprites(bitmap,cliprect);
}


INPUT_PORTS_START( common )
	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP |  IPF_PLAYER1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN |  IPF_PLAYER1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT |  IPF_PLAYER1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP |   IPF_PLAYER2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN |  IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT |  IPF_PLAYER2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static struct GfxLayout sprites_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24, 32,40,48,56, 64,72,80,88 ,96,104,112,120 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*128,
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &sprites_layout, 0, 0x80 },
	{ -1 } /* end of array */
};

static struct YM2151interface ym2151_interface =
{
	1,
	4000000, /* ? */
	{ YM3012_VOL(100,MIXER_PAN_LEFT,100,MIXER_PAN_RIGHT) },
	{ 0 }, /* irq handler */
	{ 0 } /* port_write */
};

static struct OKIM6295interface m6295_interface =
{
	1,              	/* 1 chip */
	{ 10000 },			/* ? */
	{ REGION_SOUND1 },	/* memory region */
	{ 100 }				/* volume */
};

static INTERRUPT_GEN( common_interrupts )
{
	if(cpu_getiloops())
	{
		cpunum_set_input_line(0, 4, PULSE_LINE);
	}
	else
	{
		cpunum_set_input_line(0, 7, PULSE_LINE);
	}
}

static MACHINE_DRIVER_START( common )
	MDRV_CPU_ADD_TAG("main", E132XS, 100000000)		 /* ?? */
	MDRV_CPU_PROGRAM_MAP(common_map,0)
	MDRV_CPU_VBLANK_INT(common_interrupts, 2)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_NVRAM_HANDLER(93C46)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_VISIBLE_AREA(31, 350, 16, 255)

	MDRV_PALETTE_LENGTH(0x8000)
	MDRV_GFXDECODE(gfxdecodeinfo)

	MDRV_VIDEO_UPDATE(common)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( vamphalf )
	MDRV_IMPORT_FROM(common)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(vamphalf_io,0)

	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(YM2151, ym2151_interface)
	MDRV_SOUND_ADD(OKIM6295, m6295_interface)
MACHINE_DRIVER_END



/*

Vamp 1/2 (Semi Vamp)
Danbi, 1999

Official page here...
http://f2.co.kr/eng/product/intro1-17.asp


PCB Layout
----------
             KA12    VROM1.

             BS901   AD-65    ROML01.   ROMU01.
                              ROML00.   ROMU00.
                 62256
                 62256

T2316162A  E1-16T  PROM1.          QL2003-XPL84C

                 62256
                 62256       62256
                             62256
    93C46.IC3                62256
                             62256
    50.000MHz  QL2003-XPL84C
B1 B2 B3                     28.000MHz



Notes
-----
B1 B2 B3:      Push buttons for SERV, RESET, TEST
T2316162A:     Main program RAM
E1-16T:        Hyperstone E1-16T CPU
QL2003-XPL84C: QuickLogic PLCC84 PLD
AD-65:         Compatible to OKI M6295
KA12:          Compatible to Y3012 or Y3014
BS901          Compatible to YM2151
PROM1:         Main program
VROM1:         OKI samples
ROML* / U*:    Graphics, device is MX29F1610ML (surface mounted SOP44 MASK ROM)

*/

ROM_START( vamphalf )
	ROM_REGION32_BE( 0x80000, REGION_USER1, 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "prom1", 0x00000, 0x80000, CRC(f05e8e96) SHA1(c860e65c811cbda2dc70300437430fb4239d3e2d) )

	ROM_REGION( 0x800000, REGION_GFX1, ROMREGION_DISPOSE ) /* 16x16x8 Sprites */
	ROM_LOAD32_WORD( "roml00", 0x000000, 0x200000, CRC(cc075484) SHA1(6496d94740457cbfdac3d918dce2e52957341616) )
	ROM_LOAD32_WORD( "romu00", 0x000002, 0x200000, CRC(711c8e20) SHA1(1ef7f500d6f5790f5ae4a8b58f96ee9343ef8d92) )
	ROM_LOAD32_WORD( "roml01", 0x400000, 0x200000, CRC(626c9925) SHA1(c90c72372d145165a8d3588def12e15544c6223b) )
	ROM_LOAD32_WORD( "romu01", 0x400002, 0x200000, CRC(d5be3363) SHA1(dbdd0586909064e015f190087f338f37bbf205d2) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 ) /* Oki Samples */
	ROM_LOAD( "vrom1", 0x00000, 0x40000, CRC(ee9e371e) SHA1(3ead5333121a77d76e4e40a0e0bf0dbc75f261eb) )
ROM_END



static READ32_HANDLER( vamphalf_speedup_r )
{
	if(activecpu_get_pc() == 0x82de)
		cpu_spinuntil_int();

	return wram[0x4a6d0/4];
}

DRIVER_INIT( vamphalf )
{
	cpu_setbank(1, memory_region(REGION_USER1));
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x0004a6d0, 0x0004a6d3, 0, 0, vamphalf_speedup_r );

}


/*           rom       parent    machine   inp       init */
GAME( 1999, vamphalf, 0, vamphalf, common, vamphalf, ROT0,  "Danbi & F2 System", "Vamp 1/2 (Korea)" )   //gamezfan adds vamphalf

#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()
