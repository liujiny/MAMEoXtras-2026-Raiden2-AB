#pragma code_seg("C164")
#pragma data_seg("D164")
#pragma bss_seg("B164")
#pragma const_seg("K164")
#pragma comment(linker, "/merge:D164=164")
#pragma comment(linker, "/merge:C164=164")
#pragma comment(linker, "/merge:B164=164")
#pragma comment(linker, "/merge:K164=164")
/***************************************************************************

 Bishi Bashi Champ Mini Game Senshuken
 (c) 1996 Konami

 Driver by R. Belmont

 WORKING: ram/rom test passes, IRQs, sound/music, inputs ,colors.
 TODO: some minor visrgn/layer alignment problems

***************************************************************************/

#include "driver.h"
#include "state.h"
#include "vidhrdw/generic.h"
#include "vidhrdw/konamiic.h"
#include "cpu/m68000/m68000.h"

VIDEO_START(bishi);
VIDEO_UPDATE(bishi);

/*static int init_eeprom_count;*/
static data16_t cur_control;

static READ16_HANDLER( control_r )
{
	return cur_control;
}

static WRITE16_HANDLER( control_w )
{
	COMBINE_DATA(&cur_control);
}

static INTERRUPT_GEN(bishi_interrupt)
{
	switch (cpu_getiloops())
	{
		case 0:
			cpunum_set_input_line(0, MC68000_IRQ_3, HOLD_LINE);
			break;

		case 1:
			cpunum_set_input_line(0, MC68000_IRQ_4, HOLD_LINE);
			break;
	}
}

/* compensate for a bug in the ram/rom test */
static READ16_HANDLER( bishi_mirror_r )
{
	return paletteram16[offset];
}

static READ16_HANDLER( bishi_sound_r )
{
	return YMZ280B_status_0_r(offset);
}

static WRITE16_HANDLER( bishi_sound_w )
{
 	if (offset)
		YMZ280B_data_0_w(offset, data>>8);
 	else
		YMZ280B_register_0_w(offset, data>>8);
}

static READ16_HANDLER( dipsw_r )	// dips
{
	return input_port_1_r(0) | (input_port_5_r(0)<<8);
}

static READ16_HANDLER( player1_r ) 	// players 1 and 3
{
	return 0xff | (input_port_2_r(0)<<8);
}

static READ16_HANDLER( player2_r )	// players 2 and 4
{
	return input_port_3_r(0) | (input_port_4_r(0)<<8);
}

static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x400000, 0x407fff) AM_READ(MRA16_RAM)		// work RAM
	AM_RANGE(0x800000, 0x800001) AM_READ(control_r)
	AM_RANGE(0x800004, 0x800005) AM_READ(dipsw_r)
	AM_RANGE(0x800006, 0x800007) AM_READ(player1_r)
	AM_RANGE(0x800008, 0x800009) AM_READ(player2_r)
	AM_RANGE(0x880000, 0x880003) AM_READ(bishi_sound_r)
	AM_RANGE(0xa00000, 0xa01fff) AM_READ(K056832_ram_word_r)	// VRAM
	AM_RANGE(0xb00000, 0xb03fff) AM_READ(MRA16_RAM)
	AM_RANGE(0xb04000, 0xb047ff) AM_READ(bishi_mirror_r)		// bug in the ram/rom test?
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x400000, 0x407fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(control_w)
	AM_RANGE(0x820000, 0x820001) AM_WRITE(MWA16_NOP)		// lamps (see lamp test in service menu)
	AM_RANGE(0x830000, 0x83003f) AM_WRITE(K056832_word_w)
	AM_RANGE(0x840000, 0x840007) AM_WRITE(K056832_b_word_w)	// VSCCS
	AM_RANGE(0x850000, 0x85001f) AM_WRITE(K054338_word_w)		// CLTC
	AM_RANGE(0x870000, 0x8700ff) AM_WRITE(K055555_word_w)		// PCU2
	AM_RANGE(0x880000, 0x880003) AM_WRITE(bishi_sound_w)
	AM_RANGE(0xa00000, 0xa01fff) AM_WRITE(K056832_ram_word_w)	/* Graphic planes */
	AM_RANGE(0xb00000, 0xb03fff) AM_WRITE(paletteram16_xbgr_word_w) AM_BASE(&paletteram16)
ADDRESS_MAP_END

INPUT_PORTS_START( bishi )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START	// dips bank 1
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "Easiest" )
	PORT_DIPSETTING(    0x06, "Very Easy" )
	PORT_DIPSETTING(    0x05, "Easy" )
	PORT_DIPSETTING(    0x04, "Medium" )
	PORT_DIPSETTING(    0x03, "Medium Hard" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x01, "Very Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x38, 0x28, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0xc0, "All The Time" )
	PORT_DIPSETTING(    0x80, "Loop At 2 Times" )
	PORT_DIPSETTING(    0x40, "Loop At 4 Times" )
	PORT_DIPSETTING(    0x00, "No Sounds" )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BITX(0x40, IP_ACTIVE_LOW, IPT_SERVICE, DEF_STR( Service_Mode ), KEYCODE_F2, IP_JOY_NONE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START	// dips bank 2
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ))
	PORT_DIPSETTING(    0x10, DEF_STR(No))
	PORT_DIPSETTING(    0x00, DEF_STR(Yes))
	PORT_DIPNAME( 0x20, 0x20, "Slack Difficulty")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x00, "Title Demo")
	PORT_DIPSETTING(    0x40, "At 1 Loop")
	PORT_DIPSETTING(    0x00, "At Every Gamedemo")
	PORT_DIPNAME( 0x80, 0x00, "Gamedemo")
	PORT_DIPSETTING(    0x80, "4 Kinds")
	PORT_DIPSETTING(    0x00, "7 Kinds")
INPUT_PORTS_END

static MACHINE_INIT( bishi )
{
}

static void sound_irq_gen(int state)
{
	if (state)
		cpunum_set_input_line(0, MC68000_IRQ_1, ASSERT_LINE);
	else
		cpunum_set_input_line(0, MC68000_IRQ_1, CLEAR_LINE);
}

static struct YMZ280Binterface ymz280b_intf =
{
	1,
	{ 16934400 },
	{ REGION_SOUND1 },
	{ YM3012_VOL(100,MIXER_PAN_LEFT,100,MIXER_PAN_RIGHT) },
	{ sound_irq_gen }
};

static MACHINE_DRIVER_START( bishi )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(bishi_interrupt, 2)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(1200)

	MDRV_MACHINE_INIT(bishi)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_NEEDS_6BITS_PER_GUN | VIDEO_RGB_DIRECT | VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS | VIDEO_UPDATE_AFTER_VBLANK)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_VISIBLE_AREA(29, 29+288-1, 16, 16+224-1)

	MDRV_PALETTE_LENGTH(4096)

	MDRV_VIDEO_START(bishi)
	MDRV_VIDEO_UPDATE(bishi)

	/* sound hardware */
	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(YMZ280B, ymz280b_intf)
MACHINE_DRIVER_END

// ROM definitions

ROM_START( bishi )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_WORD_SWAP( "575jaa.a05", 0x000000, 0x80000, CRC(7d354567) SHA1(7fc11585693c91c0ef7a8e00df4f2f01b356210f) )
	ROM_LOAD16_WORD_SWAP( "575jaa.a06", 0x080000, 0x80000, CRC(9b2f7fbb) SHA1(26c828085c44a9c4d4e713e8fcc0bc8fc973d107) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "575_ja.a07", 0x000000, 0x080000, CRC(37bbf387) SHA1(dcf7b151b865d251f3122611b6339dd84eb1f990) )
	ROM_LOAD16_BYTE( "575_ja.a08", 0x000001, 0x080000, CRC(47ecd559) SHA1(7baac23557d40cccc21b93f181606563924244b0) )
	ROM_LOAD16_BYTE( "575_ja.a09", 0x100000, 0x080000, CRC(c1db6e68) SHA1(e951661e3b39a83db21aed484764e032adcf3c2a) )
	ROM_LOAD16_BYTE( "575_ja.a10", 0x100001, 0x080000, BAD_DUMP CRC(c8b145d6) SHA1(15cb3e4bebb999f1791fafa7a2ce3875a56991ff) )  // both halves identical (bad?)

	// dummy region (game has no sprites, but we want to use the GX mixer)
	ROM_REGION( 0x80000, REGION_GFX2, 0 )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 )
	ROM_LOAD( "575jaa.a01", 0x000000, 0x080000, CRC(e1e9f7b2) SHA1(4da93e384a6018d829cbb02cfde98fc3662c5267) )
	ROM_LOAD( "575jaa.a02", 0x080000, 0x080000, CRC(d228eb06) SHA1(075bd48242b5f590bfbfc45bc430578375fad70f) )
	ROM_LOAD( "575jaa.a03", 0x100000, 0x080000, CRC(9ec0321f) SHA1(03999dc415f556d0cd58e6358f826b97e85b477b) )
	ROM_LOAD( "575jaa.a04", 0x180000, 0x080000, CRC(0120967f) SHA1(14cc2b9269f46859d1de418c8d4c76a6bdb09d16) )
ROM_END

ROM_START( sbishi )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_WORD_SWAP( "675_ja.a05", 0x000000, 0x80000, CRC(28a09c01) SHA1(627f6c9b9e88434ff3198c778ae5c57d9cda82c5) )
	ROM_LOAD16_WORD_SWAP( "675_ja.a06", 0x080000, 0x80000, CRC(e4998b33) SHA1(3012f7661542b38b1a113c5c10e2729c6a37e709) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "675_ja.a07", 0x000000, 0x080000, CRC(6fe7c658) SHA1(a786a417053a5fc62f967bdd564e8d3bdc89f958) )
	ROM_LOAD16_BYTE( "675_ja.a08", 0x000001, 0x080000, CRC(c230afc9) SHA1(f23c64ed08e77960beb0f8db2605622a3887e5f8) )
	ROM_LOAD16_BYTE( "675_ja.a09", 0x100000, 0x080000, CRC(63fe85a5) SHA1(e5ef1f3fc634264260d5fc3a669646abf1601b23) )
	ROM_LOAD16_BYTE( "675_ja.a10", 0x100001, 0x080000, CRC(703ac462) SHA1(6dd05b2a78517a46b9ae8322c6b94bddbe91e848) )

	// dummy region (game has no sprites, but we want to use the GX mixer)
	ROM_REGION( 0x80000, REGION_GFX2, 0 )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 )
	ROM_LOAD( "675_ja.a01", 0x000000, 0x080000, CRC(67910b15) SHA1(6566e2344ebe9d61c584a1ab9ecbc8e7dd0a9a5b) )
	ROM_LOAD( "675_ja.a02", 0x080000, 0x080000, CRC(3313a7ae) SHA1(a49df87446a5b1bbf77fdf13a298ed486d7d7476) )
	ROM_LOAD( "675_ja.a03", 0x100000, 0x080000, CRC(ec977e6a) SHA1(9beb13e716d1694a64ce787fa3db4ba986a07d51) )
	ROM_LOAD( "675_ja.a04", 0x180000, 0x080000, CRC(1d1de34e) SHA1(1671216545cc0842cf8c128eaa0c612e6d91875c) )
ROM_END

ROM_START( sbishik )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_WORD_SWAP( "675kaa05.12e", 0x000000, 0x80000, CRC(23600e1d) SHA1(b3224c84e41e3077425a60232bb91775107f37a8) )
	ROM_LOAD16_WORD_SWAP( "675kaa06.15e", 0x080000, 0x80000, CRC(bd1091f5) SHA1(29872abc49fe8209d0f414ca40a34fc494ff9b96) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "675kaa07.14n", 0x000000, 0x080000, CRC(1177c1f8) SHA1(42c6f3c3a6bd0adb7d927386fd99f1497e5df30c) )
	ROM_LOAD16_BYTE( "675kaa08.17n", 0x000001, 0x080000, CRC(7117e9cd) SHA1(5a9b4b7427edcc10725d5936869927874fef6463) )
	ROM_LOAD16_BYTE( "675kaa09.19n", 0x100000, 0x080000, CRC(8d49c765) SHA1(7921f8f3671fbbc3d5ea529234268a1e23ea622c) )
	ROM_LOAD16_BYTE( "675kaa10.22n", 0x100001, 0x080000, CRC(c16acf32) SHA1(df3eeb5ab3bab8e707eaa79ffc500e1dc2332a82) )

	// dummy region (game has no sprites, but we want to use the GX mixer)
	ROM_REGION( 0x80000, REGION_GFX2, 0 )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 )
	ROM_LOAD( "675kaa01.2f", 0x000000, 0x080000, CRC(73ac6ae6) SHA1(37e4722647a13275c5f51d2bfa50df3e12ea1ebf) )
	ROM_LOAD( "675kaa02.4f", 0x080000, 0x080000, CRC(4c341e7c) SHA1(b944ea59d94f9ea5cea8ed8ad68da2a52c4bbfd7) )
	ROM_LOAD( "675kaa03.6f", 0x100000, 0x080000, CRC(83f91beb) SHA1(3af95f503f26fc88e75c786a9fef8a333c21d1d6) )
	ROM_LOAD( "675kaa04.8f", 0x180000, 0x080000, CRC(ebcbd813) SHA1(d67540d0ea303f09866f4a766e2d5162f05cd4ac) )
ROM_END

static DRIVER_INIT( bishi )
{
	state_save_register_INT32("bishi", 0, "control2", (INT32 *)&cur_control, 1);
}

GAMEX( 1996, bishi,     0,       bishi,     bishi,     bishi,      ROT0, "Konami", "Bishi Bashi Championship Mini Game Senshuken", GAME_IMPERFECT_GRAPHICS)
GAMEX( 1998, sbishi,    0,       bishi,     bishi,     bishi,      ROT0, "Konami", "Super Bishi Bashi Championship (Japan JAA)", GAME_IMPERFECT_GRAPHICS)
GAMEX( 1998, sbishik,  sbishi,   bishi,     bishi,     bishi,      ROT0, "Konami", "Super Bishi Bashi Championship (ver KAA)", GAME_IMPERFECT_GRAPHICS)
//gamezfan adds super Bishi Bashi Championship
#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()
