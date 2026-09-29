#pragma code_seg("C238")
#pragma data_seg("D238")
#pragma bss_seg("B238")
#pragma const_seg("K238")
#pragma comment(linker, "/merge:D238=238")
#pragma comment(linker, "/merge:C238=238")
#pragma comment(linker, "/merge:B238=238")
#pragma comment(linker, "/merge:K238=238")
/* Cross Shooter (c) 1987 Seibu */

/*

Haze's notes :

  - interrupts are probably wrong .. it ends up writing to rom etc.
  - how do the sprites / bg's work .. these big black unknown things on the pcb
    also sound a bit disturbing, what are they?
  - i can't decode the other gfx? rom
  - there don't seem to be any sprites / bg's in ram, interrupts?
  - palette? format isn't understood
  - the other sets ('cshootre' and 'airraid') need decrypting ..
    is the main one protected ? theres a 68705 on it


Stephh's notes (based on the game Z80 code and some tests) :

  - Memory notes (100% guess) :

      * There are still some writes to the ROM area, but I think it's
        related to wrong interrupts and/or incomplete memory mapping.
      * Reads from 0xb0?? seem to be related to sound
      * Write to 0xc500 happens LOTS of time - related to scanlines ?
      * Write to 0xc600 might be used to disable the interrupts and
        the possible communication between CPUs (if they are 2)
      * Write to 0xc700 seems to be done when a coin is inserted
        (also done once during P.O.S.T. - unknown purpose here).
      * Write to 0xc801 might be sort of watchdog as it "pollutes"
        the error.log file.


  - Interrupts notes :

      * I think that they aren't handled correctly : after a few frames,
        the number of lives are reset to 0, causing a "GAME OVER" 8(
			* - or is this protection from the 68705, haze


  - Inputs notes :

      * COINx don't work correcly : see "cshooter_coin_r" read handler.
	* In game, bits 3 and 4 of 0xc202 ("START") are tested,
        while bits 4 and 5 are tested in the "test mode".
      * Pressing STARTx while in game adds lives (depending on the
        "Lives" Dip Switch) for player x.


  - Other notes :

      * 0x0006 contains the "region" stuff (value in 'cshooter' = 0xc4) :
          . bits 2 and 3 determine the manufacturer :
              0x00 : "J.K.H. Corp."         (no logo)
              0x04 : "Taito Corporation."   (+ logo)
              0x08 : "International Games"  (+ logo)
              0x0c : "Seibu Kaihatsu,Inc."  (+ logo)
          . bits 6 and 7 determine the title screen :
              0x00 : unknown - scrambled GFX *probably air raid, haze
              0x40 : unknown - scrambled GFX (alternate entry) *probably air raid, haze
              0x80 : "Cross Shooter"
              0xc0 : "Cross Shooter" (same as 0x80)


  - Addresses :

      * 0xe222 : contents of DSW1 (0xc204)
      * 0xe223 : contents of DSW2 (0xc203)
      * 0xe228 : difficulty (from DSW2)
      * 0xe229 : bonus life (from DSW2 - table at 0x6264)
      * 0xe22b : lives      (from DSW2 - table at 0x7546)
      * 0xe234 : credits (0x00-0xff, even if display is limited to 9)
          . if 1 coin slot , total credits
          . if 2 coin slots, credits for player 1
      * 0xe235 : credits (0x00-0xff, even if display is limited to 9)
          . if 1 coin slot , always 0x00 !
          . if 2 coin slots, credits for player 2
      * 0xe237 : lives for player 1
      * 0xe238 : lives for player 2

*/

#include "driver.h"
#include "sndhrdw/seibu.h"
#include "vidhrdw/generic.h"


/* vidhrdw */

data8_t* cshooter_txram;
static struct tilemap *cshooter_txtilemap;

static void get_cstx_tile_info(int tile_index)
{
	int code = (cshooter_txram[tile_index*2]);
	int attr = (cshooter_txram[tile_index*2+1]);
	int rg;
	rg=0;
	if (attr & 0x20) rg = 1;

	SET_TILE_INFO(

			rg,
			code & 0x1ff,
			attr&0x1f,
			0)
}

WRITE8_HANDLER(cshooter_txram_w)
{
	cshooter_txram[offset] = data;
	tilemap_mark_tile_dirty(cshooter_txtilemap,offset/2);
}

VIDEO_START(cshooter)
{
	cshooter_txtilemap = tilemap_create(get_cstx_tile_info,tilemap_scan_rows,TILEMAP_OPAQUE,8,8,32, 32);

	return 0;
}

VIDEO_UPDATE(cshooter)
{
	tilemap_draw(bitmap,cliprect,cshooter_txtilemap,0,0);
}


/* main cpu */

INTERRUPT_GEN( cshooter_interrupt )
{
	cpunum_set_input_line_and_vector(0, 0, HOLD_LINE, 0x08); // almost certainly wrong?
}


static int cshooter_counter;

static MACHINE_INIT( cshooter )
{
	cshooter_counter = 0;
}

READ8_HANDLER ( cshooter_coin_r )
{
	/* Even reads must return 0xff - Odd reads must return the contents of input port 5.
	   Code at 0x5061 is executed once during P.O.S.T. where there is one read.
	   Code at 0x50b4 is then executed each frame (not sure) where there are 2 reads. */

	return ( (cshooter_counter++ & 1) ? 0xff : input_port_5_r(0) );
}

WRITE8_HANDLER ( cshooter_c500_w )
{
}

WRITE8_HANDLER ( cshooter_c700_w )
{
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(MRA8_RAM)			// to be confirmed
	AM_RANGE(0xb000, 0xb0ff) AM_READ(MRA8_RAM)			// sound related ?
	AM_RANGE(0xc000, 0xc1ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xc200, 0xc200) AM_READ(input_port_0_r)
	AM_RANGE(0xc201, 0xc201) AM_READ(input_port_1_r)
	AM_RANGE(0xc202, 0xc202) AM_READ(input_port_2_r)
	AM_RANGE(0xc203, 0xc203) AM_READ(input_port_3_r)
	AM_RANGE(0xc204, 0xc204) AM_READ(input_port_4_r)
	AM_RANGE(0xc205, 0xc205) AM_READ(cshooter_coin_r)	// hack until I understand
	AM_RANGE(0xd000, 0xd7ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xd800, 0xdfff) AM_READ(MRA8_RAM)
	AM_RANGE(0xe000, 0xffff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(MWA8_RAM)			// to be confirmed
	AM_RANGE(0xc000, 0xc1ff) AM_WRITE(paletteram_BBGGGRRR_w) AM_BASE(&paletteram)	// guess, maybe not
	AM_RANGE(0xc500, 0xc500) AM_WRITE(cshooter_c500_w)
	AM_RANGE(0xc600, 0xc600) AM_WRITE(MWA8_NOP)			// see notes
	AM_RANGE(0xc700, 0xc700) AM_WRITE(cshooter_c700_w)
	AM_RANGE(0xc801, 0xc801) AM_WRITE(MWA8_NOP)			// see notes
	AM_RANGE(0xd000, 0xd7ff) AM_WRITE(cshooter_txram_w) AM_BASE(&cshooter_txram)
	AM_RANGE(0xd800, 0xdfff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xe000, 0xffff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( readport, ADDRESS_SPACE_IO, 8 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport, ADDRESS_SPACE_IO, 8 )
ADDRESS_MAP_END


/* Sound CPU */

static ADDRESS_MAP_START( s_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(MRA8_ROM)
	AM_RANGE(0xf800, 0xffff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( s_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(MWA8_NOP) // YM2203_control_port_0_w ?
	AM_RANGE(0xc001, 0xc001) AM_WRITE(MWA8_NOP) // YM2203_write_port_0_w
	AM_RANGE(0xc800, 0xc800) AM_WRITE(MWA8_NOP) // YM2203_control_port_1_w ?
	AM_RANGE(0xc801, 0xc801) AM_WRITE(MWA8_NOP) // YM2203_write_port_1_w
	AM_RANGE(0xf800, 0xffff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( s_readport, ADDRESS_SPACE_IO, 8 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( s_writeport, ADDRESS_SPACE_IO, 8 )
ADDRESS_MAP_END


INPUT_PORTS_START( cshooter )
	PORT_START	/* IN0	(0xc200) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN1	(0xc201) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* START	(0xc202) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT_IMPULSE( 0x08, IP_ACTIVE_LOW, IPT_START1, 1 )
	PORT_BIT_IMPULSE( 0x10, IP_ACTIVE_LOW, IPT_START2, 1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* DSW2	(0xc203) */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "Easy" )
	PORT_DIPSETTING(    0x02, "Medium" )
	PORT_DIPSETTING(    0x01, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "2k 10k 20k" )
	PORT_DIPSETTING(    0x08, "5k 20k 40k" )
	PORT_DIPSETTING(    0x04, "6k 30k 60k" )
	PORT_DIPSETTING(    0x00, "7k 40k 80k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW1	(0xc204) */
	PORT_DIPNAME( 0x01, 0x01, "Coin Slots" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* COIN	(0xc205) */
	PORT_BIT_IMPULSE( 0x01, IP_ACTIVE_LOW, IPT_COIN1, 1 )
	PORT_BIT_IMPULSE( 0x02, IP_ACTIVE_LOW, IPT_COIN2, 1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static struct GfxLayout cshooter_charlayout =
{
	8,8,		/* 8*8 characters */
	RGN_FRAC(1,1),		/* 512 characters */
	2,			/* 4 bits per pixel */
	{ 0,4 },
	{ 8,9,10,11,0,1,2,3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128*2
};

static struct GfxLayout cshooter_unknownlayout =
{
	8,8,		/* 8*8 characters */
	RGN_FRAC(1,1),		/* 512 characters */
	4,			/* 4 bits per pixel */
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0,     &cshooter_charlayout,   0, 1  },
	{ REGION_GFX1, 128/8, &cshooter_charlayout,   0, 1  },
	{ REGION_GFX2, 0,     &cshooter_unknownlayout,   0, 1  },
	{ -1 } /* end of array */
};

static MACHINE_DRIVER_START( cshooter )
	MDRV_CPU_ADD(Z80,6000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_IO_MAP(readport,writeport)
	MDRV_CPU_VBLANK_INT(cshooter_interrupt,1)

	MDRV_CPU_ADD(Z80,6000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(s_readmem,s_writemem)
	MDRV_CPU_IO_MAP(s_readport,s_writeport)
//	MDRV_CPU_VBLANK_INT(cshooter_interrupt,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_INIT(cshooter)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(cshooter)
	MDRV_VIDEO_UPDATE(cshooter)

	/* sound hardware */
MACHINE_DRIVER_END


/*

-----------------------------
Cross Shooter by TAITO (1987)
-----------------------------
malcor




Location    Type     File ID   Checksum
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LB 4U       27256      R1        C2F0   [  main program  ]
LB 2U       27512      R2        74EA   [  program/GFX   ]
TB 11A      2764       R3        DFA7   [     fix?       ]
LB 5C       27256      R4        D7B8   [ sound program  ]
LB 7A       82S123     0.BPR     00A1   [   forgrounds   ]
LB 9S       82S129     1.BPR     0194   [ motion objects ]
TB 4E       82S129     2.BPR     00DC   [ motion objects ]
LB J3       68705


Notes:   LB - CPU board        S-0086-002-0B
         TB - GFX board        S-0087-807

         The PCB looks like a prototype, due to the modifications
         to the PCB. The game is probably licenced from Seibu.

         The bipolar PROMs are not used for colour.



Brief hardware overview:
------------------------

Main processor  - Z80 6MHz
                - 68705

GFX             - custom TC15G008AP-0048  SEI0040BU
            3 x - custom TC17G008AN-0015  SEI0020BU
                - custom TC17G005AN-0028  SEI0030BU
            3 x - custom SIPs. No ID, unusually large.

Sound processor - Z80 6MHz (5.897MHz)
            2 x - YM2203C


The game data seems to be small. There may be graphics
data in the custom SIPs. I am not sure though.

*/


ROM_START( cshooter )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )	// Main CPU?
	ROM_LOAD( "r1",  0x00000, 0x08000, CRC(fbe8c518) SHA1(bff8319f4892e6d06f1c7a679f67dc8407279cfa) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	// Sub/Sound CPU?
	ROM_LOAD( "r4",  0x00000, 0x08000, CRC(84fed017) SHA1(9a564c9379eb48569cfba48562889277991864d8) )

	ROM_REGION( 0x0800, REGION_CPU3, 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "68705.bin",    0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x02000, REGION_GFX1, 0 )	// TX Layer
	ROM_LOAD( "r3",  0x00000, 0x02000, CRC(67b50a47) SHA1(b1f4aefc9437edbeefba5371149cc08c0b55c741) )	// only 1 byte difference with 3.f11, bad dump?

	ROM_REGION( 0x10000, REGION_GFX2, 0 )	// Sprites & Backgrounds ?
	ROM_LOAD( "r2",  0x00000, 0x10000, CRC(5ddf9f4e) SHA1(69e4d422ca272bf2e9f00edbe7d23760485fdfe6) )

	ROM_REGION( 0x220, REGION_PROMS, 0 )
	ROM_LOAD( "0.bpr", 0x0000, 0x0020, CRC(93e2d292) SHA1(af8edd0cfe85f28ede9604cfaf4516d54e5277c9) )	/* priority? (not used) */
	ROM_LOAD( "1.bpr", 0x0020, 0x0100, CRC(cf14ba30) SHA1(3284b6809075756b3c8e07d9705fc7eacb7556f1) )	/* timing? (not used) */
	ROM_LOAD( "2.bpr", 0x0120, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )	/* timing? (not used) */
ROM_END

/*

Cross Shooter
(C) J K H Corp  (Seibu?)

Seibu Hardware
PCB is coloured black and supposed to be proto, but mask roms are present......?

PCB No. S-0087-011A-0
CPU: SHARP LH0080B (Z80B)
SND: YM2151, Z80A, SEI80BU 611 787, YM3012, SEI0100BU YM3931
RAM: TMM2015 x 7, TMM2063 x 1
DIPs: 2 x 8 position
OTHER: SEI0020BU TC17G008AN-0015 (x 3), SEI0050BU M  6 4 0 00, SEI10040BU TC15G008AP-0048,
       SEI0030BU TC17G005AN-0026, SEI0060BU TC17G008AN-0024
XTAL: 14.318 MHz (near SEI80BU), xx.000 MHz (cant read speed, near SEI0040BU)

There are 3 BIG custom black (resistor?) packs on the PCB.

ROMS:
Note, all ROMs have official sticker, "(C) SEIBU KAIHATSU INC." and a number.

1.k19  TMM27256      \
2.k20  TMM27512      / Program
3.f11  TMM2764         Gfx?
4.g8   TMM24256 Mask   Sound (Samples?)
5.g6   TMM2764         Sound program


*/

ROM_START( cshootre )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	// Main CPU?
	ROM_LOAD( "1.k19",  0x00000, 0x08000, CRC(71418952) SHA1(9745ca006576381c9e9595d8e42ab276bab80a41) )

	ROM_REGION( 2*0x10000, REGION_CPU2, 0 )	// Sub/Sound CPU?
	ROM_LOAD( "5.6f",  0x00000, 0x02000, CRC(30be398c) SHA1(6c61200ee8888d6270c8cec50423b3b5602c2027) )	// 5.g6

	ROM_REGION( 0x02000, REGION_GFX1, 0 )	// TX Layer
	ROM_LOAD( "3.f11",  0x00000, 0x02000, CRC(704c26d7) SHA1(e5964f409cbc2c4752e3969f3e84ace08d5ad9cb) )	// only 1 byte difference with R3, bad dump?

	ROM_REGION( 0x10000, REGION_GFX2, 0 )	// Sprites & Backgrounds ?
	ROM_LOAD( "2.k20",  0x00000, 0x10000, CRC(5812fe72) SHA1(3b28bff6b62a411d2195bb228952db62ad32ef3d) )

	ROM_REGION( 0x08000, REGION_GFX3, 0 )	// ?? sound ?? unknown rom
	ROM_LOAD( "4.7f",  0x00000, 0x08000, CRC(3cd715b4) SHA1(da735fb5d262908ddf7ed7dacdea68899f1723ff) )	// 4.g8
ROM_END

/*

Air Raid (Seibu 1987)
S-0087-011A-0

            82S129        TMM2015      Z80B  2.19J
SEI0020BU                 TMM2015            1.18J
SEI0020BU         63S281                    TMM2063
SEI0020BU         TMM2015
63S281
SEI0050BU                 3.13F

SEI0040BU                                   TMM2015          on
                                     4.7F    YM2151          x x
TMM2015                TMM2015       5.6F    Z80         sw2  x xxxxx
TMM2015               TMM2015                                  x
SEI0030BU          SEI0060BU                             sw1 xx xxxxx
                                   SEI80BU
                                   SEI0100BU(YM3931) YM3012

*/

ROM_START( airraid )
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 )	// Main CPU?
	ROM_LOAD( "1.16j",  0x00000, 0x08000, CRC(7ac2cedf) SHA1(272831f51a2731e067b5aec6dba6bddd3c5350c9) )

	ROM_REGION( 2*0x10000, REGION_CPU2, 0 )	// Sub/Sound CPU?
	ROM_LOAD( "5.6f",  0x00000, 0x02000, CRC(30be398c) SHA1(6c61200ee8888d6270c8cec50423b3b5602c2027) )

	ROM_REGION( 0x02000, REGION_GFX1, 0 )	// TX Layer
	ROM_LOAD( "3.13e",  0x00000, 0x02000, CRC(672ec0e8) SHA1(a11cd90d6494251ceee3bc7c72f4e7b1580b77e2) )

	ROM_REGION( 0x10000, REGION_GFX2, 0 )	// Sprites & Backgrounds ?
	ROM_LOAD( "2.19j",  0x00000, 0x10000, CRC(842ae6c2) SHA1(0468445e4ab6f42bac786f9a258df3972fd1fde9) )

	ROM_REGION( 0x08000, REGION_GFX3, 0 )	// ?? sound ?? unknown rom
	ROM_LOAD( "4.7f",  0x00000, 0x08000, CRC(3cd715b4) SHA1(da735fb5d262908ddf7ed7dacdea68899f1723ff) )
ROM_END


DRIVER_INIT( cshooter )
{
	/* temp so it boots */
	unsigned char *rom = memory_region(REGION_CPU1);

	rom[0xa2] = 0x00;
	rom[0xa3] = 0x00;
	rom[0xa4] = 0x00;

}

DRIVER_INIT( cshootre )
{
	int A;
	unsigned char *rom = memory_region(REGION_CPU1);
	int diff = memory_region_length(REGION_CPU1) / 2;

	memory_set_opcode_base(0,rom+diff);

	for (A = 0x0000;A < 0x8000;A++)
	{
		/* decode the opcodes */
		rom[A+diff] = rom[A];

		if (BIT(A,5) && !BIT(A,3))
			rom[A+diff] ^= 0x40;

		if (BIT(A,10) && !BIT(A,9) && BIT(A,3))
			rom[A+diff] ^= 0x20;

		if ((BIT(A,10) ^ BIT(A,9)) && BIT(A,1))
			rom[A+diff] ^= 0x02;

		if (BIT(A,9) || !BIT(A,5) || BIT(A,3))
			rom[A+diff] = BITSWAP8(rom[A+diff],7,6,1,4,3,2,5,0);

		/* decode the data */
		if (BIT(A,5))
			rom[A] ^= 0x40;

		if (BIT(A,9) || !BIT(A,5))
			rom[A] = BITSWAP8(rom[A],7,6,1,4,3,2,5,0);
	}


	seibu_sound_decrypt(REGION_CPU2,0x2000);
}



GAMEX( 1987, cshooter, 0,        cshooter, cshooter, cshooter, ROT270, "[Seibu Kaihatsu] (Taito license)",  "Cross Shooter (not encrypted)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAMEX( 1987, cshootre, cshooter, cshooter, cshooter, cshootre, ROT270, "[Seibu Kaihatsu] (J.K.H. license)", "Cross Shooter (encrypted)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAMEX( 1987, airraid,  cshooter, cshooter, cshooter, cshootre, ROT270, "Seibu Kaihatsu",                    "Air Raid (encrypted)", GAME_NOT_WORKING | GAME_NO_SOUND )

/*

Jolly Jogger
Taito, 1982

PCB Layout
----------

FGO70007
KDN00004
KDK00502
|----------------------------------------------------------|
|             TMM416  TMM416  TMM416  TMM416               |
|             TMM416  TMM416  TMM416  TMM416              |-|
|             TMM416  TMM416  TMM416  TMM416              | |
|             TMM416  TMM416  TMM416  TMM416              | |
|                                                         | |
|                                                         | |
|                                                         | |
|                                                         | |
|2                                                        |-|
|2                                                         |
|W   DSW1(8)                                     KD13.1F   |
|A                                                         |
|Y                                                         |
|                                                         |-|
|                                                         | |
|                                                         | |
|                                                         | |
|                MB14241                                  | |
|    DSW2(8)                                              | |
|                                                         | |
|                                                         |-|
|    VOL         AY3-8910     3.579545MHz                  |
|----------------------------------------------------------|

FGO70008
KDN00007
|----------------------------------------------------------|
|                                                          |
|                                            6116         |-|
|                                                         | |
|                                            KD21.8J      | |
|   D2125          D2125                                  | |
|                                            KD20.8H      | |
|   D2125          D2125                                  | |
|                                            KD19.8F      | |
|   D2125          D2125                                  |-|
|                                            KD18.8E       |
|   D2125          D2125                                   |
|                                            KD17.8D       |
|   D2125          D2125        Z80                        |
|                                            KD16.8C      |-|
|                                                         | |
|                                            KD15.8B      | |
|                                                         | |
|                                            KD14.8A      | |
|                                                         | |
|                                                         | |
|                                                         |-|
|                                                          |
|----------------------------------------------------------|

FGO70009
KDN00006
|----------------------------------------------------------|
|                                                18MHz     |
|                                                         |-|
|                                                         | |
|                                                         | |
|                                                         | |
|                                                         | |
|                                                         | |
|                         KD11.5H   KD12.7H               | |
|                                                         |-|
|1                                                         |
|8                                                         |
|W                                                         |
|A                                                         |
|Y                                                        |-|
|                                                         | |
|                                                         | |
|                                                         | |
|   KD09.1C  KD10.2C                                      | |
|                                                         | |
|                                                         | |
|               2114  2114     2114  2114  2114  2114     |-|
|                                                          |
|----------------------------------------------------------|


  driver by Pierpaolo Prazzoli

Notes:
  The hardware is very similar to Galaxian but has some differencies, like the 3bpp bitmap addition
  To advance input tests, press Tilt button

*/


static struct tilemap *bg_tilemap;
static UINT8 nmi_enable = 0, jullyjgr_flip_screen_x = 0, jullyjgr_flip_screen_y = 0, bitmap_disable = 0, tilemap_bank = 0, pri = 0;
static UINT8 *jollyjgr_bitmap;
static UINT8 *bulletram;

static WRITE8_HANDLER( jollyjgr_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

static WRITE8_HANDLER( jollyjgr_attrram_w )
{
	if(offset & 1)
	{
		/* color change */
		int i;

		for (i = offset >> 1; i < 0x0400; i += 32)
			tilemap_mark_tile_dirty(bg_tilemap, i);
	}
	else
	{
		tilemap_set_scrolly(bg_tilemap, offset >> 1, data);
	}

	colorram[offset] = data;
}

static WRITE8_HANDLER( jollyjgr_misc_w )
{
	/* they could be swapped, because it always set "data & 3" */
	jullyjgr_flip_screen_x = data & 1;
	jullyjgr_flip_screen_y = data & 2;
	
	/* same for these two (used by Frog & Spiders) */
	bitmap_disable = data & 0x40;
	tilemap_bank = data & 0x20;

    pri = data & 4;

	tilemap_set_flip(bg_tilemap, (jullyjgr_flip_screen_x ? TILEMAP_FLIPX : 0) | (jullyjgr_flip_screen_y ? TILEMAP_FLIPY : 0));

	nmi_enable = data & 0x80;
}

static WRITE8_HANDLER( jollyjgr_coin_lookout_w )
{
	coin_lockout_global_w(data & 1);

	/* bits 4, 5, 6 and 7 are used too */
}

static ADDRESS_MAP_START( jollyjgr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8ff8, 0x8ff8) AM_READ(input_port_0_r)
	AM_RANGE(0x8ff9, 0x8ff9) AM_READ(input_port_1_r)
	AM_RANGE(0x8ff8, 0x8ff8) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x8ffa, 0x8ffa) AM_READWRITE(input_port_2_r, AY8910_write_port_0_w)
	AM_RANGE(0x8fff, 0x8fff) AM_READ(input_port_3_r)
	AM_RANGE(0x8ffc, 0x8ffc) AM_WRITE(jollyjgr_misc_w)
	AM_RANGE(0x8ffd, 0x8ffd) AM_WRITE(jollyjgr_coin_lookout_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_WRITE(jollyjgr_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x9800, 0x983f) AM_RAM AM_WRITE(jollyjgr_attrram_w) AM_BASE(&colorram)
	AM_RANGE(0x9840, 0x987f) AM_RAM AM_BASE(&spriteram)
	AM_RANGE(0x9880, 0x9bff) AM_RAM
	AM_RANGE(0xa000, 0xffff) AM_RAM AM_BASE(&jollyjgr_bitmap)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fspider_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8ff8, 0x8ff8) AM_READ(input_port_0_r)
	AM_RANGE(0x8ff9, 0x8ff9) AM_READ(input_port_1_r)
	AM_RANGE(0x8ff8, 0x8ff8) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x8ffa, 0x8ffa) AM_READWRITE(input_port_2_r, AY8910_write_port_0_w)
	AM_RANGE(0x8fff, 0x8fff) AM_READ(input_port_3_r)
	AM_RANGE(0x8ffc, 0x8ffc) AM_WRITE(jollyjgr_misc_w)
	AM_RANGE(0x8ffd, 0x8ffd) AM_WRITE(jollyjgr_coin_lookout_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_WRITE(jollyjgr_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x9800, 0x983f) AM_RAM AM_WRITE(jollyjgr_attrram_w) AM_BASE(&colorram)
	AM_RANGE(0x9840, 0x987f) AM_RAM AM_BASE(&spriteram)
	AM_RANGE(0x9880, 0x989f) AM_RAM /* ? */
	AM_RANGE(0x98a0, 0x98af) AM_RAM AM_BASE(&bulletram)
	AM_RANGE(0x98b0, 0x9bff) AM_RAM /* ? */
	AM_RANGE(0xa000, 0xffff) AM_RAM AM_BASE(&jollyjgr_bitmap)
ADDRESS_MAP_END


INPUT_PORTS_START( jollyjgr )
	PORT_START
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x03, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x10, "Timer" )
	PORT_DIPSETTING(    0x18, "2 min 20 sec" )
	PORT_DIPSETTING(    0x10, "2 min 40 sec" )
	PORT_DIPSETTING(    0x08, "3 min" )
	PORT_DIPSETTING(    0x00, "3 min 20 sec" )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) ) /* it works only when Cabinet is set to Upright */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN |  IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT |  IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP |    IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN |  IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT |  IPF_4WAY | IPF_COCKTAIL )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_START
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x00, "Display Coinage" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Display Year" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "No Hit" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Number of Coin Switches" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )
INPUT_PORTS_END


INPUT_PORTS_START( fspider )
	PORT_START
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ) )

	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_8C ) )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_2WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_2WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_2WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_2WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_COCKTAIL )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_START
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x0c, "40000" )
	PORT_DIPNAME( 0x10, 0x10, "Display Coinage Settings" )	
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Show only 1P Coinage" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static PALETTE_INIT( jollyjgr )
{
	int i;

	/* tilemap / sprites palette */
	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = BIT(*color_prom,0);
		bit1 = BIT(*color_prom,1);
		bit2 = BIT(*color_prom,2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = BIT(*color_prom,3);
		bit1 = BIT(*color_prom,4);
		bit2 = BIT(*color_prom,5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = BIT(*color_prom,6);
		bit1 = BIT(*color_prom,7);
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(i,r,g,b);
		color_prom++;
	}

	/* bitmap palette */
	for (i = 0;i < 8;i++)
	{
		int r = (i >> 0) & 1;
		int g = (i >> 1) & 1;
		int b = (i >> 2) & 1;

		palette_set_color(32 + i,r * 0xff,g * 0xff,b * 0xff);
	}
}

/* Tilemap is the same as in Galaxian */
static void get_bg_tile_info(int tile_index)
{
	int color = colorram[((tile_index & 0x1f) << 1) | 1] & 7;
	int region = (tilemap_bank & 0x20) ? 2 : 0;
	SET_TILE_INFO(region, videoram[tile_index], color, 0);
}

VIDEO_START( jollyjgr )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows, TILEMAP_TRANSPARENT, 8, 8, 32, 32);

	if(!bg_tilemap)
		return 1;

	tilemap_set_transparent_pen(bg_tilemap, 0);
	tilemap_set_scroll_cols(bg_tilemap, 32);

	return 0;
}

static void draw_bitmap(struct mame_bitmap *bitmap)
{
	int x,y,count;
	int i, bit0, bit1, bit2;
	int color;

	count = 0;
	for (y=0;y<256;y++)
	{
		for (x=0;x<256/8;x++)
		{
			for(i = 0; i < 8; i++)
			{
				bit0 = (jollyjgr_bitmap[count] >> i) & 1;
				bit1 = (jollyjgr_bitmap[count + 0x2000] >> i) & 1;
				bit2 = (jollyjgr_bitmap[count + 0x4000] >> i) & 1;
				color = bit0 | (bit1 << 1) | (bit2 << 2);

				if(color)
				{
					if(jullyjgr_flip_screen_x && jullyjgr_flip_screen_y)
						plot_pixel(bitmap, x*8 + i, y, Machine->pens[color + 32]);
					else if(jullyjgr_flip_screen_x && !jullyjgr_flip_screen_y)
						plot_pixel(bitmap, x*8 + i, 255 - y, Machine->pens[color + 32]);
					else if(!jullyjgr_flip_screen_x && jullyjgr_flip_screen_y)
						plot_pixel(bitmap, 255 - x*8 - i, y, Machine->pens[color + 32]);
					else
						plot_pixel(bitmap, 255 - x*8 - i, 255 - y, Machine->pens[color + 32]);
				}
			}

			count++;
		}
	}
}

VIDEO_UPDATE( jollyjgr )
{
	int offs;

	fillbitmap(bitmap,Machine->pens[32],cliprect);

	if(pri) /* used in Frog & Spiders level 3 */
	{
		if(!(bitmap_disable))
			draw_bitmap(bitmap);

		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

		if(!(bitmap_disable))
			draw_bitmap(bitmap);
	}

	/* Sprites are the same as in Galaxian */
	for(offs = 0; offs < 0x40; offs += 4)
	{
		int sx = spriteram[offs + 3] + 1;
		int sy = spriteram[offs];
		int flipx = spriteram[offs + 1] & 0x40;
		int flipy = spriteram[offs + 1] & 0x80;
		int code = spriteram[offs + 1] & 0x3f;
		int color = spriteram[offs + 2] & 7;

		if (jullyjgr_flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (jullyjgr_flip_screen_y)
		{
			flipy = !flipy;
		}
		else
		{
			sy = 240 - sy;
		}

		if (offs < 3*4)  sy++;

		drawgfx(bitmap,Machine->gfx[1],
				code,color,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);

	}
}

VIDEO_UPDATE( fspider )
{
	int offs, x;

	fillbitmap(bitmap,Machine->pens[32],cliprect);


	if(pri) /* used in Frog & Spiders level 3 */
	{
		if(!(bitmap_disable))
			draw_bitmap(bitmap);

		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

		if(!(bitmap_disable))
			draw_bitmap(bitmap);
	}

	/* Sprites are the same as in Galaxian */
	for(offs = 0; offs < 0x40; offs += 4)
	{
		int sx = spriteram[offs + 3] + 1;
		int sy = spriteram[offs];
		int flipx = spriteram[offs + 1] & 0x40;
		int flipy = spriteram[offs + 1] & 0x80;
		int code = spriteram[offs + 1] & 0x3f;
		int color = spriteram[offs + 2] & 7;

		if (jullyjgr_flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (jullyjgr_flip_screen_y)
		{
			flipy = !flipy;
		}
		else
		{
			sy = 240 - sy;
		}

		if (offs < 3*4)  sy++;

		drawgfx(bitmap,Machine->gfx[1],
				code,color,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);

	}

	/* Draw bullets
	16 bytes, 2 bytes per bullet (y,x). 2 player bullets, 6 enemy bullets.
	Assume bullets to look the same as on Galaxian hw,
	that is, simply 4 pixels. Colours are unknown. */
	for (offs=0;offs<0x10;offs+=2) {
		UINT8 sy=~bulletram[offs];
		UINT8 sx=~bulletram[offs|1];
		UINT16 bc=(offs<4)?
			32+7: /* player, white */
			32+3; /* enemy, yellow */

		if (jullyjgr_flip_screen_y) sy^=0xff;
		if (jullyjgr_flip_screen_y) sx+=8;

		if (sy>=cliprect->min_y && sy<=cliprect->max_y)
			for (x=sx-4;x<sx;x++)
				if (x>=cliprect->min_x && x<=cliprect->max_x)
				((UINT16 *)(bitmap->line[sy]))[x]=bc;
	}
	
}


static struct GfxLayout jollyjgr_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static struct GfxLayout jollyjgr_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static struct GfxDecodeInfo jollyjgr_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &jollyjgr_charlayout,   0, 8 },
	{ REGION_GFX2, 0, &jollyjgr_spritelayout, 0, 8 },
	{ REGION_GFX3, 0, &jollyjgr_charlayout,   0, 8 },
	{ -1 }
};

static INTERRUPT_GEN( jollyjgr_interrupt )
{
	if(nmi_enable)
		cpunum_set_input_line(0, INPUT_LINE_NMI, PULSE_LINE);
}

static struct AY8910interface ay8910_interface =
{
	1,
	3579545,
	{45},
	{0},
	{0},
	{0},
	{0}
};

static MACHINE_DRIVER_START( jollyjgr )
	MDRV_CPU_ADD(Z80, 3579545)		 /* 3,579545 MHz */
	MDRV_CPU_PROGRAM_MAP(jollyjgr_map,0)
	MDRV_CPU_VBLANK_INT(jollyjgr_interrupt,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(jollyjgr_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(32+8) /* 32 for tilemap and sprites + 8 for the bitmap */

	MDRV_PALETTE_INIT(jollyjgr)
	MDRV_VIDEO_START(jollyjgr)
	MDRV_VIDEO_UPDATE(jollyjgr)

	/* sound hardware */
	MDRV_SOUND_ADD(AY8910, ay8910_interface)
MACHINE_DRIVER_END
	
static MACHINE_DRIVER_START( fspider )
	MDRV_CPU_ADD(Z80, 3579545)		 /* 3,579545 MHz */
	MDRV_CPU_PROGRAM_MAP(fspider_map,0)
	MDRV_CPU_VBLANK_INT(jollyjgr_interrupt,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(jollyjgr_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(32+8) /* 32 for tilemap and sprites + 8 for the bitmap */

	MDRV_PALETTE_INIT(jollyjgr)
	MDRV_VIDEO_START(jollyjgr)
	MDRV_VIDEO_UPDATE(fspider)

	/* sound hardware */
	MDRV_SOUND_ADD(AY8910, ay8910_interface)
MACHINE_DRIVER_END


ROM_START( jollyjgr )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "kd14.8a",      0x0000, 0x1000, CRC(404cfa2b) SHA1(023abecbc614d1deb6a239906f62e25bb688ac14) )
	ROM_LOAD( "kd15.8b",      0x1000, 0x1000, CRC(4cdc4c8b) SHA1(07257863a2de3a0e6bc1b41b8dcaae8c89bc4720) )
	ROM_LOAD( "kd16.8c",      0x2000, 0x1000, CRC(a2fa3500) SHA1(b85439e43a31c3445420896c231ac59f95331226) )
	ROM_LOAD( "kd17.8d",      0x3000, 0x1000, CRC(a5ab8c89) SHA1(b5a41581bba1c26ac3819aded29aeb48a5de64f8) )
	ROM_LOAD( "kd18.8e",      0x4000, 0x1000, CRC(8547c9a7) SHA1(43319a2271a294c2876e87a30bb7667b67582b76) )
	ROM_LOAD( "kd19.8f",      0x5000, 0x1000, CRC(2d0ed544) SHA1(547bcecd7d97f343de721d6af5f13ae6f787d838) )
	ROM_LOAD( "kd20.8h",      0x6000, 0x1000, CRC(017a0e5a) SHA1(23e066fea44690279ff7b3b65b03e4096e4d2984) )
	ROM_LOAD( "kd21.8j",      0x7000, 0x1000, CRC(e4faed01) SHA1(9b9afaff6cc4addfed7c1929a0d845bfbe9d18cc) )

	ROM_REGION( 0x1000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "kd09.1c",      0x0000, 0x0800, CRC(ecafd535) SHA1(a1f0bee247e6ab4f9fc3578560b62f5913b4ece2) )
	ROM_LOAD( "kd10.2c",      0x0800, 0x0800, CRC(e40fc594) SHA1(1a9bd670dda0405600e4c4d0107b881906969991) )

	ROM_REGION( 0x1000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "kd11.5h",      0x0000, 0x0800, CRC(d686245c) SHA1(73567b15d9399e450121ad01ad2dcb91bedc1099) )
	ROM_LOAD( "kd12.7h",      0x0800, 0x0800, CRC(d69cbb4e) SHA1(f33cc161f93cae9cc314067fa2453838fa8ac3ba) )
	
	ROM_REGION( 0x2000, REGION_GFX3, ROMREGION_ERASE00 )

	/* it's identical to kd14.8a, except for the first 32 bytes which are palette bytes */
	ROM_REGION( 0x1000, REGION_PROMS, 0 )
	ROM_LOAD( "kd13.1f",      0x0000, 0x1000, CRC(4f4e4e13) SHA1(a8fe0e1fd354e6cc2cf65eab66882c3b98c82100) )
ROM_END

ROM_START( fspiderb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.5l",   0x0000, 0x1000, CRC(3679cab1) SHA1(21c055a1e6e42dda7070452a5de9bce01fa256b1) )
	ROM_LOAD( "2.8b",   0x7000, 0x1000, CRC(6e543acf) SHA1(45b66068654aabfe9745c8c893424a6d360a1748) )
	ROM_LOAD( "3.8c",   0x6000, 0x1000, CRC(f74f83d9) SHA1(18fdd7ebf5096b022207de4504e254860428f469) )
	ROM_LOAD( "4.8e",   0x5000, 0x1000, CRC(26add629) SHA1(96a2691f81ea7d0215d99c45f7f6dad4a4a6844e) )
	ROM_LOAD( "5.8f",   0x4000, 0x1000, CRC(0457de7b) SHA1(e7a7765d971d328333e8784178762c6fe4e2d783) )
	ROM_LOAD( "6.8h",   0x2000, 0x1000, CRC(329d4716) SHA1(898d0f3d053a8ac90c731f39c1d05769882cdcf6) )
	ROM_LOAD( "7.8j",   0x3000, 0x1000, CRC(a7d8fc3c) SHA1(3b39155001d21e75f16196bf3c11b34fb6d5fa0b) )
	ROM_RELOAD(         0x1000, 0x1000 )

	ROM_REGION( 0x4000, REGION_USER1, 0 )
	ROM_LOAD(  "8.1c",   0x0000, 0x1000, CRC(4e39abad) SHA1(225a2a08a7afe404e6b74789aab8c97a39a21214) )
	ROM_LOAD(  "9.2c",   0x1000, 0x1000, CRC(04dd1604) SHA1(9e686b09e2fc59fa879fd62982adb1c681f3eb73) )
	ROM_LOAD( "10.5h",   0x2000, 0x1000, CRC(d4bce323) SHA1(f49df8318aa9e8bd49fad1931480dfd483a0248a) )
	ROM_LOAD( "11.7h",   0x3000, 0x1000, CRC(7ab56309) SHA1(b43f542a7359c3a4ccf6f116e3a84bd13af6876f) )

	ROM_REGION( 0x1000, REGION_GFX1, 0 )
	ROM_COPY( REGION_USER1, 0x0000, 0x0000, 0x800)
	ROM_COPY( REGION_USER1, 0x1000, 0x0800, 0x800)

	ROM_REGION( 0x1000, REGION_GFX2, 0 )
	ROM_COPY( REGION_USER1, 0x2000, 0x0000, 0x800)
	ROM_COPY( REGION_USER1, 0x3000, 0x0800, 0x800)

	ROM_REGION( 0x1000, REGION_GFX3, ROMREGION_ERASE00 )
	ROM_COPY( REGION_USER1, 0x0800, 0x0400, 0x400)
	ROM_COPY( REGION_USER1, 0x1800, 0x0c00, 0x400)
/*	ROM_COPY( REGION_USER1, 0x2800, 0x1000, 0x800) */
/*	ROM_COPY( REGION_USER1, 0x3800, 0x1800, 0x800) */

	ROM_REGION( 0x1000, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.1f", 0x0000, 0x0020, CRC(cda6001a) SHA1(e10fe848e8123e53bd2db8a14cfa2d8c6621d6aa) )
ROM_END

GAME( 1981, fspiderb, 0, fspider,  fspider,  0, ROT90, "Taito Corporation", "Frog & Spiders (bootleg)" ) /* comes from a Fawaz Group bootleg board */
GAME( 1982, jollyjgr, 0, jollyjgr, jollyjgr, 0, ROT90, "Taito Corporation", "Jolly Jogger" )

/* gamezfan and Mahoneyt944 add support for the jollyjgr driver which supports two new games 
Frog & Spiders and Jolly Jogger just dropped it in here for ease of porting
*/

#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()
