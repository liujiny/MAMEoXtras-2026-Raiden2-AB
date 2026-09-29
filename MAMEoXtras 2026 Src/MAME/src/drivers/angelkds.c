#pragma code_seg("C119")
#pragma data_seg("D119")
#pragma bss_seg("B119")
#pragma const_seg("K119")
#pragma comment(linker, "/merge:D119=119")
#pragma comment(linker, "/merge:C119=119")
#pragma comment(linker, "/merge:B119=119")
#pragma comment(linker, "/merge:K119=119")
/* Angel Kids / Space Position hardware driver

 driver by David Haywood
 with some help from Steph (DSWs, Inputs, other
 bits here and there)

2 Board System, Uses Boards X090-PC-A & X090-PC-B

Both games appear to be joint Sega / Nasco efforts
(although all I see in Angel Kids is 'Exa Planning'
 but I think that has something to do with Nasco   )

Space Position is encrypted, the main processor is
D317-0005 (NEC Z80 Custom), see machine/segacrpt.c
for details on this encryption scheme

*/

/* started 23/01/2002 */

/* notes / todo:

Unknown Reads / Writes
Whats the Prom for? nothing important?
Clock Speeds etc.
Is the level order correct?
the progress sprite on the side of the screen re-appears at the bottom when you get
to the top, but the wrap-around is needed for other things, actual game bug?

*/

/* readme's

------------------------------------------------------------------------

Angel Kids
833-6599-01
Sega 1988

Nasco X090-PC-A  (Sega 837-6600)

 SW1   SW2


 8255

 8255

       11429 6116 Z80   YM2203 YM2203


 11424 11425 11426 11427  -  -  -  -  5M5165 11428  Z80
                                                         4MHz

                                                         6MHz


Nasco X090-PC-B

                                  2016-55
11437  11445    2016-55  2016-55             U5
11436  11444
11435  11443
11434  11442
11433  11441                  2016-55    2016-55
11432  11440
11431  11439    11446         2016-55

                                             11148
                                             11147
   2016-55 2016-55 2016-55

                                               18.432MHz

11430  11438

------------------------------------------------------------------------

Space Position (JPN Ver.)
(c)1986 Sega / Nasco
X090-PC-A 171-5383
X090-PC-B 171-5384


CPU	:D317-0005 (NEC Z80 Custom)
Sound	:NEC D780C-1
	:YM2203C x 2
OSC	:4.000MHz 6.000MHz
	:18.432MHz


EPR10120.C1	prg
EPR10121.C2	 |
EPR10122.C3	 |
EPR10123.C4	 |
EPR10124.C5	 |
EPR10125.C10	/

EPR10126.D4	snd

EPR10127.06
EPR10128.07
EPR10129.08
EPR10130.14
EPR10131.15
EPR10132.16

EPR10133.17

EPR10134.18
EPR10135.19

63S081N.U5


--- Team Japump!!! ---
http://www.rainemu.com/japump/
http://japump.i.am/
Dumped by Chackn
02/25/2000

------------------------------------------------------------------------

*/


#include "driver.h"
#include "vidhrdw/generic.h"
#include "cpu/z80/z80.h"
#include "machine/segacrpt.h"

static READ8_HANDLER( angelkds_main_sound_r );
static WRITE8_HANDLER( angelkds_main_sound_w );
static READ8_HANDLER( angelkds_sub_sound_r );
static WRITE8_HANDLER( angelkds_sub_sound_w );

extern data8_t *angelkds_txvideoram, *angelkds_bgtopvideoram, *angelkds_bgbotvideoram;

WRITE8_HANDLER( angelkds_bgtopvideoram_w );
WRITE8_HANDLER( angelkds_bgbotvideoram_w );
WRITE8_HANDLER( angelkds_txvideoram_w );

WRITE8_HANDLER( angelkds_bgtopbank_write );
WRITE8_HANDLER( angelkds_bgtopscroll_write );
WRITE8_HANDLER( angelkds_bgbotbank_write );
WRITE8_HANDLER( angelkds_bgbotscroll_write );
WRITE8_HANDLER( angelkds_txbank_write );

WRITE8_HANDLER( angelkds_paletteram_w );
WRITE8_HANDLER( angelkds_layer_ctrl_write );

VIDEO_START( angelkds );
VIDEO_UPDATE( angelkds );

/*** CPU Banking

*/

static WRITE8_HANDLER ( angelkds_cpu_bank_write )
{
	int bankaddress;
	unsigned char *RAM = memory_region(REGION_USER1);

	bankaddress = data & 0x0f;
	cpu_setbank(1,&RAM[bankaddress*0x4000]);
}


/*** Fake Inputs

these make the game a bit easier for testing purposes

*/

#define FAKEINPUTS 0

#if FAKEINPUTS

static READ8_HANDLER( angelkds_input_r )
{
	int fake = readinputport(6+offset);

	return ((fake & 0x01) ? fake  : readinputport(4+offset));
}

#else

static READ8_HANDLER( angelkds_input_r )
{
	return readinputport(4+offset);
}

#endif

/*** Memory Structures

Angel Kids:
I would have expected f003 to be the scroll register for the bottom
part of the screen, in the attract mode this works fine, but in the
game it doesn't, so maybe it wasn't really hooked up and instead
only one of the register (f001) is used for both part?

* update, it is correct, the screen is meant to split in two when
 the kid goes what would be offscreen, just looked kinda odd

Interesting note, each Bank in the 0x8000 - 0xbfff appears to
contain a level.



*/

static ADDRESS_MAP_START( readmem_main, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_BANK1)
	AM_RANGE(0xc000, 0xdfff) AM_READ(MRA8_RAM)
	AM_RANGE(0xe000, 0xefff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem_main, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xe000, 0xe3ff) AM_WRITE(angelkds_bgtopvideoram_w) AM_BASE(&angelkds_bgtopvideoram) /* Top Half of Screen */
	AM_RANGE(0xe400, 0xe7ff) AM_WRITE(angelkds_bgbotvideoram_w) AM_BASE(&angelkds_bgbotvideoram) /* Bottom Half of Screen */
	AM_RANGE(0xe800, 0xebff) AM_WRITE(angelkds_txvideoram_w) AM_BASE(&angelkds_txvideoram)
	AM_RANGE(0xec00, 0xecff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram)
	AM_RANGE(0xed00, 0xeeff) AM_WRITE(angelkds_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(angelkds_bgtopbank_write)
	AM_RANGE(0xf001, 0xf001) AM_WRITE(angelkds_bgtopscroll_write)
	AM_RANGE(0xf002, 0xf002) AM_WRITE(angelkds_bgbotbank_write)
	AM_RANGE(0xf003, 0xf003) AM_WRITE(angelkds_bgbotscroll_write)
	AM_RANGE(0xf004, 0xf004) AM_WRITE(angelkds_txbank_write)
	AM_RANGE(0xf005, 0xf005) AM_WRITE(angelkds_layer_ctrl_write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( readport_main, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x40, 0x40) AM_READ(input_port_0_r)	/* "Coinage" Dip Switches */
	AM_RANGE(0x41, 0x41) AM_READ(input_port_1_r)	/* Other Dip Switches */
	AM_RANGE(0x42, 0x42) AM_READ(input_port_2_r)	/* Players inputs (not needed ?) */
	AM_RANGE(0x80, 0x80) AM_READ(input_port_3_r)	/* System inputs */
	AM_RANGE(0x81, 0x82) AM_READ(angelkds_input_r)	/* Players inputs */
    AM_RANGE(0xc0, 0xc3) AM_READ(angelkds_main_sound_r) /* needed by spcpostn */
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport_main, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_WRITE(MWA8_NOP) // 00 on start-up, not again
	AM_RANGE(0x42, 0x42) AM_WRITE(angelkds_cpu_bank_write)
	AM_RANGE(0x43, 0x43) AM_WRITE(MWA8_NOP) // 9a on start-up, not again
	AM_RANGE(0x83, 0x83) AM_WRITE(MWA8_NOP) // 9b on start-up, not again
	AM_RANGE(0xc0, 0xc3) AM_WRITE(angelkds_main_sound_w) // 02 various points
ADDRESS_MAP_END

/* sub cpu */

static ADDRESS_MAP_START( readmem_sub, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_READ(MRA8_RAM)
	AM_RANGE(0xaaa9, 0xaaa9) AM_READ(MRA8_NOP)
	AM_RANGE(0xaaab, 0xaaab) AM_READ(MRA8_NOP)
	AM_RANGE(0xaaac, 0xaaac) AM_READ(MRA8_NOP)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem_sub, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( readport_sub, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_READ(YM2203_status_port_0_r)
	AM_RANGE(0x40, 0x40) AM_READ(YM2203_status_port_1_r)
    AM_RANGE(0x80, 0x83) AM_READ(angelkds_sub_sound_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( writeport_sub, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_WRITE(YM2203_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(YM2203_write_port_0_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(YM2203_control_port_1_w)
	AM_RANGE(0x41, 0x41) AM_WRITE(YM2203_write_port_1_w)
    AM_RANGE(0x80, 0x83) AM_WRITE(angelkds_sub_sound_w) /* spcpostn */
ADDRESS_MAP_END


/* Input Ports */

/* Here is the way to access to the different parts of the "test mode" :

     - sound  : set "Coin A" Dip Switch to "Free Play" and "Coin B" Dip Switch to "Free Play"
     - paddle : set "Coin A" Dip Switch to "3C_1C" and "Coin B" Dip Switch to "Free Play"

If use different settings, you'll only see a black screen.

I haven't found how to exit the tests. The only way seems to reset the game.
*/

#define ANGELDSK_PLAYERS_INPUT( player ) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP    | IPF_8WAY | player ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN  | IPF_8WAY | player ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT  | IPF_8WAY | player ) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT | IPF_8WAY | player ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP     | IPF_8WAY | player ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN   | IPF_8WAY | player ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT   | IPF_8WAY | player ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT  | IPF_8WAY | player )

#define ANGELDSK_FAKE_PLAYERS_INPUT( player ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | player )	/* To enter initials */ \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )		/* Unused */ \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | player ) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_8WAY | player ) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_8WAY | player ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_8WAY | player ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 | player )	/* To shorten the rope and */ \
										/* move right in hiscores table */


INPUT_PORTS_START( angelkds )
	PORT_START		/* inport $40 */
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x90, DEF_STR( 2C_1C ) )
//	PORT_DIPSETTING(	0x60, DEF_STR( 2C_1C ) )
//	PORT_DIPSETTING(	0x50, DEF_STR( 2C_1C ) )
//	PORT_DIPSETTING(	0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0xf0, DEF_STR( 1C_1C ) )
//	PORT_DIPSETTING(	0x30, DEF_STR( 1C_1C ) )
//	PORT_DIPSETTING(	0x20, DEF_STR( 1C_1C ) )
//	PORT_DIPSETTING(	0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )	// needed to enter "test mode" (see above)

	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(	0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x09, DEF_STR( 2C_1C ) )
//	PORT_DIPSETTING(	0x06, DEF_STR( 2C_1C ) )
//	PORT_DIPSETTING(	0x05, DEF_STR( 2C_1C ) )
//	PORT_DIPSETTING(	0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x0f, DEF_STR( 1C_1C ) )
//	PORT_DIPSETTING(	0x03, DEF_STR( 1C_1C ) )
//	PORT_DIPSETTING(	0x02, DEF_STR( 1C_1C ) )
//	PORT_DIPSETTING(	0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )

	PORT_START		/* inport $41 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, "Hi Score" )
	PORT_DIPSETTING(	0x00, "3 Characters" )
	PORT_DIPSETTING(	0x02, "10 Characters" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(	0x0c, "20k, 50k, 100k, 200k and 500k" )
	PORT_DIPSETTING(	0x08, "50k, 100k, 200k and 500k" )
	PORT_DIPSETTING(	0x04, "100k, 200k and 500k" )
	PORT_DIPSETTING(	0x00, "None" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x30, "3" )
	PORT_DIPSETTING(	0x20, "4" )
	PORT_DIPSETTING(	0x10, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "99", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Unknown ) )	// Stored at 0xc023
	PORT_DIPSETTING(	0xc0, "4" )
	PORT_DIPSETTING(	0x40, "5" )
	PORT_DIPSETTING(	0x80, "6" )
	PORT_DIPSETTING(	0x00, "7" )

	PORT_START		/* inport $42 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	// duplicated IPT_JOYSTICK_LEFTRIGHT  | IPF_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// duplicated IPT_JOYSTICK_LEFTRIGHT  | IPF_8WAY | IPF_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START		/* inport $80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START		/* inport $81 */
	ANGELDSK_PLAYERS_INPUT( IPF_PLAYER1 )

	PORT_START		/* inport $82 */
	ANGELDSK_PLAYERS_INPUT( IPF_PLAYER2 )

#if FAKEINPUTS

	/* Fake inputs to allow to play the game with 1 joystick instead of 2 */
	PORT_START
	PORT_DIPNAME( 0x01, 0x00, "FAKE (for debug) Joysticks (Player 1)" )
	PORT_DIPSETTING(	0x01, "1" )
	PORT_DIPSETTING(	0x00, "2" )
	ANGELDSK_FAKE_PLAYERS_INPUT( IPF_PLAYER1 )

	PORT_START
	PORT_DIPNAME( 0x01, 0x00, "FAKE (for debug) Joysticks (Player 2)" )
	PORT_DIPSETTING(	0x01, "1" )
	PORT_DIPSETTING(	0x00, "2" )
	ANGELDSK_FAKE_PLAYERS_INPUT( IPF_PLAYER2 )

#endif

INPUT_PORTS_END

INPUT_PORTS_START( spcpostn )
	PORT_START		/* inport $40 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )

	PORT_START		/* inport $41 */
	PORT_DIPNAME( 0x01, 0x01, "Allow_Continue" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Obstruction Car" )
	PORT_DIPSETTING(    0x02, "Normal" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x0c, 0x08, "Time Limit" )
	PORT_DIPSETTING(    0x00, "1:10" )
	PORT_DIPSETTING(    0x04, "1:20" )
	PORT_DIPSETTING(    0x08, "1:30" )
	PORT_DIPSETTING(    0x0c, "1:40" )
	PORT_DIPNAME( 0x30, 0x20, "Power Down" )
	PORT_DIPSETTING(    0x30, "Slow" )
	PORT_DIPSETTING(    0x20, "Normal" )
	PORT_DIPSETTING(    0x10, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START		/* inport $42 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START		/* inport $80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START		/* inport $81 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED | IPF_PLAYER1 ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED | IPF_PLAYER1 ) /* probably unused */

	PORT_START		/* inport $82 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED | IPF_PLAYER2 ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED | IPF_PLAYER2 ) /* probably unused */

INPUT_PORTS_END

/*** Sound Hardware

todo: verify / correct things
seems a bit strange are all the addresses really
sound related ?

*/

static UINT8 angelkds_sound[4];
static UINT8 angelkds_sound2[4];

static WRITE8_HANDLER( angelkds_main_sound_w )
{
	angelkds_sound[offset]=data;
}

static READ8_HANDLER( angelkds_main_sound_r )
{
	return angelkds_sound2[offset];
}

static WRITE8_HANDLER( angelkds_sub_sound_w )
{
	angelkds_sound2[offset]=data;
}

static READ8_HANDLER( angelkds_sub_sound_r )
{
	return angelkds_sound[offset];
}

static void irqhandler(int irq)
{
	cpunum_set_input_line(1,0,irq ? ASSERT_LINE : CLEAR_LINE);
}

static struct YM2203interface ym2203_interface =
{
	2,                      /* 2 chips */
	4000000,        /* 4 MHz ? */
	{ YM2203_VOL(15,25), YM2203_VOL(15,25) },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ irqhandler }
};

/*** Graphics Decoding

all the 8x8 tiles are in one format, the 16x16 sprites in another

*/

static struct GfxLayout angelkds_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};


static struct GfxLayout angelkds_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0,4,	RGN_FRAC(1,2)+0,	RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11, 16,17,18,19, 24,25,26,27,  },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32  },
	16*32
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &angelkds_charlayout,   0x30, 1  },
	{ REGION_GFX3, 0, &angelkds_charlayout,   0, 16 },
	{ REGION_GFX4, 0, &angelkds_charlayout,   0, 16 },
	{ REGION_GFX2, 0, &angelkds_spritelayout, 0x20, 0x0d },
	{ -1 } /* end of array */
};

/*** Machine Driver

 2 x z80 (one for game, one for sound)
 2 x YM2203 (for sound)

 all fairly straightforward

*/

static MACHINE_DRIVER_START( angelkds )
	MDRV_CPU_ADD(Z80, 8000000) /* 8MHz? 6 seems too slow? */
	MDRV_CPU_PROGRAM_MAP(readmem_main,writemem_main)
	MDRV_CPU_IO_MAP(readport_main,writeport_main)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80, 4000000) /* 8 MHz? */
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
	MDRV_CPU_PROGRAM_MAP(readmem_sub,writemem_sub)
	MDRV_CPU_IO_MAP(readport_sub,writeport_sub)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(100)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(angelkds)
	MDRV_VIDEO_UPDATE(angelkds)

	MDRV_SOUND_ADD(YM2203, ym2203_interface)
MACHINE_DRIVER_END

/*** Rom Loading

 REGION_CPU1 for the main code
 REGION_USER1 for the banked data
 REGION_CPU2 for the sound cpu code
 REGION_GFX1 for the 8x8 Txt Layer Tiles
 REGION_GFX2 for the 16x16 Sprites
 REGION_GFX3 for the 8x8 Bg Layer Tiles (top tilemap)
 REGION_GFX4 for the 8x8 Bg Layer Tiles (bottom tilemap)
 REGION_PROMS for the Prom (same between games)

*/

ROM_START( angelkds )
	/* Nasco X090-PC-A  (Sega 837-6600) */
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "11428.c10",    0x00000, 0x08000, CRC(90daacd2) SHA1(7e50ad1cbed0c1e6bad04ef1611cad25538c905f) )

	ROM_REGION( 0x20000, REGION_USER1, 0 ) /* Banked Code */
	ROM_LOAD( "11424.c1",     0x00000, 0x08000, CRC(b55997f6) SHA1(7ed746becac1851f39591f1fdbeff64aa97d6206) )
	ROM_LOAD( "11425.c2",     0x08000, 0x08000, CRC(299359de) SHA1(f531dd3bfe6f64e9e043cb4f85d5657455241dc7) )
	ROM_LOAD( "11426.c3",     0x10000, 0x08000, CRC(5fad8bd3) SHA1(4d865342eb10dcfb779eee4ac1e159bb9ec140cb) )
	ROM_LOAD( "11427.c4",     0x18000, 0x08000, CRC(ef920c74) SHA1(81c0fbe4ace5441e4cd99ba423e0190cc541da31) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "11429.d4",     0x00000, 0x08000, CRC(0ca50a66) SHA1(cccb081b447419138b1ebd309e7f291e392a44d5) )

	/* Nasco X090-PC-B */
	ROM_REGION( 0x08000, REGION_GFX1, 0 )
	ROM_LOAD( "11446",        0x00000, 0x08000, CRC(45052470) SHA1(c2312a9f814d6dbe42aa465147a04a2bd9b2aa1b) )

	ROM_REGION( 0x10000, REGION_GFX2, 0 )
	ROM_LOAD( "11447.f7",     0x08000, 0x08000, CRC(b3afc5b3) SHA1(376d527f60e9044f18d19a5535bca77606efbd4c) )
	ROM_LOAD( "11448.h7",     0x00000, 0x08000, CRC(05dab626) SHA1(73feaca6e23c673a7d8c9e972714b20bd8f2d51e) )

	/* both tilemaps on angelkds use the same gfx */
	ROM_REGION( 0x40000, REGION_GFX3, 0 )
	ROM_LOAD( "11437",        0x00000, 0x08000, CRC(a520b628) SHA1(2b51f59e760e740e5e6b06dad61bbc23fc84a72b) )
	ROM_LOAD( "11436",        0x08000, 0x08000, CRC(469ab216) SHA1(8223f072a6f9135ff84841c95410368bcea073d8) )
	ROM_LOAD( "11435",        0x10000, 0x08000, CRC(b0f8c245) SHA1(882e27eaceac46c397fdae8427a082caa7d6b7dc) )
	ROM_LOAD( "11434",        0x18000, 0x08000, CRC(cbde81f5) SHA1(5d5b8e709c9dd09a45dfced6f3d4a9c52500da6b) )
	ROM_LOAD( "11433",        0x20000, 0x08000, CRC(b63fa414) SHA1(25adcafd7e17ab0be0fed2ec44245124febd74b3) )
	ROM_LOAD( "11432",        0x28000, 0x08000, CRC(00dc747b) SHA1(041b73aa48b45162af33b5f416ccc0c0dbbd995b) )
	ROM_LOAD( "11431",        0x30000, 0x08000, CRC(ac2025af) SHA1(2aba145df3ccdb1a7f0fec524bd2de3f9aab4161) )
	ROM_LOAD( "11430",        0x38000, 0x08000, CRC(d640f89e) SHA1(38fb67bcb2a3d1ad614fc62e42f22a66bc757137) )

	ROM_REGION( 0x40000, REGION_GFX4, 0 )
	ROM_LOAD( "11445",        0x00000, 0x08000, CRC(a520b628) SHA1(2b51f59e760e740e5e6b06dad61bbc23fc84a72b) )
	ROM_LOAD( "11444",        0x08000, 0x08000, CRC(469ab216) SHA1(8223f072a6f9135ff84841c95410368bcea073d8) )
	ROM_LOAD( "11443",        0x10000, 0x08000, CRC(b0f8c245) SHA1(882e27eaceac46c397fdae8427a082caa7d6b7dc) )
	ROM_LOAD( "11442",        0x18000, 0x08000, CRC(cbde81f5) SHA1(5d5b8e709c9dd09a45dfced6f3d4a9c52500da6b) )
	ROM_LOAD( "11441",        0x20000, 0x08000, CRC(b63fa414) SHA1(25adcafd7e17ab0be0fed2ec44245124febd74b3) )
	ROM_LOAD( "11440",        0x28000, 0x08000, CRC(00dc747b) SHA1(041b73aa48b45162af33b5f416ccc0c0dbbd995b) )
	ROM_LOAD( "11439",        0x30000, 0x08000, CRC(ac2025af) SHA1(2aba145df3ccdb1a7f0fec524bd2de3f9aab4161) )
	ROM_LOAD( "11438",        0x38000, 0x08000, CRC(d640f89e) SHA1(38fb67bcb2a3d1ad614fc62e42f22a66bc757137) )

	ROM_REGION( 0x20, REGION_PROMS, 0 )
	ROM_LOAD( "63s081n.u5",	  0x00,    0x20,    CRC(36b98627) SHA1(d2d54d92d1d47e7cc85104989ee421ce5d80a42a) )
ROM_END

ROM_START( spcpostn )
	/* X090-PC-A 171-5383 */
	ROM_REGION( 2*0x10000, REGION_CPU1, 0 ) /* D317-0005 (NEC Z80 Custom) */
	ROM_LOAD( "epr10125.c10", 0x00000, 0x08000, CRC(bffd38c6) SHA1(af02907124343ddecd21439d25f1ebb81ef9f51a) ) /* encrypted */

	ROM_REGION( 0x28000, REGION_USER1, 0 ) /* Banked Code */
	ROM_LOAD( "epr10120.c1",  0x00000, 0x08000, CRC(d6399f99) SHA1(4c7d19a8798e5a10b688bf793ca74f5170fd9b51) )
	ROM_LOAD( "epr10121.c2",  0x08000, 0x08000, CRC(d4861560) SHA1(74d28c36a08880abbd3c398cc3e990e8986caccb) )
	ROM_LOAD( "epr10122.c3",  0x10000, 0x08000, CRC(7a1bff1b) SHA1(e1bda8430fd632c1813dd78e0f210a358e1b0d2f) )
	ROM_LOAD( "epr10123.c4",  0x18000, 0x08000, CRC(6aed2925) SHA1(75848c8086c460b72494da2367f592d7d5dcf9f1) )
	ROM_LOAD( "epr10124.c5",  0x20000, 0x08000, CRC(a1d7ae6b) SHA1(ec81fecf63e0515cae2077e2623262227adfdf37) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* NEC D780C-1 */
	ROM_LOAD( "epr10126.d4",  0x00000, 0x08000, CRC(ab17f852) SHA1(dc0db427ddb4df97bb40dfb6fc65cb9354a6b9ad) )

	/* X090-PC-B 171-5384 */
	ROM_REGION( 0x08000, REGION_GFX1, 0 )
	ROM_LOAD( "epr10133.17",  0x00000, 0x08000, CRC(642e6609) SHA1(2dfb4cc66f89543b55ed2a5b914e2c9304e821ca) )

	ROM_REGION( 0x10000, REGION_GFX2, 0 )
	ROM_LOAD( "epr10134.18",  0x08000, 0x08000, CRC(c674ff88) SHA1(9f240910a1ffb7c9e09d2326de280e6a5dd84565) )
	ROM_LOAD( "epr10135.19",  0x00000, 0x08000, CRC(0685c4fa) SHA1(6950d9ad9ec13236cf24e83e87adb62aa53af7bb) )

	ROM_REGION( 0x30000, REGION_GFX3, 0 )
	ROM_LOAD( "epr10130.14",  0x10000, 0x08000, CRC(b68fcb36) SHA1(3943dd550b13f2911d56d8dad675410da79196e6) )
	ROM_LOAD( "epr10131.15",  0x08000, 0x08000, CRC(de223817) SHA1(1860db0a19c926fcfaabe676cb57fff38c4df8e6) )
	ROM_LOAD( "epr10132.16",  0x00000, 0x08000, CRC(2df8b1bd) SHA1(cad8befa3f2c158d2aa74073066ccd2b54e68825) )

	ROM_REGION( 0x18000, REGION_GFX4, 0 )
	ROM_LOAD( "epr10127.06",  0x10000, 0x08000, CRC(b68fcb36) SHA1(3943dd550b13f2911d56d8dad675410da79196e6) )
	ROM_LOAD( "epr10128.07",  0x08000, 0x08000, CRC(de223817) SHA1(1860db0a19c926fcfaabe676cb57fff38c4df8e6) )
	ROM_LOAD( "epr10129.08",  0x00000, 0x08000, CRC(a6f21023) SHA1(8d573446a2d3d3428409707d0c59b118d1463131) )

	ROM_REGION( 0x20, REGION_PROMS, 0 )
	ROM_LOAD( "63s081n.u5",   0x00,    0x20,    CRC(36b98627) SHA1(d2d54d92d1d47e7cc85104989ee421ce5d80a42a) )
ROM_END


static DRIVER_INIT( spcpostn )	{ spcpostn_decode(); }


GAME(1988, angelkds, 0, angelkds, angelkds,        0,  ROT90,  "Sega / Nasco?", "Angel Kids (Japan)" ) /* Nasco not displayed but 'Exa Planning' is */
GAME(1986, spcpostn, 0, angelkds, spcpostn, spcpostn,  ROT90,  "Sega / Nasco", "Space Position (Japan)" ) /* encrypted */

/*

Calorie-Kun
Sega, 1986

PCB Layout
----------

Top PCB


837-6077  171-5381
|--------------------------------------------|
|                    Z80             YM2149  |
|                                            |
|                    2016            YM2149  |
|                                            |
|10079    10077      10075           YM2149  |
|    10078    10076                          |
|                                           -|
|                                          |
|                    2114                   -|
|                                           1|
|                    2114                   8|
|                                           W|
|                    2114                   A|
|                                           Y|
|                                           -|
|                                          |
|                                    DSW2(8)-|
|           10082   10081   10080            |
|2016                                DSW1(8) |
|--------------------------------------------|

Notes:
      Z80 clock   : 3.000MHz
      YM2149 clock: 1.500MHz
      VSync       : 60Hz
      2016        : 2K x8 SRAM
      2114        : 1K x4 SRAM



Bottom PCB


837-6076  171-5380
|--------------------------------------------|
|                            12MHz           |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
| 10074                               10071  |
|               NEC                          |
| 10073      D317-0004   4MHz         10070  |
|                                            |
| 10072                               10069  |
|                                            |
| 2016         2114      6148                |
|                                            |
| 2016         2114      6148                |
|                                            |
|                        6148                |
|                        6148                |
|--------------------------------------------|
Notes:
      317-0004 clock: 4.000MHz
      2016          : 2K x8 SRAM
      2114          : 1K x4 SRAM
      6148          : 1K x4 SRAM


 driver by David Haywood and Pierpaolo Prazzoli

*/

static data8_t *calorie_fg;
static data8_t  calorie_bg;
static data8_t *calorie_sprites;

static struct tilemap *bg_tilemap,*fg_tilemap;

static void get_bg_tile_info(int tile_index)
{
	data8_t *src = memory_region(REGION_USER1);
	int bg_base = (calorie_bg & 0x0f) * 0x200;
	int code  = src[bg_base + tile_index] | (((src[bg_base + tile_index + 0x100]) & 0x10) << 4);
	int color = src[bg_base + tile_index + 0x100] & 0x0f;
	int flag  = src[bg_base + tile_index + 0x100] & 0x40 ? TILE_FLIPX : 0;

	SET_TILE_INFO(1, code, color, flag)
}

static void get_fg_tile_info(int tile_index)
{
	int code  = ((calorie_fg[tile_index + 0x400] & 0x30) << 4) | calorie_fg[tile_index];
	int color = calorie_fg[tile_index + 0x400] & 0x0f;

	SET_TILE_INFO(0, code, color, TILE_FLIPYX((calorie_fg[tile_index + 0x400] & 0xc0) >> 6))
}


VIDEO_START( calorie )
{
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_OPAQUE,     16,16,16,16);
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TRANSPARENT, 8, 8,32,32);

	if(!bg_tilemap || !fg_tilemap )
		return 1;

	tilemap_set_transparent_pen(fg_tilemap,0);

	return 0;
}

VIDEO_UPDATE( calorie )
{
	int x;

	if (calorie_bg & 0x10)
	{
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	}
	else
	{
		tilemap_draw(bitmap,cliprect,fg_tilemap,TILEMAP_IGNORE_TRANSPARENCY,0);
	}


	for (x = 0x400; x >= 0; x -= 4)
	{
		int xpos, ypos, tileno, color, flipx, flipy;

		tileno = calorie_sprites[x+0];
		color = calorie_sprites[x+1] & 0x0f;
		flipx = calorie_sprites[x+1] & 0x40;
		flipy = 0;
		ypos = 0xff - calorie_sprites[x+2];
		xpos = calorie_sprites[x+3];

		if(flip_screen)
		{
			if( calorie_sprites[x+1] & 0x10 )
				ypos = 0xff - ypos + 32;
			else
				ypos = 0xff - ypos + 16;

			xpos = 0xff - xpos - 16;
			flipx = !flipx;
			flipy = !flipy;
		}

		if( calorie_sprites[x+1] & 0x10 )
		{
			 /* 32x32 sprites */
			drawgfx(bitmap,Machine->gfx[3],tileno | 0x40,color,flipx,flipy,xpos,ypos - 31,cliprect,TRANSPARENCY_PEN,0);
		}
		else
		{
			/* 16x16 sprites */
			drawgfx(bitmap,Machine->gfx[2],tileno,color,flipx,flipy,xpos,ypos - 15,cliprect,TRANSPARENCY_PEN,0);
		}
	}
}

static WRITE8_HANDLER( calorie_fg_w )
{
	calorie_fg[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
}

static WRITE8_HANDLER( calorie_bg_w )
{
	if((calorie_bg & ~0x10) != (data & ~0x10))
	{
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	calorie_bg = data;
}

static WRITE8_HANDLER( calorie_flipscreen_w )
{
	flip_screen_set(data & 1);
}

static READ8_HANDLER( calorie_soundlatch_r )
{
	data8_t latch = soundlatch_r(0);
	soundlatch_clear_w(0,0);
	return latch;
}

static WRITE8_HANDLER( bogus_w )
{
	usrintf_showmessage("written to 3rd sound chip: data = %02X port = %02X", data, offset);
}

static ADDRESS_MAP_START( calorie_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_WRITE(calorie_fg_w) AM_BASE(&calorie_fg)
	AM_RANGE(0xd800, 0xdbff) AM_RAM AM_BASE(&calorie_sprites)
	AM_RANGE(0xdc00, 0xdcff) AM_RAM AM_WRITE(paletteram_xxxxBBBBGGGGRRRR_w) AM_BASE(&paletteram)
	AM_RANGE(0xde00, 0xde00) AM_WRITE(calorie_bg_w)
	AM_RANGE(0xf000, 0xf000) AM_READ(input_port_0_r)
	AM_RANGE(0xf001, 0xf001) AM_READ(input_port_1_r)
	AM_RANGE(0xf002, 0xf002) AM_READ(input_port_2_r)
	AM_RANGE(0xf004, 0xf004) AM_READ(input_port_3_r) AM_WRITE(calorie_flipscreen_w)
	AM_RANGE(0xf005, 0xf005) AM_READ(input_port_4_r)
	AM_RANGE(0xf800, 0xf800) AM_WRITE(soundlatch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( calorie_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xc000, 0xc000) AM_READ(calorie_soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( calorie_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x00) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x11, 0x11) AM_READWRITE(AY8910_read_port_1_r, AY8910_write_port_1_w)
	// 3rd ?
	AM_RANGE(0x00, 0xff) AM_WRITE(bogus_w)
ADDRESS_MAP_END


INPUT_PORTS_START( calorie )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) ) 
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPSETTING(    0x01, "20,000 Only" )
	PORT_DIPSETTING(    0x03, "20,000 and 60,000" ) /* No listed value for 0x02 (Off / On) */
	PORT_DIPNAME( 0x04, 0x00, "Number of Bombs" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, "Difficulty - Mogura Nian" )
	PORT_DIPSETTING(    0x00, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, "Difficulty - Select of Mogura" )
	PORT_DIPSETTING(    0x00, "Easy" )
	PORT_DIPSETTING(    0x20, "Normal" )
	PORT_DIPSETTING(    0x10, "Hard" )
	PORT_DIPSETTING(    0x30, "Hardest" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
    PORT_DIPNAME( 0x80, 0x00, "Infinite Lives" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static struct AY8910interface ay8910_interface =
{
	3,	/* 3 chips */
	1500000,	/* 1.5 MHz */
	{ 80, 80, 80 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 }
};

static struct GfxLayout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1,2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static struct GfxLayout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1,2, 3, 4, 5, 6, 7, 8*8+0,8*8+1,8*8+2,8*8+3,8*8+4,8*8+5,8*8+6,8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	8*32
};

static struct GfxLayout tiles32x32_layout =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
			40*8+0, 40*8+1, 40*8+2, 40*8+3, 40*8+4, 40*8+5, 40*8+6, 40*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			80*8, 81*8, 82*8, 83*8, 84*8, 85*8, 86*8, 87*8 },
	128*8
};

static struct GfxDecodeInfo calorie_gfxdecodeinfo[] =
{
	{ REGION_GFX2, 0, &tiles8x8_layout,   0, 16 },
	{ REGION_GFX3, 0, &tiles16x16_layout, 0, 16 },
	{ REGION_GFX1, 0, &tiles16x16_layout, 0, 16 },
	{ REGION_GFX1, 0, &tiles32x32_layout, 0, 16 },
	{ -1 }
};

static MACHINE_DRIVER_START( calorie )
	/* basic machine hardware */
	MDRV_CPU_ADD(Z80,4000000)		 /* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(calorie_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_pulse,1)

	MDRV_CPU_ADD(Z80,3000000)		 /* 3 MHz */
	MDRV_CPU_PROGRAM_MAP(calorie_sound_map,0)
	MDRV_CPU_IO_MAP(calorie_sound_io_map,0)
	MDRV_CPU_PERIODIC_INT(irq0_line_pulse,64)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MDRV_GFXDECODE(calorie_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(calorie)
	MDRV_VIDEO_UPDATE(calorie)

	/* sound hardware */
	MDRV_SOUND_ADD(AY8910, ay8910_interface)
MACHINE_DRIVER_END


ROM_START( calorie )
	ROM_REGION( 2*0x20000, REGION_CPU1, 0 )
	ROM_LOAD( "epr10072.1j", 0x00000, 0x4000, CRC(ade792c1) SHA1(6ea5afb00a87037d502c17adda7e4060d12680d7) )
	ROM_LOAD( "epr10073.1k", 0x04000, 0x4000, CRC(b53e109f) SHA1(a41c5affe917232e7adf40d7c15cff778b197e90) )
	ROM_LOAD( "epr10074.1m", 0x08000, 0x4000, CRC(a08da685) SHA1(69f9cfebc771312dbb1726350c2d9e9e8c46353f) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "epr10075.4d", 0x0000, 0x4000, CRC(ca547036) SHA1(584a65482f2b92a4c08c37560450d6db68a56c7b) )

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* background tilemaps */
	ROM_LOAD( "epr10079.8d", 0x0000, 0x2000, CRC(3c61a42c) SHA1(68ea6b5d2f3c6a9e5308c08dde20424f20021a73) )

	ROM_REGION( 0xc000, REGION_GFX1, ROMREGION_DISPOSE ) /* sprites */
	ROM_LOAD( "epr10071.7m", 0x0000, 0x4000, CRC(5f55527a) SHA1(ec1ba8f95ac47a0c783e117ef4af6fe0ab5925b5) )
	ROM_LOAD( "epr10070.7k", 0x4000, 0x4000, CRC(97f35a23) SHA1(869553a334e1b3ba900a8b9c9eaf25fbc6ab31dd) )
	ROM_LOAD( "epr10069.7j", 0x8000, 0x4000, CRC(c0c3deaf) SHA1(8bf2e2146b794a330a079dd080f0586500964b1a) )

	ROM_REGION( 0x6000, REGION_GFX2, ROMREGION_DISPOSE ) /* tiles  */
	ROM_LOAD( "epr10082.5r", 0x0000, 0x2000, CRC(5984ea44) SHA1(010011b5b8dfa593c6fc7d2366f8cf82fcc8c978) )
	ROM_LOAD( "epr10081.4r", 0x2000, 0x2000, CRC(e2d45dd8) SHA1(5e11089680b574ea4cbf64510e51b0a945f79174) )
	ROM_LOAD( "epr10080.3r", 0x4000, 0x2000, CRC(42edfcfe) SHA1(feba7b1daffcad24d4c24f55ab5466f8cebf31ad) )

	ROM_REGION( 0xc000, REGION_GFX3, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "epr10078.7d", 0x0000, 0x4000, CRC(5b8eecce) SHA1(e7eee82081939b361edcbb9587b072b4b9a162f9) )
	ROM_LOAD( "epr10077.6d", 0x4000, 0x4000, CRC(01bcb609) SHA1(5d01fa75f214d34483284aaaef985ab92a606505) )
	ROM_LOAD( "epr10076.5d", 0x8000, 0x4000, CRC(b1529782) SHA1(8e0e92aae4c8dd8720414372aa767054cc316a0f) )
ROM_END

ROM_START( calorieb )
	ROM_REGION( 0x20000, REGION_CPU1, 0 )
	ROM_LOAD( "12.bin",      0x10000, 0x4000, CRC(cf5fa69e) SHA1(520d5652e93a672a1fc147295fbd63b873967885) )
	ROM_CONTINUE(            0x00000, 0x4000 )
	ROM_LOAD( "13.bin",      0x14000, 0x4000, CRC(52e7263f) SHA1(4d684c9e3f08ddb18b0b3b982aba82d3c809a633) )
	ROM_CONTINUE(            0x04000, 0x4000 )
	ROM_LOAD( "epr10074.1m", 0x18000, 0x4000, CRC(a08da685) SHA1(69f9cfebc771312dbb1726350c2d9e9e8c46353f) )
	ROM_RELOAD(              0x08000, 0x4000 )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "epr10075.4d", 0x0000, 0x4000, CRC(ca547036) SHA1(584a65482f2b92a4c08c37560450d6db68a56c7b) )

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* background tilemaps */
	ROM_LOAD( "epr10079.8d", 0x0000, 0x2000, CRC(3c61a42c) SHA1(68ea6b5d2f3c6a9e5308c08dde20424f20021a73) )

	ROM_REGION( 0xc000, REGION_GFX1, ROMREGION_DISPOSE ) /* sprites */
	ROM_LOAD( "epr10071.7m", 0x0000, 0x4000, CRC(5f55527a) SHA1(ec1ba8f95ac47a0c783e117ef4af6fe0ab5925b5) )
	ROM_LOAD( "epr10070.7k", 0x4000, 0x4000, CRC(97f35a23) SHA1(869553a334e1b3ba900a8b9c9eaf25fbc6ab31dd) )
	ROM_LOAD( "epr10069.7j", 0x8000, 0x4000, CRC(c0c3deaf) SHA1(8bf2e2146b794a330a079dd080f0586500964b1a) )

	ROM_REGION( 0x6000, REGION_GFX2, ROMREGION_DISPOSE ) /* tiles  */
	ROM_LOAD( "epr10082.5r", 0x0000, 0x2000, CRC(5984ea44) SHA1(010011b5b8dfa593c6fc7d2366f8cf82fcc8c978) )
	ROM_LOAD( "epr10081.4r", 0x2000, 0x2000, CRC(e2d45dd8) SHA1(5e11089680b574ea4cbf64510e51b0a945f79174) )
	ROM_LOAD( "epr10080.3r", 0x4000, 0x2000, CRC(42edfcfe) SHA1(feba7b1daffcad24d4c24f55ab5466f8cebf31ad) )

	ROM_REGION( 0xc000, REGION_GFX3, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "epr10078.7d", 0x0000, 0x4000, CRC(5b8eecce) SHA1(e7eee82081939b361edcbb9587b072b4b9a162f9) )
	ROM_LOAD( "epr10077.6d", 0x4000, 0x4000, CRC(01bcb609) SHA1(5d01fa75f214d34483284aaaef985ab92a606505) )
	ROM_LOAD( "epr10076.5d", 0x8000, 0x4000, CRC(b1529782) SHA1(8e0e92aae4c8dd8720414372aa767054cc316a0f) )
ROM_END


static DRIVER_INIT( calorie )
{
	calorie_decode();
}

static DRIVER_INIT( calorieb )
{
	unsigned char *rom = memory_region(REGION_CPU1);
	int diff = memory_region_length(REGION_CPU1) / 2;

	memory_set_opcode_base(0,rom+diff);
}


/* Note: the bootleg is identical to the original once decrypted */
GAME( 1986, calorie,  0,       calorie, calorie, calorie,  ROT0, "Sega",    "Calorie Kun vs Moguranian" )
GAME( 1986, calorieb, calorie, calorie, calorie, calorieb, ROT0, "bootleg", "Calorie Kun vs Moguranian (bootleg)" )

/*
gamezfan adds a new driver for Calorie Kun vs Moguranian then added support for 
Space Position to angelkds.c and hooked both games upto the Sega 317 Decryption
*/

#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()