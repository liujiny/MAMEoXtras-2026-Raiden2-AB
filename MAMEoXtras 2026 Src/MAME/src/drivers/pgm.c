#pragma code_seg("C562")
#pragma data_seg("D562")
#pragma bss_seg("B562")
#pragma const_seg("K562")
#pragma comment(linker, "/merge:D562=562")
#pragma comment(linker, "/merge:C562=562")
#pragma comment(linker, "/merge:B562=562")
#pragma comment(linker, "/merge:K562=562")
/* PGM System (c)1997 IGS

Based on Information from ElSemi

A flexible cartridge based platform some would say was designed to compete with
SNK's NeoGeo and Capcom's CPS Hardware systems, despite its age it only uses a
68000 for the main processor and a Z80 to drive the sound, just like the two
previously mentioned systems in that respect..

Resolution is 448x224, 15 bit colour

Sound system is ICS WaveFront 2115 Wavetable midi synthesizer, used in some
actual sound cards (Turtle Beach)

Later games are encrypted.  Latest games (kov2, ddp2) include an arm7
coprocessor with an internal rom and an encrypted external rom.

Roms Contain the Following Data

Pxxxx - 68k Program
Txxxx - TX & BG Graphics (2 formats within the same rom)
Mxxxx - Music samples (8 bit mono 11025Hz)
Axxxx - Colour Data (for sprites)
Bxxxx - Masks & A Rom Colour Indexes (for sprites)

There is no rom for the Z80, the program is uploaded by the 68k

Known Games on this Platform
----------------------------

Oriental Legend
Oriental Legend Super
Sengoku Senki / Knights of Valour Series
-
Sangoku Senki (c)1999 IGS
Sangoku Senki Super Heroes (c)1999 IGS
Sangoku Senki 2 Knights of Valour (c)2000 IGS
Sangoku Senki Busyou Souha (c)2001 IGS
-
DoDonPachi II (Bee Storm)
Photo Y2K
Photo Y2K II
Martial Masters
The Killing Blade
Dragon World 2
Dragon World 3
Dragon World 2001
Demon Front (c) 2002
The Gladiator (c) 2002
Puzzli II

There is also a single board version of the PGM system used by

Demon Front
Later Cave shooters (with different bios?)


To Do / Notes:

some sprite problems
optimize?
layer enables?
sprites use dma?
verify some things
protection in many games

General Notes:

the current 'kov' sets were from 'sango' boards but the protection determines the region so
it makes more sense to name them kov since the roms are probably the same on the various
boards.  The current sets were taken from taiwan boards incase somebody finds
it not to be the case however due to the previous note.

dragon world 2 still has strange protection issues, we have to patch the code for now, what
should really happen, it jumps to invalid code, should the protection device cause the 68k
to see valid code there or something?

kov superheroes uses a different protection chip / different protection commands and doesn't
work, some of the gfx also need redumping to check they're the same as kov, its using invalid
codes for the ones we have (could just be protection tho)


Protection Devices / Co-processors

IGS used a variety of additional ASIC chips on the game boards, these act as protection and
also give additional power to the board to make up for the limited power of the 68000
processor.  Some protection devices use external data roms, others have internal code only.
Most of these are not emulated correctly.

ASIC 3:
	used by:
	different per region, supplies region code
	used by:
	Oriental Legend
	function:

ASIC 12 + ASIC 25
	these seem to be used together
	ASIC 25 appears to perform some kind of bitswap operations
	used by:
	Dragon World 2

ASIC 22 + ASIC 25
	these seem to be used together, ASIC22 (could be 25..) has an external software decrypted? data rom
	ASIC 22 might be an updated version of ASIC12 ?
	used by:
	Dragon World 3
	The Killing Blade
	Oriental Legend Super (maybe, not confirmed)

ASIC 28:
	performs a variety of calculations, quite complex, different per region, supplies region code
	used by:
	Knights of Valour 1 / Plus / Superheroes (plus & superheroes seems to use extra functions, emulation issues reported in places in plus)
	Photo Y2k / Real and Fake (maybe..)

ASIC 27:
	arm9? cpu with 64kb internal rom (different per game) + external data rom
	probably used to give extra power to the system, lots of calculations are offloaded to it
	used by:
	DoDonPachi II
	Knights of Valor 2 / 2 Plus
	Martial Masters
	Demon Front
	Puzzli II

there are probably more...

PCB Layout
----------

IGS PCB NO-0133-2 (Main Board)
|-------------------------------------------------------------------------------------|
|   |----------------------------|   |----------|   |----------------------------|    |
|   |----------------------------|   |----------|   |----------------------------|    |
|                                      PGM_T01S.U29  UM61256    SRM2B61256  SRM2B61256|
| |---------|  33.8688MHz   |----------|                        SRM2B61256  SRM2B61256|
| |WAVEFRONT|               |L8A0290   |   UM6164  UM6164                             |
| |ICS2115V |               |IGS023    |                 PGM_P01S.U20              SW2|
| |(PLCC84) |               |(QFP256)  |                                              |
| |         |               |          |                                              |
| |---------|        50MHz  |----------|                                              |
|    UPD6379  PGM_M01S.U18                             |----------|                   |
|VOL                                                   |MC68HC000 |          74HC132  |
|                                                      |FN20      |   20MHz  74HC132  |
|  UPC844C    |------|                                 |(PLCC68)  |                   |
|             |Z80   |                                 |          |          V3021    |
|             |PLCC44|                  PAL            |----------|                   |
|             |------|    |--------|                                      32.768kHz   |-|
|                         |IGS026  |                                                    |
|                         |(QFP144)|           |--------|                              I|
|                         |        |           |IGS026  |                              D|
|                         |--------|           |(QFP144)|                              C|
|TDA1519A    UM61256 UM61256                   |        |                              3|
|                              TD62064         |--------|                              4|
|                                                                          3.6V_BATT    |
|                                                                                     |-|
|              |----|                                           |-----|     SW3       |
|              |    |               J  A  M  M  A               |     | SW1           |
|--------------|    |-------------------------------------------|     |---------------|


IGS PCB NO-0136 (Riser)
|-------------------------------------------------------------------------------------|
|      |---------------------------------|  |---------------------------------|       |
|      |---------------------------------|  |---------------------------------|       |
|                                                                                     |
|      |---------------------------------|  |---------------------------------|       |
|      |---------------------------------|  |---------------------------------|       |
|                                                                                     |
|   |----------------------------|   |----------|   |----------------------------|    |
|---|                            |---|          |---|                            |----|
    |----------------------------|   |----------|   |----------------------------|

Notes:
      All IC's are shown.

      CPU's
      -----
         68HC000FN20 - Motorola 68000 processor, clocked at 20.000MHz (PLCC68)
         Z80         - Zilog Z0840008VSC Z80 processor, clocked at 8.468MHz (PLCC44)

      SOUND
      -----
         ICS2115     - ICS WaveFront ICS2115V Wavetable Midi Synthesizer, clocked at 33.8688MHz (PLCC84)

      RAM
      ---
         SRM2B256 - Epson SRM2B256SLMX55 8K x8 SRAM (x4, SOP28)
         UM6164   - Unicorn Microelectronics UM6164DS-12 8K x8 SRAM (x2, SOJ28)
         UM61256  - Unicorn Microelectronics UM61256FS-15 32K x8 SRAM (x3, SOJ28)

      ROMs
      ----
         PGM_M01S.U18 - 16MBit MASKROM (TSOP48)
         PGM_P01S.U20 - 1MBit  MASKROM (DIP40, socketed, equivalent to 27C1024 EPROM)
         PGM_T01S.U29 - 16MBit MASKROM (SOP44)

      CUSTOM IC's
      -----------
         IGS023 (QFP256)
         IGS026 (x2, QFP144)

      OTHER
      -----
         3.6V_BATT - 3.6V NICad battery, connected to the V3021 RTC
         IDC34     - IDC34 way flat cable plug, doesn't appear to be used for any games. Might be for
                     re-programming some of the custom IC's or on-board surface mounted ROMs?
         PAL       - Atmel ATF16V8B PAL (DIP20)
         SW1       - Push button switch to enter Test Mode
         SW2       - 8 position DIP Switch (for configuration of PCB/game options)
         SW3       - SPDT switch (purpose unknown)
         TD62064   - Toshiba NPN 50V 1.5A Quad Darlinton Switch; for driving coin meters (DIP16)
         TDA1519A  - Philips 2x 6W Stereo Power AMP (SIL9)
         uPD6379   - NEC 2-channel 16-bit D/A converter 10mW typ. (SOIC8)
         uPC844C   - NEC Quad High Speed Wide Band Operational Amplifier (DIP14)
         V3021     - EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS Real Time Clock (DIP8)
         VOL       - Volume potentiometer

*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/ics2115.h"
#include <time.h>
#include "timer.h"

static MACHINE_INIT( killbld );
static MACHINE_INIT( olds );
data16_t *pgm_mainram, *pgm_bg_videoram, *pgm_tx_videoram, *pgm_videoregs, *pgm_rowscrollram;
static data8_t *z80_mainram;
WRITE16_HANDLER( pgm_tx_videoram_w );
WRITE16_HANDLER( pgm_bg_videoram_w );
VIDEO_START( pgm );
VIDEO_EOF( pgm );
VIDEO_UPDATE( pgm );
void pgm_kov_decrypt(void);
void pgm_kovsh_decrypt(void);
void pgm_dw2_decrypt(void);
void pgm_djlzz_decrypt(void);
void pgm_dw3_decrypt(void);
void pgm_killbld_decrypt(void);
void pgm_pstar_decrypt(void);
void pgm_ket_decrypt(void);
void pgm_espgal_decrypt(void);
void pgm_py2k2_decrypt(void);
void pgm_puzzli2_decrypt(void);

READ16_HANDLER( pgm_calendar_r );
READ16_HANDLER( pgm_asic3_r );
WRITE16_HANDLER( pgm_asic3_w );
WRITE16_HANDLER( pgm_asic3_reg_w );

READ16_HANDLER (sango_protram_r);
READ16_HANDLER (ASIC28_r16);
WRITE16_HANDLER (ASIC28_w16);

READ16_HANDLER (dw2_d80000_r );


static READ16_HANDLER ( z80_ram_r )
{
	return (z80_mainram[offset*2] << 8)|z80_mainram[offset*2+1];
}

static WRITE16_HANDLER ( z80_ram_w )
{
	int pc = activecpu_get_pc();
	if(ACCESSING_MSB)
		z80_mainram[offset*2] = data >> 8;
	if(ACCESSING_LSB)
		z80_mainram[offset*2+1] = data;

	if(pc != 0xf12 && pc != 0xde2 && pc != 0x100c50 && pc != 0x100b20)
		logerror("Z80: write %04x, %04x @ %04x (%06x)\n", offset*2, data, mem_mask, activecpu_get_pc());
}

static WRITE16_HANDLER ( z80_reset_w )
{
	logerror("Z80: reset %04x @ %04x (%06x)\n", data, mem_mask, activecpu_get_pc());

	if(data == 0x5050) {
		ics2115_reset();
		cpunum_set_input_line(1, INPUT_LINE_HALT, CLEAR_LINE);
		cpunum_set_input_line(1, INPUT_LINE_RESET, PULSE_LINE);
		if(0) {
			FILE *out;
			out = fopen("z80ram.bin", "wb");
			fwrite(z80_mainram, 1, 65536, out);
			fclose(out);
		}
	}
	else
	{
		/* this might not be 100% correct, but several of the games (ddp2, puzzli2 etc. expect the z80 to be turned
           off during data uploads, they write here before the upload */
		cpunum_set_input_line(1, INPUT_LINE_HALT, ASSERT_LINE);
		//Note Puzzle Star needs this also
	}
}

static WRITE16_HANDLER ( z80_ctrl_w )
{
	logerror("Z80: ctrl %04x @ %04x (%06x)\n", data, mem_mask, activecpu_get_pc());
}

static WRITE16_HANDLER ( m68k_l1_w )
{
	if(ACCESSING_LSB) {
		logerror("SL 1 m68.w %02x (%06x) IRQ\n", data & 0xff, activecpu_get_pc());
		soundlatch_w(0, data);
		cpunum_set_input_line(1, INPUT_LINE_NMI, PULSE_LINE );
	}
}

static WRITE8_HANDLER( z80_l3_w )
{
	logerror("SL 3 z80.w %02x (%04x)\n", data, activecpu_get_pc());
	soundlatch3_w(0, data);
}

static void sound_irq(int level)
{
	cpunum_set_input_line(1, 0, level);
}

struct ics2115_interface pgm_ics2115_interface = {
	{ MIXER(100, MIXER_PAN_LEFT), MIXER(100, MIXER_PAN_RIGHT) },
	REGION_SOUND1,
	sound_irq
};

/* Calendar Emulation */

static unsigned char CalVal, CalMask, CalCom=0, CalCnt=0;

static unsigned char bcd(unsigned char data)
{
	return ((data / 10) << 4) | (data % 10);
}

READ16_HANDLER( pgm_calendar_r )
{
	unsigned char calr;
	calr = (CalVal & CalMask) ? 1 : 0;
	CalMask <<= 1;
	return calr;
}

WRITE16_HANDLER( pgm_calendar_w )
{
	static time_t ltime;
	static struct tm *today;

	// initialize the time, otherwise it crashes
	if( !ltime )
	{
		time(&ltime);
		today = localtime(&ltime);
	}

	CalCom <<= 1;
	CalCom |= data & 1;
	++CalCnt;
	if(CalCnt==4)
	{
		CalMask = 1;
		CalVal = 1;
		CalCnt = 0;
		switch(CalCom & 0xf)
		{
			case 1: case 3: case 5: case 7: case 9: case 0xb: case 0xd:
				CalVal++;
				break;

			case 0:
				CalVal=bcd(today->tm_wday); //??
				break;

			case 2:  //Hours
				CalVal=bcd(today->tm_hour);
				break;

			case 4:  //Seconds
				CalVal=bcd(today->tm_sec);
				break;

			case 6:  //Month
				CalVal=bcd(today->tm_mon + 1); //?? not bcd in MVS
				break;

			case 8:
				CalVal=0; //Controls blinking speed, maybe milliseconds
				break;

			case 0xa: //Day
				CalVal=bcd(today->tm_mday);
				break;

			case 0xc: //Minute
				CalVal=bcd(today->tm_min);
				break;

			case 0xe:  //Year
				CalVal=bcd(today->tm_year % 100);
				break;

			case 0xf:  //Load Date
				time(&ltime);
				today = localtime(&ltime);
				break;
		}
	}
}


static READ16_HANDLER( nvram_r )
{
	return ((data16_t *)generic_nvram)[offset] | 0xfff0;
}


/*** Memory Maps *************************************************************/

static ADDRESS_MAP_START( pgm_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
	AM_RANGE(0x100000, 0x5fffff) AM_ROMBANK(1) /* Game ROM */

	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) /* Main Ram */

	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(MRA16_RAM, pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) /* Backgrounds */
	AM_RANGE(0x904000, 0x905fff) AM_READWRITE(MRA16_RAM, pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) /* Text Layer */
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_READWRITE(MRA16_RAM, paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ(input_port_0_word_r) // p1+p2 controls
	AM_RANGE(0xc08002, 0xc08003) AM_READ(input_port_1_word_r) // p3+p4 controls
	AM_RANGE(0xc08004, 0xc08005) AM_READ(input_port_2_word_r) // extra controls
	AM_RANGE(0xc08006, 0xc08007) AM_READ(input_port_3_word_r) // dipswitches

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END

static ADDRESS_MAP_START( cavepgm_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?
	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram)
	AM_RANGE(0x800000, 0x81ffff) AM_READWRITE(nvram_r, MWA16_RAM) AM_BASE((data16_t **)&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(MRA16_RAM, pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) // Backgrounds 
	AM_RANGE(0x904000, 0x905fff) AM_READWRITE(MRA16_RAM, pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) // Text Layer 
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_READWRITE(MRA16_RAM, paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) // Video Regs inc. Zoom Table

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ(input_port_0_word_r) // p1+p2 controls
	AM_RANGE(0xc08002, 0xc08003) AM_READ(input_port_1_word_r) // p3+p4 controls
	AM_RANGE(0xc08004, 0xc08005) AM_READ(input_port_2_word_r) // extra controls
	AM_RANGE(0xc08006, 0xc08007) AM_READ(input_port_3_word_r) // dipswitches

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END

static UINT16 *killbld_sharedprotram;

static ADDRESS_MAP_START( killbld_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
	AM_RANGE(0x100000, 0x2fffff) AM_ROMBANK(1) /* Game ROM */
	AM_RANGE(0x300000, 0x303fff) AM_RAM AM_BASE(&killbld_sharedprotram) // Shared with protection device

	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) /* Main Ram */

	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(MRA16_RAM, pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) /* Backgrounds */
	AM_RANGE(0x904000, 0x905fff) AM_READWRITE(MRA16_RAM, pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) /* Text Layer */
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_READWRITE(MRA16_RAM, paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ(input_port_0_word_r) // p1+p2 controls
	AM_RANGE(0xc08002, 0xc08003) AM_READ(input_port_1_word_r) // p3+p4 controls
	AM_RANGE(0xc08004, 0xc08005) AM_READ(input_port_2_word_r) // extra controls
	AM_RANGE(0xc08006, 0xc08007) AM_READ(input_port_3_word_r) // dipswitches

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END

static UINT16 *sharedprotram;

static ADDRESS_MAP_START( olds_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
	AM_RANGE(0x100000, 0x3fffff) AM_ROMBANK(1) /* Game ROM */
	AM_RANGE(0x400000, 0x403fff) AM_RAM AM_BASE(&sharedprotram) // Shared with protection device
	
	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) /* Main Ram */

	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(MRA16_RAM, pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) /* Backgrounds */
	AM_RANGE(0x904000, 0x905fff) AM_READWRITE(MRA16_RAM, pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) /* Text Layer */
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_READWRITE(MRA16_RAM, paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ(input_port_0_word_r) // p1+p2 controls
	AM_RANGE(0xc08002, 0xc08003) AM_READ(input_port_1_word_r) // p3+p4 controls
	AM_RANGE(0xc08004, 0xc08005) AM_READ(input_port_2_word_r) // extra controls
	AM_RANGE(0xc08006, 0xc08007) AM_READ(input_port_3_word_r) // dipswitches

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_BASE(&z80_mainram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x8000, 0x8003) AM_READWRITE(ics2115_r, ics2115_w)
	AM_RANGE(0x8100, 0x81ff) AM_READWRITE(soundlatch3_r, z80_l3_w)
	AM_RANGE(0x8200, 0x82ff) AM_READWRITE(soundlatch_r, soundlatch_w)
	AM_RANGE(0x8400, 0x84ff) AM_READWRITE(soundlatch2_r, soundlatch2_w)
ADDRESS_MAP_END


/*** Input Ports *************************************************************/

/* enough for 4 players, the basic dips mapped are listed in the test mode */

INPUT_PORTS_START( pgm )
	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER2 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START4                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER4 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER4 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER4 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER4 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
/*	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) */ /* test 1p+2p*/
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*  what should i use?*/
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* service 1p+2p*/
/*	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) */ /* test 3p+4p*/
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* what should i use?*/
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* service 3p+4p*/
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/

	PORT_START	/* DSW */
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Free" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stop" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START	/* Region */
	PORT_DIPNAME( 0x0003, 0x0000, "Region" )
	PORT_DIPSETTING(      0x0000, "World" )
/*	PORT_DIPSETTING(      0x0001, "World" ) */ /* again?*/
	PORT_DIPSETTING(      0x0002, "Korea" )
	PORT_DIPSETTING(      0x0003, "China" )
INPUT_PORTS_END


INPUT_PORTS_START( sango )
	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER2 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START4                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER4 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER4 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER4 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER4 )
	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
//	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) // test 1p+2p
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) //  what should i use?
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service 1p+2p
//	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) // test 3p+4p
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // what should i use?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) // service 3p+4p
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?

	PORT_START	/* DSW */
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Free" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stop" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START	/* Region */
	PORT_DIPNAME( 0x000f, 0x0005, "Region" )
	PORT_DIPSETTING(      0x0000, "China" )
	PORT_DIPSETTING(      0x0001, "Taiwan" )
	PORT_DIPSETTING(      0x0002, "Japan (Alta License)" )
	PORT_DIPSETTING(      0x0003, "Korea" )
	PORT_DIPSETTING(      0x0004, "Hong Kong" )
	PORT_DIPSETTING(      0x0005, "World" )
INPUT_PORTS_END


INPUT_PORTS_START( photoy2k )
	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START4                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER4 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER4 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER4 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER4 )
	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
//  PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) // test 1p+2p
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) //  what should i use?
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service 1p+2p
//  PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) // test 3p+4p
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // what should i use?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) // service 3p+4p
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?

	PORT_START	/* DSW */
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Free" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stop" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START	/* Region - supplied by protection device */
	PORT_DIPNAME( 0x000f, 0x0003, "Region" )
	PORT_DIPSETTING(      0x0000, "Taiwan" )
	PORT_DIPSETTING(      0x0001, "China" )
	PORT_DIPSETTING(      0x0002, "Japan (Alta License)" )
	PORT_DIPSETTING(      0x0003, "World")
	PORT_DIPSETTING(      0x0004, "Korea" )
	PORT_DIPSETTING(      0x0005, "Hong Kong" )
INPUT_PORTS_END

static INPUT_PORTS_START( killbld )
	PORT_START/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START4                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_PLAYER4 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_PLAYER4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER4 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER4 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER4 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
//  PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) // test 1p+2p
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) //  what should i use?
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service 1p+2p
//  PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) // test 3p+4p
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // what should i use?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) // service 3p+4p
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // uused?

	PORT_START /* DSW */
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Free" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stop" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START_TAG("Region") /* Region - supplied by protection device */
	PORT_DIPNAME( 0x00ff, 0x0021, "Region" )
	PORT_DIPSETTING(      0x0016, "Taiwan" )
	PORT_DIPSETTING(      0x0017, "China" )
	PORT_DIPSETTING(      0x0018, "Hong Kong" )
	PORT_DIPSETTING(      0x0019, "Japan" )
//  PORT_DIPSETTING(      0x001a, "1a" ) // invalid
//  PORT_DIPSETTING(      0x001b, "1b" ) // invalid
//  PORT_DIPSETTING(      0x001c, "1c" ) // invalid
//  PORT_DIPSETTING(      0x001d, "1d" ) // invalid
//  PORT_DIPSETTING(      0x001e, "1e" ) // invalid
//  PORT_DIPSETTING(      0x001f, "1f" ) // invalid
	PORT_DIPSETTING(      0x0020, "Korea" )
	PORT_DIPSETTING(      0x0021, "World" )
INPUT_PORTS_END

INPUT_PORTS_START( olds )
	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER2 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START4                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER4 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER4 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER4 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER4 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
/*	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) */ /* test 1p+2p*/
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*  what should i use?*/
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* service 1p+2p*/
/*	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) */ /* test 3p+4p*/
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* what should i use?*/
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* service 3p+4p*/
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/

	PORT_START	/* DSW */
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Free" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stop" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START	/* Region - supplied by protection device */
	PORT_DIPNAME( 0x000f, 0x0006, "Region" )
	PORT_DIPSETTING(      0x0001, "Taiwan" )
	PORT_DIPSETTING(      0x0002, "China" )
	PORT_DIPSETTING(      0x0003, "Japan" )
	PORT_DIPSETTING(      0x0004, "Korea")
	PORT_DIPSETTING(      0x0005, "Hong Kong" )
	PORT_DIPSETTING(      0x0006, "World" )
INPUT_PORTS_END

INPUT_PORTS_START( puzzli2 )
	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER2 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3                       )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER3 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START4                       )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_PLAYER4 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_PLAYER4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_PLAYER4 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1        | IPF_PLAYER4 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2        | IPF_PLAYER4 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3        | IPF_PLAYER4 )

	PORT_START	/* DSW */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
/*	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) */ /* test 1p+2p*/
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) /*  what should i use?*/
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* service 1p+2p*/
/*	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) */ /* test 3p+4p*/
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* what should i use?*/
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* service 3p+4p*/
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* uused?*/

	PORT_START	/* DSW */
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Free" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stop" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START	/* Region */
	PORT_DIPNAME( 0x000f, 0x0005, "Region" )
	PORT_DIPSETTING(      0x0000, "Taiwan" )
	PORT_DIPSETTING(      0x0001, "China" )
	PORT_DIPSETTING(      0x0002, "Japan (Alta License)" )
	PORT_DIPSETTING(      0x0003, "Korea" )
	PORT_DIPSETTING(      0x0004, "Hong Kong" )
	PORT_DIPSETTING(      0x0005, "World" )
INPUT_PORTS_END


/*** GFX Decodes *************************************************************/

/* we can't decode the sprite data like this because it isn't tile based.  the
   data decoded by pgm32_charlayout was rearranged at start-up */

static struct GfxLayout pgm8_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4, 0, 12, 8, 20,16,  28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static struct GfxLayout pgm32_charlayout =
{
	32,32,
	RGN_FRAC(1,1),
	5,
	{ 3,4,5,6,7 },
	{ 0  , 8 ,16 ,24 ,32 ,40 ,48 ,56 ,
	  64 ,72 ,80 ,88 ,96 ,104,112,120,
	  128,136,144,152,160,168,176,184,
	  192,200,208,216,224,232,240,248 },
	{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256,
	  8*256, 9*256,10*256,11*256,12*256,13*256,14*256,15*256,
	 16*256,17*256,18*256,19*256,20*256,21*256,22*256,23*256,
	 24*256,25*256,26*256,27*256,28*256,29*256,30*256,31*256 },
	 32*256
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &pgm8_charlayout,    0x800, 32  }, /* 8x8x4 Tiles */
	{ REGION_GFX2, 0, &pgm32_charlayout,   0x400, 32  }, /* 32x32x5 Tiles */
	{ -1 } /* end of array */
};

/*** Machine Driver **********************************************************/

static INTERRUPT_GEN( pgm_interrupt ) {
	if( cpu_getiloops() == 0 )
		cpunum_set_input_line(0, 6, HOLD_LINE);
	else
		cpunum_set_input_line(0, 4, HOLD_LINE);
}

static MACHINE_INIT( pgm )
{
	cpunum_set_input_line(1, INPUT_LINE_HALT, ASSERT_LINE);
}

static MACHINE_INIT( olds )
{
	machine_init_pgm();
	//	written by protection device
//	there seems to be an auto-dma that writes from $401000-402573?
	sharedprotram[0x1000/2] = 0x4749; // 'IGS.28'
	sharedprotram[0x1002/2] = 0x2E53;
	sharedprotram[0x1004/2] = 0x3832;

	sharedprotram[0x3064/2] = 0xB315; // crc?
}

static MACHINE_DRIVER_START( pgm )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 20000000) /* 20 mhz! verified on real board */
	MDRV_CPU_PROGRAM_MAP(pgm_mem,0)
	MDRV_CPU_VBLANK_INT(pgm_interrupt,2)

    MDRV_CPU_ADD(Z80, 8468000)
	MDRV_CPU_PROGRAM_MAP(z80_mem, 0)
	MDRV_CPU_IO_MAP(z80_io, 0)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU | CPU_16BIT_PORT)

	MDRV_SOUND_ADD(ICS2115, pgm_ics2115_interface)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x1200/2)

	MDRV_VIDEO_START(pgm)
	MDRV_VIDEO_EOF(pgm)
	MDRV_VIDEO_UPDATE(pgm)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cavepgm )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 20000000) /* 20 mhz! verified on real board */
	MDRV_CPU_PROGRAM_MAP(cavepgm_mem,0)
	MDRV_CPU_VBLANK_INT(pgm_interrupt,2)

	MDRV_CPU_ADD(Z80, 8468000)
	MDRV_CPU_PROGRAM_MAP(z80_mem, 0)
	MDRV_CPU_IO_MAP(z80_io, 0)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU | CPU_16BIT_PORT)
	
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_SOUND_ADD(ICS2115, pgm_ics2115_interface)

    MDRV_FRAMES_PER_SECOND(59.170000)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x1200/2)

	MDRV_VIDEO_START(pgm)
	MDRV_VIDEO_EOF(pgm)
	MDRV_VIDEO_UPDATE(pgm)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( killbld )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 20000000) /* 20 mhz! verified on real board */
	MDRV_CPU_PROGRAM_MAP(killbld_mem,0)
	MDRV_CPU_VBLANK_INT(pgm_interrupt,2)

    MDRV_CPU_ADD(Z80, 8468000)
	MDRV_CPU_PROGRAM_MAP(z80_mem, 0)
	MDRV_CPU_IO_MAP(z80_io, 0)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU | CPU_16BIT_PORT)

	MDRV_SOUND_ADD(ICS2115, pgm_ics2115_interface)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_MACHINE_INIT(killbld)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x1200/2)

	MDRV_VIDEO_START(pgm)
	MDRV_VIDEO_EOF(pgm)
	MDRV_VIDEO_UPDATE(pgm)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( olds )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 20000000) /* 20 mhz! verified on real board */
	MDRV_CPU_PROGRAM_MAP(olds_mem,0)
	MDRV_CPU_VBLANK_INT(pgm_interrupt,2)

    MDRV_CPU_ADD(Z80, 8468000)
	MDRV_CPU_PROGRAM_MAP(z80_mem, 0)
	MDRV_CPU_IO_MAP(z80_io, 0)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU | CPU_16BIT_PORT)

	MDRV_SOUND_ADD(ICS2115, pgm_ics2115_interface)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_MACHINE_INIT(olds)
	
	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER )
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x1200/2)

	MDRV_VIDEO_START(pgm)
	MDRV_VIDEO_EOF(pgm)
	MDRV_VIDEO_UPDATE(pgm)
MACHINE_DRIVER_END

/*** Init Stuff **************************************************************/

/* This function expands the 32x32 5-bit data into a format which is easier to
   decode in MAME */

static void expand_32x32x5bpp(void)
{
	data8_t *src    = memory_region       ( REGION_GFX1 );
	data8_t *dst    = memory_region       ( REGION_GFX2 );
	size_t  srcsize = memory_region_length( REGION_GFX1 );
	int cnt, pix;

	for (cnt = 0; cnt < srcsize/5 ; cnt ++)
	{
		pix =  ((src[0+5*cnt] >> 0)& 0x1f );							  dst[0+8*cnt]=pix;
		pix =  ((src[0+5*cnt] >> 5)& 0x07) | ((src[1+5*cnt] << 3) & 0x18);dst[1+8*cnt]=pix;
		pix =  ((src[1+5*cnt] >> 2)& 0x1f );		 					  dst[2+8*cnt]=pix;
		pix =  ((src[1+5*cnt] >> 7)& 0x01) | ((src[2+5*cnt] << 1) & 0x1e);dst[3+8*cnt]=pix;
		pix =  ((src[2+5*cnt] >> 4)& 0x0f) | ((src[3+5*cnt] << 4) & 0x10);dst[4+8*cnt]=pix;
		pix =  ((src[3+5*cnt] >> 1)& 0x1f );							  dst[5+8*cnt]=pix;
		pix =  ((src[3+5*cnt] >> 6)& 0x03) | ((src[4+5*cnt] << 2) & 0x1c);dst[6+8*cnt]=pix;
		pix =  ((src[4+5*cnt] >> 3)& 0x1f );							  dst[7+8*cnt]=pix;
	}
}

/* This function expands the sprite colour data (in the A Roms) from 3 pixels
   in each word to a byte per pixel making it easier to use */

data8_t *pgm_sprite_a_region;
size_t	pgm_sprite_a_region_allocate;

static void expand_colourdata(void)
{
	data8_t *src    = memory_region       ( REGION_GFX3 );
	size_t  srcsize = memory_region_length( REGION_GFX3 );
	int cnt;
	size_t	needed = srcsize / 2 * 3;

	/* work out how much ram we need to allocate to expand the sprites into
	   and be able to mask the offset */
	pgm_sprite_a_region_allocate = 1;
	while (pgm_sprite_a_region_allocate < needed)
		pgm_sprite_a_region_allocate = pgm_sprite_a_region_allocate <<1;

	pgm_sprite_a_region = auto_malloc (pgm_sprite_a_region_allocate);


	for (cnt = 0 ; cnt < srcsize/2 ; cnt++)
	{
		data16_t colpack;

		colpack = ((src[cnt*2]) | (src[cnt*2+1] << 8));
		pgm_sprite_a_region[cnt*3+0] = (colpack >> 0 ) & 0x1f;
		pgm_sprite_a_region[cnt*3+1] = (colpack >> 5 ) & 0x1f;
		pgm_sprite_a_region[cnt*3+2] = (colpack >> 10) & 0x1f;
	}
}

static void pgm_basic_init(void)
{
	unsigned char *ROM = memory_region(REGION_CPU1);
	cpu_setbank(1,&ROM[0x100000]);

	expand_32x32x5bpp();
	expand_colourdata();
}

/* Oriental Legend INIT */


static DRIVER_INIT( orlegend )
{
	pgm_basic_init();

	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0xC0400e, 0xC0400f, 0, 0, pgm_asic3_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0xC04000, 0xC04001, 0, 0, pgm_asic3_reg_w);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0xC0400e, 0xC0400f, 0, 0, pgm_asic3_w);

}

static void drgwld2_common_init(void)
{
	pgm_basic_init();
	pgm_dw2_decrypt();
	/*
	Info from Elsemi
	Here is how to "bypass" the dw2 hang protection, it fixes the mode
	select and after failing in the 2nd stage (probably there are other checks
	out there).
	*/
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0xd80000, 0xd80003, 0, 0, dw2_d80000_r);
}

static DRIVER_INIT( drgw2 )
{
	data16_t *mem16 = (data16_t *)memory_region(REGION_CPU1);
	drgwld2_common_init();
	/* patches .. i hate patches .. */
	mem16[0x131098/2]=0x4e93;
	mem16[0x13113e/2]=0x4e93;
	mem16[0x1311ce/2]=0x4e93;
}

static DRIVER_INIT( drgw2c )
{
	data16_t *mem16 = (data16_t *)memory_region(REGION_CPU1);
	drgwld2_common_init();
	/* this seems to be some strange protection
	ElSemi
	patch the jmp (a0) to jmp(a3) (otherwise they jump to illegal code)
	there are 3 (consecutive functions)
	303bc, 30462, 304f2
	*/
	mem16[0x1303bc/2]=0x4e93;
	mem16[0x130462/2]=0x4e93;
	mem16[0x1304F2/2]=0x4e93;
}

static DRIVER_INIT( kov )
{
	pgm_basic_init();

	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500003, 0, 0, ASIC28_r16);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500003, 0, 0, ASIC28_w16);

	/* 0x4f0000 - ? is actually ram shared with the protection device,
	  the protection device provides the region code */
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x4f0000, 0x4fffff, 0, 0, sango_protram_r);

 	pgm_kov_decrypt();
}

static DRIVER_INIT( kovsh )
{
	pgm_basic_init();

	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500003, 0, 0, ASIC28_r16);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500003, 0, 0, ASIC28_w16);

	/* 0x4f0000 - ? is actually ram shared with the protection device,
	  the protection device provides the region code */
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x4f0000, 0x4fffff, 0, 0, sango_protram_r);

 	pgm_kovsh_decrypt();
}

static DRIVER_INIT( djlzz )
{
	pgm_basic_init();

	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500003, 0, 0, ASIC28_r16);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500003, 0, 0, ASIC28_w16);

	/* 0x4f0000 - ? is actually ram shared with the protection device,
	  the protection device provides the region code */
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x4f0000, 0x4fffff, 0, 0, sango_protram_r);

 	pgm_djlzz_decrypt();
}

static DRIVER_INIT( dw3 )
{
	pgm_basic_init();

//	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0xda0000, 0xdaffff, 0, 0, dw3_prot_r);
//	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0xda0000, 0xdaffff, 0, 0, dw3_prot_w);

 	pgm_dw3_decrypt();
}

static DRIVER_INIT( olds103t )
{
	pgm_basic_init();
}


/* Killing Blade uses some kind of DMA protection device which can copy data from a data rom.  The
   MCU appears to have an internal ROM as if you remove the data ROM then the shared ram is filled
   with a constant value.
   The device can perform various decryption operations on the data it copies.
*/

static int kb_cmd;
static int kb_reg;
static int kb_ptr;
UINT32 kb_regs[0x10];


static WRITE16_HANDLER( killbld_prot_w )
{
	offset &= 0xf;

	if (offset == 0)
		kb_cmd = data;
	else /*offset==2 */
	{
		logerror("%06X: ASIC25 W CMD %X  VAL %X\n", activecpu_get_pc(),kb_cmd,data);
		if (kb_cmd == 0)
			kb_reg = data;
		else if (kb_cmd == 2)
		{
			if (data == 1)	/* Execute cmd */
			{
				UINT16 cmd = killbld_sharedprotram[0x200/2];

				if (cmd == 0x6d)	//Store values to asic ram
				{
					UINT32 p1 = (killbld_sharedprotram[0x298/2] << 16) | killbld_sharedprotram[0x29a/2];
					UINT32 p2 = (killbld_sharedprotram[0x29c/2] << 16) | killbld_sharedprotram[0x29e/2];

					if ((p2 & 0xffff) == 0x9)	//Set value
					{
						int reg = (p2 >> 16) & 0xffff;
						if (reg & 0x200)
							kb_regs[reg & 0xff] = p1;
					}
					if ((p2 & 0xffff) == 0x6)	//Add value
					{
						int src1 = (p1 >> 16) & 0xff;
						int src2 = (p1 >> 0) & 0xff;
						int dst = (p2 >> 16) & 0xff;
						kb_regs[dst] = kb_regs[src2] - kb_regs[src1];
					}
					if ((p2 & 0xffff) == 0x1)	//Add Imm?
					{
						int reg = (p2 >> 16) & 0xff;
						int imm = (p1 >> 0) & 0xffff;
						kb_regs[reg] += imm;
					}
					if ((p2 & 0xffff) == 0xa)	//Get value
					{
						int reg = (p1 >> 16) & 0xFF;
						killbld_sharedprotram[0x29c/2] = (kb_regs[reg] >> 16) & 0xffff;
						killbld_sharedprotram[0x29e/2] = kb_regs[reg] & 0xffff;
					}
				}
				if(cmd == 0x4f)	//memcpy with encryption / scrambling
				{
					UINT16 src = killbld_sharedprotram[0x290 / 2] >> 1; // ?
					UINT32 dst = killbld_sharedprotram[0x292 / 2];
					UINT16 size = killbld_sharedprotram[0x294 / 2];
					UINT16 mode = killbld_sharedprotram[0x296 / 2];

					UINT16 param;
					
					/*
                    P_SRC =0x300290 (offset from prot rom base)
                    P_DST =0x300292 (words from 0x300000)
                    P_SIZE=0x300294 (words)
                    P_MODE=0x300296
                    Mode 5 direct
                    Mode 6 swap nibbles and bytes
                    1,2,3 table based ops
                    */
	
					param = mode >> 8;
					mode &=0xf;  // what are the other bits?

					if (mode == 0)
					{
						printf("unhandled copy mode %04x!\n", mode);
						// not used by killing blade
						/* plain byteswapped copy */
					}
					if ((mode == 1) || (mode == 2) || (mode == 3))
					{
						/* mode3 applies a xor from a 0x100 byte table to the data being
						   transferred
						   the table is stored at the start of the protection rom.
						   the param used with the mode gives a start offset into the table
						   odd offsets seem to change the table slightly (see rawDataOdd)
					   */
						  					
						/*
						unsigned char rawDataOdd[256] = {
							0xB6, 0xA8, 0xB1, 0x5D, 0x2C, 0x5D, 0x4F, 0xC1,
							0xCF, 0x39, 0x3A, 0xB7,	0x65, 0x85, 0xD9, 0xEE,
							0xDB, 0x7B, 0x5F, 0x81, 0x03, 0x6D, 0xEB, 0x07,
							0x0F, 0xB5, 0x61, 0x59, 0xCD, 0x60, 0x06, 0x21,
							0xA0, 0x99, 0xDD, 0x27,	0x42, 0xD7, 0xC5, 0x5B,
							0x3B, 0xC6, 0x4F, 0xA2, 0x20, 0xF6, 0x61, 0x61,
							0x8C, 0x46, 0x8C, 0xCA, 0xE0, 0x0E, 0x2C, 0xE9,
							0xBA, 0x0F, 0x45, 0x6D,	0x36, 0x1C, 0x18, 0x37,
							0xE7, 0x85, 0x89, 0xA4, 0x94, 0x46, 0x30, 0x9B,
							0xB2, 0xF4, 0x41, 0x55, 0xA5, 0x63, 0x1C, 0xEF,
							0xB7, 0x18, 0xB3, 0xB1,	0xD4, 0x72, 0xA0, 0x1C,
							0x0B, 0x97, 0x02, 0xB6, 0xC5, 0x1F, 0x1B, 0x94,
							0xC3, 0x83, 0xAA, 0xAC, 0xD9, 0x44, 0x09, 0xD7,
							0x6C, 0xDB, 0x07, 0xA9,	0xAD, 0x64, 0x83, 0xF1,
							0x92, 0x09, 0xCD, 0x0E, 0x99, 0x2F, 0xBC, 0xF8,
							0x3C, 0x63, 0x8F, 0x0A, 0x33, 0x03, 0x84, 0x91,
							0x6C, 0xAC, 0x3A, 0x15,	0xCB, 0x67, 0xC7, 0x69,
							0xA1, 0x92, 0x99, 0x74, 0xEE, 0x90, 0x0D, 0xBE,
							0x57, 0x30, 0xD1, 0xBA, 0xE5, 0xDE, 0xFA, 0xD6,
							0x83, 0x8C, 0xE4, 0x43,	0x36, 0x5E, 0xCD, 0x84,
							0x1A, 0x18, 0x31, 0xB9, 0x20, 0x48, 0xE3, 0xA8,
							0x89, 0x32, 0xF0, 0x90, 0x21, 0x80, 0x33, 0xAE,
							0x3C, 0xA6, 0xB8, 0x8C,	0x72, 0x17, 0xD1, 0x0C,
							0x1A, 0x29, 0xFA, 0x38, 0x87, 0xC9, 0x6E, 0xC7,
							0x05, 0xDE, 0x85, 0x6E, 0x92, 0x7E, 0xD4, 0xED,
							0x5C, 0xD3, 0x03, 0xD4,	0xFE, 0xCB, 0x6C, 0x19,
							0x7A, 0x83, 0x79, 0x5B, 0xF6, 0x71, 0xBA, 0xF4,
							0x37, 0x53, 0xC9, 0xC1, 0xDE, 0xDB, 0xDE, 0xB1,
							0x64, 0x17, 0x31, 0x0E,	0xD7, 0xA2, 0x13, 0x8E,
							0x52, 0x8D, 0xCB, 0x19, 0x3D, 0x0B, 0x31, 0x58,
							0x4A, 0xDE, 0x0C, 0x01, 0x2B, 0x85, 0x2D, 0xE5,
							0x13, 0x22, 0x48, 0xB6,	0xF3, 0x2D, 0x00, 0x9A
						};
						*/
						int x;
						UINT16 *PROTROM = (UINT16*)memory_region(REGION_USER1);

						for (x = 0; x < size; x++)
						{
				
							UINT16 dat2 = PROTROM[src + x];

							UINT8 extraoffset = param&0xfe; // the lowest bit changed the table addressing in tests, see 'rawDataOdd' table instead.. it's still related to the main one, not identical
							UINT8* dectable = (UINT8*)memory_region(REGION_USER1);//rawDataEven; // the basic decryption table is at the start of the mcu data rom! at least in killbld
							UINT16 extraxor = ((dectable[((x*2)+0+extraoffset)&0xff]) << 8) | (dectable[((x*2)+1+extraoffset)&0xff] << 0);
							
							dat2 = ((dat2 & 0x00ff)<<8) | ((dat2 & 0xff00)>>8);
							
							if (mode==3) dat2 ^= extraxor;						
							if (mode==2) dat2 += extraxor;
							if (mode==1) dat2 -= extraxor;
							
							/*if (dat!=dat2)
								printf("Mode %04x Param %04x Mismatch %04x %04x\n", mode, param, dat, dat2);
							*/

							killbld_sharedprotram[dst + x] = dat2;
						}
	
						/* hack, patches out some additional security checks... we need to emulate them instead!
						  they occur before it displays the disclaimer, so if you remove the overlay patches it will display
						  the highscore table before coming up with this error... */
						if ((mode==3) && (param==0x54) && (src*2==0x2120) && (dst*2==0x2600)) killbld_sharedprotram[0x2600 / 2] = 0x4e75;
						
					}
					if (mode == 4)
					{
						printf("unhandled copy mode %04x!\n", mode);
						/* not used by killing blade */
						/* looks almost like a fixed value xor, but isn't */
					}
					else if (mode == 5)
					{
						/* mode 5 seems to be a straight copy */
						int x;
						UINT16 *PROTROM = (UINT16*)memory_region(REGION_USER1);
						for (x = 0; x < size; x++)
						{
							UINT16 dat = PROTROM[src + x];


							killbld_sharedprotram[dst + x] = dat;
						}
					}
					else if (mode == 6)
					{
						/* mode 6 seems to swap bytes and nibbles */
						int x;
						UINT16 *PROTROM = (UINT16*)memory_region(REGION_USER1);
						for (x = 0; x < size; x++)
						{
							UINT16 dat = PROTROM[src + x];

							dat = ((dat & 0xf000) >> 12)|
								  ((dat & 0x0f00) >> 4)|
								  ((dat & 0x00f0) << 4)|
								  ((dat & 0x000f) << 12);

							killbld_sharedprotram[dst + x] = dat;
						}
					}
					else if (mode == 7)
					{
						printf("unhandled copy mode %04x!\n", mode);
						/* not used by killing blade */
						/* weird mode, the params get left in memory? - maybe it's a NOP? */
					}
					else
					{
						printf("unhandled copy mode %04x!\n", mode);
						/* not used by killing blade */
						/* invalid? */

					}

				}
				kb_reg++;
			}
		}
		else if (kb_cmd == 4)
			kb_ptr = data;
		else if (kb_cmd == 0x20)
			kb_ptr++;
	}
}

static READ16_HANDLER( killbld_prot_r )
{

	UINT16 res ;

	offset &= 0xf;
	res = 0;

	if (offset == 1)
	{
		if (kb_cmd == 1)
		{
			res = kb_reg & 0x7f;
		}
		else if (kb_cmd == 5)
		{
			UINT32 protvalue = 0x89911400 | readinputport(4);
			res = (protvalue >> (8 * (kb_ptr - 1))) & 0xff;
		}
	}
	logerror("%06X: ASIC25 R CMD %X  VAL %X\n", activecpu_get_pc(),kb_cmd,res);
	return res;
}

static MACHINE_INIT( killbld )
{
	int i;

	machine_init_pgm();

	/* fill the protection ram with a5 */
	for (i = 0; i < 0x4000/2; i++)
		killbld_sharedprotram[i] = 0xa5a5;
}


/* ASIC025/ASIC022 don't provide rom patches like the DW2 protection does, the previous dump was bad :-) */
static DRIVER_INIT( killbld )
{

	pgm_basic_init();
	pgm_killbld_decrypt();

	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0xd40000, 0xd40003, 0, 0, killbld_prot_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0xd40000, 0xd40003, 0, 0, killbld_prot_w);

	kb_cmd = 0;
	kb_reg = 0;
	kb_ptr = 0;
	memset(kb_regs, 0, 0x10);

}

extern READ16_HANDLER( PSTARS_protram_r );
extern READ16_HANDLER( PSTARS_r16 );
extern WRITE16_HANDLER( PSTARS_w16 );

static DRIVER_INIT( pstar )
{
	pgm_basic_init();
 	pgm_pstar_decrypt();

	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x4f0000, 0x4f0025, 0, 0, PSTARS_protram_r);
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500003, 0, 0, PSTARS_r16);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500005, 0, 0, PSTARS_w16);
}

/* olds */
int           olds_cmd;
int           olds_reg;
int           olds_ptr;
UINT16        olds_bs;
UINT16        olds_cmd3;
UINT16        olds_prot_hold;
UINT16        olds_prot_hilo;
UINT16        olds_prot_hilo_select;
const UINT8  *olds_prot_hilo_source2;
UINT32 olds_prot_addr( UINT16 addr );
UINT32 olds_read_reg( UINT16 addr );
void olds_write_reg( UINT16 addr, UINT32 val );
void IGS028_do_dma(UINT16 src, UINT16 dst, UINT16 size, UINT16 mode);
void olds_protection_calculate_hilo();
void olds_protection_calculate_hold(int y, int z);
READ16_HANDLER( olds_r );
WRITE16_HANDLER( olds_w );
// tables are xored by table at $1998dc
// tables are the same as drgw3 and drgw2
static const UINT8 olds_source_data[8][0xec] = // table addresses $2951CA
{
	{ // region 0, unused...
		0,
	},
	{ // region 1, $1A669A
		0x67, 0x51, 0xf3, 0x19, 0xa0, 0x11, 0xe1, 0x11, 0x10, 0xee, 0xe3, 0xf6, 0xbe, 0x81, 0x35, 0xe3,
		0xfb, 0xe6, 0xef, 0xdf, 0x61, 0x01, 0xfa, 0x22, 0x5d, 0x43, 0x01, 0xa5, 0x3b, 0x17, 0xd4, 0x74,
		0xf0, 0xf4, 0xf3, 0x43, 0xb5, 0x19, 0x04, 0xd5, 0x84, 0xce, 0x87, 0xfe, 0x35, 0x3e, 0xc4, 0x3c,
		0xc7, 0x85, 0x2a, 0x33, 0x00, 0x86, 0xd0, 0x4d, 0x65, 0x4b, 0xf9, 0xe9, 0xc0, 0xba, 0xaa, 0x77,
		0x9e, 0x66, 0xf6, 0x0f, 0x4f, 0x3a, 0xb6, 0xf1, 0x64, 0x9a, 0xe9, 0x25, 0x1a, 0x5f, 0x22, 0xa3,
		0xa2, 0xbf, 0x4b, 0x77, 0x3f, 0x34, 0xc9, 0x6e, 0xdb, 0x12, 0x5c, 0x33, 0xa5, 0x8b, 0x6c, 0xb1,
		0x74, 0xc8, 0x40, 0x4e, 0x2f, 0xe7, 0x46, 0xae, 0x99, 0xfc, 0xb0, 0x55, 0x54, 0xdf, 0xa7, 0xa1,
		0x0f, 0x5e, 0x49, 0xcf, 0x56, 0x3c, 0x90, 0x2b, 0xac, 0x65, 0x6e, 0xdb, 0x58, 0x3e, 0xc9, 0x00,
		0xae, 0x53, 0x4d, 0x92, 0xfa, 0x40, 0xb2, 0x6b, 0x65, 0x4b, 0x90, 0x8a, 0x0c, 0xe2, 0xa5, 0x9a,
		0xd0, 0x20, 0x29, 0x55, 0xa4, 0x44, 0xac, 0x51, 0x87, 0x54, 0x53, 0x34, 0x24, 0x4b, 0x81, 0x67,
		0x34, 0x4c, 0x5f, 0x31, 0x4e, 0xf2, 0xf1, 0x19, 0x18, 0x1c, 0x34, 0x38, 0xe1, 0x81, 0x17, 0xcf,
		0x24, 0xb9, 0x9a, 0xcb, 0x34, 0x51, 0x50, 0x59, 0x44, 0xb1, 0x0b, 0x50, 0x95, 0x6c, 0x48, 0x7e,
		0x14, 0xa4, 0xc6, 0xd9, 0xd3, 0xa5, 0xd6, 0xd0, 0xc5, 0x97, 0xf0, 0x45, 0xd0, 0x98, 0x51, 0x91,
		0x9f, 0xa3, 0x43, 0x51, 0x05, 0x90, 0xee, 0xca, 0x7e, 0x5f, 0x72, 0x53, 0xb1, 0xd3, 0xaf, 0x36,
		0x08, 0x75, 0xb0, 0x9b, 0xe0, 0x0d, 0x43, 0x88, 0xaa, 0x27, 0x44, 0x11
	},
	{ // region 2, $19A5F8
		0xf9, 0x19, 0xf3, 0x09, 0xa0, 0x11, 0xe0, 0x11, 0x10, 0x22, 0xfd, 0x8e, 0xd3, 0xc8, 0x31, 0x67,
		0xc0, 0x10, 0x3c, 0xc2, 0x03, 0xf2, 0x6a, 0x0a, 0x54, 0x49, 0xca, 0xb5, 0x4b, 0xe0, 0x94, 0xe8,
		0x8d, 0xc8, 0x90, 0xee, 0x6b, 0x6f, 0xfa, 0x09, 0x76, 0x84, 0x6f, 0x55, 0xd1, 0x94, 0xca, 0x9c,
		0xe1, 0x22, 0xc6, 0x02, 0xb5, 0x8c, 0xf9, 0x3a, 0x52, 0x10, 0xf0, 0x22, 0xe4, 0x11, 0x15, 0x73,
		0x5e, 0x9e, 0xde, 0xc4, 0x5a, 0xbd, 0xa3, 0x89, 0xe7, 0x9b, 0x95, 0x5d, 0x75, 0xf6, 0xc3, 0x9f,
		0xe4, 0xcf, 0x65, 0x73, 0x90, 0xd0, 0x75, 0x56, 0xfa, 0xcc, 0xe4, 0x3e, 0x9c, 0x41, 0x81, 0x62,
		0xb1, 0xd3, 0x28, 0xbd, 0x6c, 0xed, 0x60, 0x28, 0x27, 0xee, 0xf2, 0xa1, 0xb4, 0x2c, 0x6c, 0xbb,
		0x42, 0xd7, 0x1d, 0x62, 0xc0, 0x33, 0x7d, 0xf9, 0xe4, 0x5c, 0xe2, 0x41, 0xa4, 0x1c, 0x98, 0xa1,
		0x87, 0x95, 0xad, 0x61, 0x56, 0x96, 0x40, 0x08, 0x6b, 0xe2, 0x4b, 0x95, 0x7b, 0x1b, 0xd8, 0x64,
		0xb3, 0xee, 0x9d, 0x79, 0x69, 0xea, 0x5d, 0xcf, 0x01, 0x91, 0xea, 0x3f, 0x70, 0x29, 0xdc, 0xe0,
		0x08, 0x20, 0xbf, 0x46, 0x90, 0xa8, 0xfc, 0x29, 0x14, 0xd1, 0x0d, 0x20, 0x79, 0xd2, 0x2c, 0xe9,
		0x52, 0xa6, 0x8c, 0xbd, 0xa3, 0x3e, 0x88, 0x2d, 0xb8, 0x4e, 0xf2, 0x74, 0x50, 0xcc, 0x12, 0xde,
		0xd3, 0x5a, 0xa4, 0x7b, 0xa2, 0x8d, 0x91, 0x68, 0x12, 0x0c, 0x9c, 0xb9, 0x6d, 0x26, 0x66, 0x60,
		0xc3, 0x6d, 0xd0, 0x11, 0x33, 0x05, 0x1d, 0xa8, 0xb6, 0x51, 0xe6, 0xe0, 0x58, 0x61, 0x74, 0x37,
		0xcc, 0x3a, 0x4d, 0x6a, 0x0a, 0x09, 0x71, 0xe3, 0x7e, 0xa5, 0x3b, 0xe9
	},
	{ // region 3, $1F9508
		0x73, 0x59, 0xf3, 0x09, 0xa0, 0x11, 0xe1, 0x11, 0x10, 0x55, 0x18, 0x0d, 0xe8, 0x29, 0x2d, 0x04,
		0x85, 0x39, 0x88, 0xbe, 0x8b, 0xcb, 0xd9, 0x0b, 0x32, 0x36, 0x94, 0xac, 0x74, 0xc3, 0x3b, 0x5d,
		0x2a, 0x83, 0x46, 0xb3, 0x3a, 0xac, 0xd8, 0x55, 0x68, 0x21, 0x57, 0xab, 0x6e, 0xd1, 0xd0, 0xfc,
		0xe2, 0xbe, 0x63, 0xd0, 0x6b, 0x79, 0x23, 0x40, 0x58, 0xd4, 0xe7, 0x73, 0x22, 0x67, 0x7f, 0x88,
		0x05, 0xbd, 0xdf, 0x7a, 0x65, 0x41, 0x90, 0x3a, 0x52, 0x83, 0x28, 0xae, 0xe9, 0x8e, 0x65, 0x82,
		0x0e, 0xdf, 0x98, 0x88, 0xe1, 0x86, 0x21, 0x3e, 0x1a, 0x87, 0x6d, 0x62, 0x7a, 0xf6, 0xaf, 0x2c,
		0xd5, 0xc5, 0x10, 0x2d, 0xa9, 0xda, 0x93, 0xa1, 0x9b, 0xc7, 0x35, 0xd4, 0x15, 0x78, 0x18, 0xd5,
		0x75, 0x6a, 0xd7, 0xdb, 0x12, 0x2a, 0x6a, 0xc8, 0x36, 0x53, 0x57, 0xa6, 0xf0, 0x13, 0x67, 0x43,
		0x79, 0xf0, 0x0e, 0x49, 0xb1, 0xec, 0xcd, 0xa4, 0x8a, 0x61, 0x06, 0xb9, 0xea, 0x53, 0xf2, 0x47,
		0x7d, 0xd6, 0xf8, 0x9d, 0x2e, 0xaa, 0x27, 0x35, 0x61, 0xce, 0x9b, 0x63, 0xbc, 0x07, 0x51, 0x5a,
		0xc2, 0x0d, 0x39, 0x42, 0xd2, 0x5e, 0x21, 0x20, 0x10, 0xa0, 0xe5, 0x08, 0xf7, 0x3d, 0x28, 0x04,
		0x99, 0x93, 0x97, 0xaf, 0xf9, 0x12, 0xc0, 0x01, 0x2d, 0xea, 0xf3, 0x98, 0x0b, 0x46, 0xc2, 0x26,
		0x93, 0x10, 0x69, 0x1d, 0x71, 0x8e, 0x33, 0x00, 0x5e, 0x80, 0x2f, 0x47, 0x0a, 0xcc, 0x94, 0x16,
		0xe7, 0x37, 0x45, 0xd0, 0x61, 0x79, 0x32, 0x86, 0x08, 0x2a, 0x5b, 0x55, 0xfe, 0xee, 0x52, 0x38,
		0xaa, 0x18, 0xe9, 0x39, 0x1a, 0x1e, 0xb8, 0x26, 0x6b, 0x3d, 0x4b, 0xa9
	},
	{ // region 4, $1CA7B8
		0x06, 0x01, 0xf3, 0x39, 0xa0, 0x11, 0xf0, 0x11, 0x10, 0x6f, 0x32, 0x8b, 0xfd, 0x89, 0x29, 0xa0,
		0x4a, 0x62, 0xed, 0xa1, 0x2d, 0xa4, 0x49, 0xf2, 0x10, 0x3c, 0x77, 0xa3, 0x84, 0x8d, 0xfa, 0xd1,
		0xc6, 0x57, 0xe2, 0x78, 0xef, 0xe9, 0xb6, 0xa1, 0x5a, 0xbd, 0x3f, 0x02, 0x0b, 0x28, 0xd6, 0x76,
		0xfc, 0x5b, 0x19, 0x9f, 0x21, 0x66, 0x4c, 0x2d, 0x45, 0x99, 0xde, 0xab, 0x46, 0xbd, 0xe9, 0x84,
		0xc4, 0xdc, 0xc7, 0x30, 0x70, 0xdd, 0x64, 0xea, 0xbc, 0x6b, 0xd3, 0xe6, 0x45, 0x3f, 0x07, 0x7e,
		0x50, 0xef, 0xb2, 0x84, 0x33, 0x3c, 0xcc, 0x3f, 0x39, 0x5b, 0xf5, 0x6d, 0x71, 0xc5, 0xdd, 0xf5,
		0xf9, 0xd0, 0xf7, 0x9c, 0xe6, 0xc7, 0xad, 0x1b, 0x29, 0xb9, 0x90, 0x08, 0x75, 0xc4, 0xc3, 0xef,
		0xa8, 0xfc, 0xab, 0x55, 0x7c, 0x21, 0x57, 0x97, 0x87, 0x4a, 0xcb, 0x0c, 0x56, 0x0a, 0x4f, 0xcb,
		0x52, 0x33, 0x87, 0x31, 0xf3, 0x43, 0x5b, 0x41, 0x90, 0xf8, 0xc0, 0xdd, 0x5a, 0xa4, 0x26, 0x2a,
		0x60, 0xa5, 0x6d, 0xda, 0xf2, 0x6a, 0xf0, 0xb3, 0xda, 0x25, 0x33, 0x87, 0x22, 0xe4, 0xac, 0xd3,
		0x96, 0xe0, 0x99, 0x3e, 0xfb, 0x14, 0x45, 0x17, 0x25, 0x56, 0xbe, 0xef, 0x8f, 0x8e, 0x3d, 0x1e,
		0xc7, 0x99, 0xa2, 0xa1, 0x50, 0xfe, 0xdf, 0xd4, 0xa1, 0x87, 0xf4, 0xd5, 0xde, 0xa6, 0x8c, 0x6d,
		0x6c, 0xde, 0x47, 0xbe, 0x59, 0x8f, 0xd4, 0x97, 0xc3, 0xf4, 0xda, 0xbb, 0xa6, 0x73, 0xa9, 0xcb,
		0xf2, 0x01, 0xb9, 0x90, 0x8f, 0xed, 0x60, 0x64, 0x40, 0x1c, 0xb6, 0xc9, 0xa5, 0x7c, 0x17, 0x52,
		0x6f, 0xdc, 0x6d, 0x08, 0x2a, 0x1a, 0xe6, 0x68, 0x3f, 0xd4, 0x42, 0x69
	},
	{ // region 5, $1A19FA
		0x7f, 0x41, 0xf3, 0x39, 0xa0, 0x11, 0xf1, 0x11, 0x10, 0xa2, 0x4c, 0x23, 0x13, 0xe9, 0x25, 0x3d,
		0x0f, 0x72, 0x3a, 0x9d, 0xb5, 0x96, 0xd1, 0xda, 0x07, 0x29, 0x41, 0x9a, 0xad, 0x70, 0xba, 0x46,
		0x63, 0x2b, 0x7f, 0x3d, 0xbe, 0x40, 0xad, 0xd4, 0x4c, 0x73, 0x27, 0x58, 0xa7, 0x65, 0xdc, 0xd6,
		0xfd, 0xde, 0xb5, 0x6e, 0xd6, 0x6c, 0x75, 0x1a, 0x32, 0x45, 0xd5, 0xe3, 0x6a, 0x14, 0x6d, 0x80,
		0x84, 0x15, 0xaf, 0xcc, 0x7b, 0x61, 0x51, 0x82, 0x40, 0x53, 0x7f, 0x38, 0xa0, 0xd6, 0x8f, 0x61,
		0x79, 0x19, 0xe5, 0x99, 0x84, 0xd8, 0x78, 0x27, 0x3f, 0x16, 0x97, 0x78, 0x4f, 0x7b, 0x0c, 0xa6,
		0x37, 0xdb, 0xc6, 0x0c, 0x24, 0xb4, 0xc7, 0x94, 0x9d, 0x92, 0xd2, 0x3b, 0xd5, 0x11, 0x6f, 0x0a,
		0xdb, 0x76, 0x66, 0xe7, 0xcd, 0x18, 0x2b, 0x66, 0xd8, 0x41, 0x40, 0x58, 0xa2, 0x01, 0x1e, 0x6d,
		0x44, 0x75, 0xe7, 0x19, 0x4f, 0xb2, 0xe8, 0xc4, 0x96, 0x77, 0x62, 0x02, 0xc9, 0xdc, 0x59, 0xf3,
		0x43, 0x8d, 0xc8, 0xfe, 0x9e, 0x2a, 0xba, 0x32, 0x3b, 0x62, 0xe3, 0x92, 0x6e, 0xc2, 0x08, 0x4d,
		0x51, 0xcd, 0xf9, 0x3a, 0x3e, 0xc9, 0x50, 0x27, 0x21, 0x25, 0x97, 0xd7, 0x0e, 0xf8, 0x39, 0x38,
		0xf5, 0x86, 0x94, 0x93, 0xbf, 0xeb, 0x18, 0xa8, 0xfc, 0x24, 0xf5, 0xf9, 0x99, 0x20, 0x3d, 0xcd,
		0x2c, 0x94, 0x25, 0x79, 0x28, 0x77, 0x8f, 0x2f, 0x10, 0x69, 0x86, 0x30, 0x43, 0x01, 0xd7, 0x9a,
		0x17, 0xe3, 0x47, 0x37, 0xbd, 0x62, 0x75, 0x42, 0x78, 0xf4, 0x2b, 0x57, 0x4c, 0x0a, 0xdb, 0x53,
		0x4d, 0xa1, 0x0a, 0xd6, 0x3a, 0x16, 0x15, 0xaa, 0x2c, 0x6c, 0x39, 0x42
	},
	{ // region 6, $2937EA
		0x12, 0x09, 0xf3, 0x29, 0xa0, 0x11, 0xf0, 0x11, 0x10, 0xd5, 0x66, 0xa1, 0x28, 0x4a, 0x21, 0xc0,
		0xd3, 0x9b, 0x86, 0x80, 0x57, 0x6f, 0x41, 0xc2, 0xe4, 0x2f, 0x0b, 0x91, 0xbd, 0x3a, 0x7a, 0xba,
		0x00, 0xe5, 0x35, 0x02, 0x74, 0x7d, 0x8b, 0x21, 0x57, 0x10, 0x0f, 0xae, 0x44, 0xbb, 0xe2, 0x37,
		0x18, 0x7b, 0x52, 0x3d, 0x8c, 0x59, 0x9e, 0x20, 0x1f, 0x0a, 0xcc, 0x1c, 0x8e, 0x6a, 0xd7, 0x95,
		0x2b, 0x34, 0xb0, 0x82, 0x6d, 0xfd, 0x25, 0x33, 0xaa, 0x3b, 0x2b, 0x70, 0x15, 0x87, 0x31, 0x5d,
		0xbb, 0x29, 0x19, 0x95, 0xd5, 0x8e, 0x24, 0x28, 0x5e, 0xd0, 0x20, 0x83, 0x46, 0x4a, 0x21, 0x70,
		0x5b, 0xcd, 0xae, 0x7b, 0x61, 0xa1, 0xfa, 0xf4, 0x2b, 0x84, 0x15, 0x6e, 0x36, 0x5d, 0x1b, 0x24,
		0x0f, 0x09, 0x3a, 0x61, 0x38, 0x0f, 0x18, 0x35, 0x11, 0x38, 0xb4, 0xbd, 0xee, 0xf7, 0xec, 0x0f,
		0x1d, 0xb7, 0x48, 0x01, 0xaa, 0x09, 0x8f, 0x61, 0xb5, 0x0f, 0x1d, 0x26, 0x39, 0x2e, 0x8c, 0xd6,
		0x26, 0x5c, 0x3d, 0x23, 0x63, 0xe9, 0x6b, 0x97, 0xb4, 0x9f, 0x7b, 0xb6, 0xba, 0xa0, 0x7c, 0xc6,
		0x25, 0xa1, 0x73, 0x36, 0x67, 0x7f, 0x74, 0x1e, 0x1d, 0xda, 0x70, 0xbf, 0xa5, 0x63, 0x35, 0x39,
		0x24, 0x8c, 0x9f, 0x85, 0x16, 0xd8, 0x50, 0x95, 0x71, 0xc0, 0xf6, 0x1e, 0x6d, 0x80, 0xed, 0x15,
		0xeb, 0x63, 0xe9, 0x1b, 0xf6, 0x78, 0x31, 0xc6, 0x5c, 0xdd, 0x19, 0xbd, 0xdf, 0xa7, 0xec, 0x50,
		0x22, 0xad, 0xbb, 0xf6, 0xeb, 0xd6, 0xa3, 0x20, 0xc9, 0xe6, 0x9f, 0xcb, 0xf2, 0x97, 0xb9, 0x54,
		0x12, 0x66, 0xa6, 0xbe, 0x4a, 0x12, 0x43, 0xec, 0x00, 0xea, 0x49, 0x02
	},
	{ // region 7, $255E8C
		0xa4, 0x49, 0xf3, 0x29, 0xa0, 0x11, 0xf1, 0x11, 0x10, 0xef, 0x80, 0x20, 0x3d, 0xaa, 0x36, 0x5d,
		0x98, 0xc4, 0xd2, 0x63, 0xdf, 0x61, 0xb0, 0xc3, 0xc2, 0x35, 0xd4, 0x88, 0xe6, 0x1d, 0x3a, 0x2f,
		0x9c, 0xb9, 0xd1, 0xc6, 0x43, 0xba, 0x69, 0x6d, 0x49, 0xac, 0xdd, 0x05, 0xe0, 0xf8, 0xe8, 0x97,
		0x19, 0x18, 0x08, 0x0c, 0x42, 0x46, 0xc7, 0x0d, 0x25, 0xce, 0xc3, 0x54, 0xb2, 0xd9, 0x42, 0x91,
		0xea, 0x53, 0x98, 0x38, 0x78, 0x81, 0x12, 0xca, 0x15, 0x23, 0xbd, 0xc1, 0x70, 0x1f, 0xd2, 0x40,
		0xfd, 0x39, 0x33, 0xaa, 0x27, 0x2b, 0xe8, 0x10, 0x7d, 0xa4, 0xa8, 0x8e, 0x3d, 0x00, 0x4f, 0x3a,
		0x7f, 0xd8, 0x96, 0xea, 0x9e, 0x8e, 0x15, 0x6e, 0x9f, 0x76, 0x57, 0xba, 0x7d, 0xc2, 0xdf, 0x57,
		0x42, 0x82, 0xf4, 0xda, 0x89, 0x06, 0x05, 0x04, 0x62, 0x2f, 0x29, 0x23, 0x54, 0xd5, 0xbb, 0x97,
		0xf5, 0xf9, 0xc1, 0xcf, 0xec, 0x5f, 0x1d, 0xfd, 0xbb, 0xa6, 0xd7, 0x4a, 0xa8, 0x66, 0xbf, 0xb9,
		0x09, 0x44, 0xb1, 0x60, 0x28, 0xa9, 0x35, 0x16, 0x15, 0xf5, 0x13, 0xc1, 0x07, 0x7e, 0xd7, 0x40,
		0xdf, 0x8e, 0xd3, 0x32, 0xa9, 0x35, 0x98, 0x15, 0x32, 0xa9, 0x49, 0xc0, 0x24, 0xb4, 0x4a, 0x53,
		0x6b, 0x79, 0xaa, 0x77, 0x6c, 0xc5, 0x88, 0x69, 0xe5, 0x5d, 0xde, 0x42, 0x28, 0xf9, 0xb7, 0x5c,
		0xab, 0x19, 0xc7, 0xbc, 0xc5, 0x60, 0xeb, 0x5e, 0xa8, 0x52, 0xc4, 0x32, 0x7c, 0x35, 0x02, 0x06,
		0x46, 0x77, 0x30, 0xb6, 0x33, 0x4b, 0xb8, 0xfd, 0x02, 0xd8, 0x14, 0x40, 0x99, 0x25, 0x7e, 0x55,
		0xd6, 0x44, 0x43, 0x8d, 0x73, 0x0e, 0x71, 0x48, 0xd3, 0x82, 0x40, 0xda
	}
};

UINT32 olds_prot_addr(UINT16 addr)
{
	switch (addr & 0xff)
	{
		case 0x0:
		case 0x5:
		case 0xa: return 0x402a00 + ((addr >> 8) << 2);
		case 0x2:
		case 0x8: return 0x402e00 + ((addr >> 8) << 2);
		case 0x1: return 0x40307e;
		case 0x3: return 0x403090;
		case 0x4: return 0x40309a;
		case 0x6: return 0x4030a4;
		case 0x7: return 0x403000;
		case 0x9: return 0x40306e;
	}

	return 0;
}

UINT32 olds_read_reg(UINT16 addr)
{
	UINT32 protaddr = (olds_prot_addr(addr) - 0x400000) / 2;
	return sharedprotram[protaddr] << 16 | sharedprotram[protaddr + 1];
}

void olds_write_reg( UINT16 addr, UINT32 val )
{
	sharedprotram[((olds_prot_addr(addr) - 0x400000) / 2) + 0] = val >> 16;
	sharedprotram[((olds_prot_addr(addr) - 0x400000) / 2) + 1] = val & 0xffff;
}

void IGS028_do_dma(UINT16 src, UINT16 dst, UINT16 size, UINT16 mode)
{
	UINT16 param = mode >> 8;
	UINT16 *PROTROM = (UINT16*)memory_region(REGION_USER1);

//	logerror ("mode: %2.2x, src: %4.4x, dst: %4.4x, size: %4.4x, data: %4.4x\n", (mode &0xf), src, dst, size, mode);

	mode &= 0x0f;

	switch (mode)
	{
		case 0x00: // This mode copies code later on in the game! Encrypted somehow?
		// src:2fc8, dst: 045a, size: 025e, mode: 0000
		// jumps from 12beb4 in unprotected set
		// jumps from 1313e0 in protected set
		case 0x01: // swap bytes and nibbles
		case 0x02: // ^= encryption
		case 0x05: // copy
		case 0x06: // += encryption (correct?)
		{
			UINT8 extraoffset = param & 0xff;
			UINT8 *dectable = (UINT8 *)(PROTROM + (0x100 / 2));

			INT32 x = 0;
			for (x = 0; x < size; x++)
			{
				UINT16 dat2 = PROTROM[src + x];

				int taboff = ((x*2)+extraoffset) & 0xff; // must allow for overflow in instances of odd offsets
				unsigned short extraxor = ((dectable[taboff + 0]) << 0) | (dectable[taboff + 1] << 8);

				if (mode==0) dat2 = 0x4e75; // hack
				if (mode==1) dat2  = ((dat2 & 0xf000) >> 12) | ((dat2 & 0x0f00) >> 4) | ((dat2 & 0x00f0) << 4) | ((dat2 & 0x000f) << 12);
				if (mode==2) dat2 ^= extraxor;
			//	if (mode==5) dat2  = dat2;
				if (mode==6) dat2 += extraxor;

				if (mode==2 || mode==6) dat2 = (dat2<<8)|(dat2>>8);

				sharedprotram[dst + x] = (dat2 << 8) | (dat2 >> 8);
			}
		}
		break;

	//	default:
	//		logerror ("DMA mode unknown!!!\nsrc:%4.4x, dst: %4.4x, size: %4.4x, mode: %4.4x\n", src, dst, size, mode);
	}
}

void olds_protection_calculate_hold(int y, int z) // calculated in routine $12dbc2 in olds
{
	unsigned short old = olds_prot_hold;

	olds_prot_hold = ((old << 1) | (old >> 15));

	olds_prot_hold ^= 0x2bad;
	olds_prot_hold ^= BIT(z, y);
	olds_prot_hold ^= BIT( old,  7) <<  0;
	olds_prot_hold ^= BIT(~old, 13) <<  4;
	olds_prot_hold ^= BIT( old,  3) << 11;

	olds_prot_hold ^= (olds_prot_hilo & ~0x0408) << 1; // $81790c
}

void olds_protection_calculate_hilo() // calculated in routine $12dbc2 in olds
{
	UINT8 source;

	olds_prot_hilo_select++;
	if (olds_prot_hilo_select > 0xeb) {
		olds_prot_hilo_select = 0;
	}

	source = olds_source_data[readinputport(4)][olds_prot_hilo_select];

	if (olds_prot_hilo_select & 1)    // $8178fa
	{
		olds_prot_hilo = (olds_prot_hilo & 0x00ff) | (source << 8);     // $8178d8
	}
	else
	{
		olds_prot_hilo = (olds_prot_hilo & 0xff00) | (source << 0);     // $8178d8
	}
}

WRITE16_HANDLER(olds_w )
{
	if (offset == 0)
	{
		olds_cmd = data;
	}
	else
	{
		switch (olds_cmd)
		{
			case 0x00:
				olds_reg = data;
			break;

			case 0x02:
				olds_bs = ((data & 0x03) << 6) | ((data & 0x04) << 3) | ((data & 0x08) << 1);
			break;

			case 0x03:
			{
				UINT16 cmd = sharedprotram[0x3026 / 2];

			//  logerror ("command: %x\n", cmd);

				switch (cmd)
				{
					case 0x12:
					{
						UINT16 mode = sharedprotram[0x303e / 2];  // ?
						UINT16 src  = sharedprotram[0x306a / 2] >> 1; // ?
						UINT16 dst  = sharedprotram[0x3084 / 2] & 0x1fff;
						UINT16 size = sharedprotram[0x30a2 / 2] & 0x1fff;

						IGS028_do_dma(src, dst, size, mode);
					}
					break;

					case 0x64: // incomplete?
					{
					        UINT16 p1 = sharedprotram[0x3050 / 2];
					        UINT16 p2 = sharedprotram[0x3082 / 2];
					        UINT16 p3 = sharedprotram[0x3054 / 2];
					        UINT16 p4 = sharedprotram[0x3088 / 2];

					        if (p2  == 0x02)
					                olds_write_reg(p1, olds_read_reg(p1) + 0x10000);

					        switch (p4)
					        {
					                case 0xd:
					                        olds_write_reg(p1,olds_read_reg(p3));
					                        break;
					                case 0x0:
					                        olds_write_reg(p3,(olds_read_reg(p2))^(olds_read_reg(p1)));
					                        break;
					                case 0xe:
					                        olds_write_reg(p3,olds_read_reg(p3)+0x10000);
					                        break;
					                case 0x2:
					                        olds_write_reg(p1,(olds_read_reg(p2))+(olds_read_reg(p3)));
					                        break;
					                case 0x6:
					                        olds_write_reg(p3,(olds_read_reg(p2))&(olds_read_reg(p1)));
					                        break;
					                case 0x1:
					                        olds_write_reg(p2,olds_read_reg(p1)+0x10000);
					                        break;
					                case 0x7:
					                        olds_write_reg(p3,olds_read_reg(p1));
					                        break;
					                default:
					                        break;
					        }
					}
					break;

				//  default:
				//      logerror ("unemulated command!\n");
				}

				olds_cmd3 = ((data >> 4) + 1) & 0x3;
			}
			break;

			case 0x04:
				olds_ptr = data;
			break;

			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
				olds_ptr++;
				olds_protection_calculate_hold(olds_cmd & 0x0f, data & 0xff);
			break;

		//	default:
		//		logerror ("unemulated write mode!\n");
		}
	}
}

READ16_HANDLER(olds_r )
{
	if (offset)
	{
		switch (olds_cmd)
		{
			case 0x01:
				return olds_reg & 0x7f;

			case 0x02:
				return olds_bs | 0x80;

			case 0x03:
				return olds_cmd3;

			case 0x05:
			{
				switch (olds_ptr)
				{
					case 1: return 0x3f00 | readinputport(4);

					case 2:
						return 0x3f00 | 0x00;

					case 3:
						return 0x3f00 | 0x90;

					case 4:
						return 0x3f00 | 0x00;

					case 5:
					default: // >= 5
						return 0x3f00 | BITSWAP8(olds_prot_hold, 5,2,9,7,10,13,12,15);    // $817906
				}
			}

			case 0x40:
				olds_protection_calculate_hilo();
				return 0; // unused?
		}
	}

	return 0;
}


DRIVER_INIT(olds)
{
	pgm_basic_init();

	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0xdcb400, 0xdcb403, 0, 0, olds_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0xdcb400, 0xdcb403, 0, 0, olds_w);

	olds_prot_hold = 0;
	olds_prot_hilo = 0;
	olds_prot_hilo_select = 0;

	olds_cmd = 0;
	olds_reg = 0;
	olds_ptr = 0;
	olds_bs = 0;
	olds_cmd3 = 0;

}

static UINT16 value0, value1, valuekey, ddp3lastcommand;
static UINT32 valueresponse;
int ddp3internal_slot = 0;
UINT32 ddp3slots[0xff];
INT16 thrust;

static WRITE16_HANDLER( ddp3_asic_w )
{

	if (offset == 0)
	{
		value0 = data;
		return;
	}
	else if (offset == 1)
	{
		UINT16 realkey;
		if ((data >> 8) == 0xff)
			valuekey = 0xff00;
		realkey = valuekey >> 8;
		realkey |= valuekey;
		{
			valuekey += 0x0100;
			valuekey &= 0xff00;
			if (valuekey == 0xff00)
				valuekey =  0x0100;
		}
		data ^= realkey;
		value1 = data;
		value0 ^= realkey;

		ddp3lastcommand = value1 & 0xff;

		/* typical frame (ddp3) (all 3 games use only these commands? for the most part of levels espgal just issues 8e)
            vbl
            145f28 command 67
            145f70 command e5
            145f28 command 67
            145f70 command e5
            1460c6 command 40
            145ec0 command 8e
            */

		switch (ddp3lastcommand)
		{
			default:
				//printf("%06x command %02x | %04x\n", cpu_get_pc(space->cpu), ddp3lastcommand, value0);
				valueresponse = 0x880000;
				break;

			case 0x40:
				//printf("%06x command %02x | %04x\n", cpu_get_pc(space->cpu), ddp3lastcommand, value0);
				valueresponse = 0x880000;
				//if (value0 == 0x420) ddp3slots[ddp3internal_slot] -= (thrust);
				//else printf("ignoring 0x40 - %04x %04x %04x\n", ddp3internal_slot, value0, ddp3slots[ddp3internal_slot]);
				ddp3slots[(value0>>10)&0x1F] = (ddp3slots[(value0>>5)&0x1F]+ddp3slots[(value0>>0)&0x1F])&0xffffff;
				
				break;

			case 0x67:
				//printf("%06x command %02x | %04x\n", cpu_get_pc(space->cpu), ddp3lastcommand, value0);
				valueresponse = 0x880000;
				ddp3internal_slot = (value0 & 0xff00)>>8;
				ddp3slots[ddp3internal_slot] = (value0 & 0x00ff) << 16;
				break;

			case 0xe5:
				//printf("%06x command %02x | %04x\n", cpu_get_pc(space->cpu), ddp3lastcommand, value0);
				valueresponse = 0x880000;
				ddp3slots[ddp3internal_slot] |= (value0 & 0xffff);
				if (value0 > 0xf000) thrust = -value0;
				break;

			case 0x8e:
				//valueresponse = 0x880000;
				//valueresponse = 0x880000 | ((UINT32)pgm_mainram[0x0c840/2] << 2) | pgm_mainram[0x0c842/2];
				//printf("%06x command %02x | %08x ** [value respond]\n", cpu_get_pc(space->cpu), ddp3lastcommand, valueresponse);
				valueresponse = ddp3slots[value0 & 0xff];
				break;

			case 0x99: // reset?
			valuekey = 0x100;
				valueresponse = 0x880000;
				break;

		}
	}
	else if (offset==2)
	{

	}

}

static READ16_HANDLER( ddp3_asic_r )
{

	if (offset == 0)
	{
		UINT16 d = valueresponse & 0xffff;
		UINT16 realkey = valuekey >> 8;
		realkey |= valuekey;
		d ^= realkey;

		//printf("ASIC read: 0x%x\n", d);

		return d;

	}
	else if (offset == 1)
	{
		UINT16 d = valueresponse >> 16;
		UINT16 realkey = valuekey >> 8;
		realkey |= valuekey;
		d ^= realkey;

		//printf("ASIC read: 0x%x\n", d);

		return d;

	}
	return 0xffff;
}

static READ16_HANDLER( ddp3_ram_mirror_r )
{
	return 0x0000;
}

void install_asic27a_ddp3(void)
{
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500005, 0, 0, ddp3_asic_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500005, 0, 0, ddp3_asic_w);
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x880000, 0x89ffff, 0, 0, ddp3_ram_mirror_r);
}

void install_asic27a_ket(void)
{
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400005, 0, 0, ddp3_asic_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400005, 0, 0, ddp3_asic_w);
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x880000, 0x89ffff, 0, 0, ddp3_ram_mirror_r);
}

void install_asic27a_espgal(void)
{
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400005, 0, 0, ddp3_asic_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x400000, 0x400005, 0, 0, ddp3_asic_w);
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0x880000, 0x89ffff, 0, 0, ddp3_ram_mirror_r);
}

static void pgm_basic_init_nobank(void)
{
	expand_32x32x5bpp();
	expand_colourdata();
}

static DRIVER_INIT( ddp3 )
{
	pgm_basic_init_nobank();
	pgm_py2k2_decrypt(); // yes, it's the same as photo y2k2
	install_asic27a_ddp3();
}

static DRIVER_INIT( ddp3blk )
{
	UINT16 *rom = (UINT16 *)memory_region(REGION_CPU1);
	pgm_basic_init_nobank();
	pgm_py2k2_decrypt(); // yes, it's the same as photo y2k2
	install_asic27a_ddp3();
	
	rom[0x13C33A/2] = 0x4e71;
	rom[0x13C33C/2] = 0x4e71;
	rom[0x13C348/2] = 0x4e71;
	rom[0x13C34A/2] = 0x4e71;
}

static DRIVER_INIT( ket )
{
	pgm_basic_init_nobank();
	pgm_ket_decrypt();
	install_asic27a_ket();
}

static DRIVER_INIT( espgal )
{
	pgm_basic_init_nobank();
	pgm_espgal_decrypt();
	install_asic27a_espgal();
}

/* Puzzli 2 / Puzzli 2 Super*/
INT32 puzzli_54_trigger;
static int stage;
static int tableoffs;
static int tableoffs2;
static int entries_left;
static int currentcolumn;
static int currentrow;
static int num_entries;
static int full_entry;
static int prev_tablloc;
static int numbercolumns;
static int depth;
UINT16 row_bitmask;
static int hackcount;
static int hackcount2;
static int hack_47_value;
static int hack_31_table_offset;
static int hack_31_table_offset2;
static int p2_31_retcounter;
static int simregion;
static int num_mask_bits;
static int columns;
static int desired_mask;
static int leveldata;
static int rows;
static int realrow;
static int pc;
UINT8 coverage[256]; // coverage is how much of the table we've managed to verify using known facts about the table structure
static int command_31_write_type;
UINT16 level_structure[8][10];
static int puzzli2_take_leveldata_value(UINT8 datvalue);

static int count_bits(UINT16 value)
{
	int count = 0;
	int i = 0;
	for (i=0;i<16;i++)
	{
		int bit = (value >> i) & 1;

		if (bit) count++;
	}

	return count;
}

static int get_position_of_bit(UINT16 value, int bit_wanted)
{
	int count = 0;
	int i=0;
	for (i=0;i<16;i++)
	{
		int bit = (value >> i) & 1;

		if (bit) count++;

		if (count==(bit_wanted+1))
			return i;
	}

	return -1;
}

// should be correct, note each value only appears once
UINT8 puzzli2_level_decode[256] = {
  // 0  ,  1  ,  2  ,  3  ,  4  ,  5   , 6  ,  7  ,  8  ,  9  ,  a  ,  b  ,  c  ,  d  ,  e  ,  f  ,
	0x32, 0x3e, 0xb2, 0x37, 0x31, 0x22, 0xd6, 0x0d, 0x35, 0x5c, 0x8d, 0x3c, 0x7a, 0x5f, 0xd7, 0xac, // 0x0
//   0  ,  0  ,  0  ,  0  ,  0  ,  1  ,  1  ,  0  ,  1  ,  1  ,  0  ,  0  ,  0  ,  0  ,  x  ,  x  , 
    0x53, 0xff, 0xeb, 0x44, 0xe8, 0x11, 0x69, 0x77, 0xd9, 0x34, 0x36, 0x45, 0xa6, 0xe9, 0x1c, 0xc6, // 0x1
//   0  ,  0  ,  x  ,  x  ,  x  ,  0  ,  x  ,  0  ,  x  ,  0  ,  0  ,  0  ,  0  ,  x  ,  0  ,  x  , 
	0x3b, 0xbd, 0xad, 0x2e, 0x18, 0xdf, 0xa1, 0xab, 0xdd, 0x52, 0x57, 0xc2, 0xe5, 0x0a, 0x00, 0x6d, // 0x2
//   0  ,  0  ,  0  ,  1  ,  1  ,  1  ,  1  ,  x  ,  1  ,  1  ,  0  ,  0  ,  1  ,  1  ,  x  ,  0  , 
	0x67, 0x64, 0x15, 0x70, 0xb6, 0x39, 0x27, 0x78, 0x82, 0xd2, 0x71, 0xb9, 0x13, 0xf5, 0x93, 0x92, // 0x3
//   0  ,  x  ,  1  ,  1  ,  x  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  x  ,  x  , 
	0xfa, 0xe7, 0x5e, 0xb0, 0xf6, 0xaf, 0x95, 0x8a, 0x7c, 0x73, 0xf9, 0x63, 0x86, 0xcb, 0x1a, 0x56, // 0x4
//   0  ,  1  ,  1  ,  0  ,  0  ,  0  ,  0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  1  ,  1  ,  1  ,  0  , 
    0xf1, 0x3a, 0xae, 0x61, 0x01, 0x29, 0x97, 0x23, 0x8e, 0x5d, 0x9a, 0x65, 0x74, 0x21, 0x20, 0x40, // 0x5
//   0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  x  ,  x  ,  x  ,  0  ,  0  ,  1  ,  1  ,  1  ,  1  ,  1  , 
    0xd3, 0x05, 0xa2, 0xe1, 0xbc, 0x9e, 0x1e, 0x10, 0x14, 0x0c, 0x88, 0x9c, 0xec, 0x38, 0xb5, 0x9d, // 0x6
//   1  ,  0  ,  0  ,  x  ,  1  ,  1  ,  0  ,  0  ,  x  ,  0  ,  x  ,  0  ,  0  ,  1  ,  1  ,  1  , 
    0x2d, 0xf7, 0x17, 0x0e, 0x84, 0xc7, 0x7d, 0xce, 0x94, 0x16, 0x48, 0xa8, 0x81, 0x6e, 0x7b, 0xd8, // 0x7
//   1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  x  ,  0  ,  1  ,  1  ,  1  ,  x  ,  x  ,  1  ,  1  ,  1  , 
    0xa7, 0x7f, 0x42, 0xe6, 0xa0, 0x2a, 0xef, 0xee, 0x24, 0xba, 0xb8, 0x7e, 0xc9, 0x2b, 0x90, 0xcc, // 0x8
//   1  ,  x  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 
    0x5b, 0xd1, 0xf3, 0xe2, 0x6f, 0xed, 0x9f, 0xf0, 0x4b, 0x54, 0x8c, 0x08, 0xf8, 0x51, 0x68, 0xc8, // 0x9
//   x  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  x  ,  0  ,  x  ,  0  ,  0  ,  0  ,  0  ,  1  , 
    0x03, 0x0b, 0xbb, 0xc1, 0xe3, 0x4d, 0x04, 0xc5, 0x8f, 0x09, 0x0f, 0xbf, 0x62, 0x49, 0x76, 0x59, // 0xa
//   1  ,  1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  1  ,  1  ,  0  ,  x  ,  1  ,  1  ,  1  ,  1  ,  0  , 
    0x1d, 0x80, 0xde, 0x60, 0x07, 0xe0, 0x1b, 0x66, 0xa5, 0xbe, 0xcd, 0x87, 0xdc, 0xc3, 0x6b, 0x4e, // 0xb
//   0  ,  1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  x  ,  0  ,  x  ,  0  ,  1  ,  1  ,  0  ,  1  ,  1  , 
    0xd0, 0xfd, 0xd4, 0x3f, 0x98, 0x96, 0x2f, 0x4c, 0xb3, 0xea, 0x2c, 0x75, 0xe4, 0xc0, 0x6c, 0x6a, // 0xc
//   0  ,  x  ,  1  ,  1  ,  0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  1  ,  1  ,  x  ,  1  ,  1  ,  1  , 
    0x9b, 0xb7, 0x43, 0x8b, 0x41, 0x47, 0x02, 0xdb, 0x99, 0x3d, 0xa3, 0x79, 0x50, 0x4f, 0xb4, 0x55, // 0xd
//   1  ,  0  ,  0  ,  0  ,  1  ,  0  ,  0  ,  x  ,  x  ,  1  ,  1  ,  1  ,  0  ,  1  ,  1  ,  1  ,
    0x5a, 0x25, 0xf4, 0xca, 0x58, 0x30, 0xc4, 0x12, 0xa9, 0x46, 0xda, 0x91, 0xa4, 0xaa, 0xfc, 0x85, // 0xe
//   1  ,  1  ,  0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  0  ,  x  , 
    0xfb, 0x89, 0x06, 0xcf, 0xfe, 0x33, 0xd5, 0x28, 0x1f, 0x19, 0x4a, 0xb1, 0x83, 0xf2, 0x72, 0x26, // 0xf
//   x  ,  x  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  , 
};


//#define printf logerror

static int puzzli2_take_leveldata_value(UINT8 datvalue)
{
	int num_mask_bits;
    int desired_mask;
    int realrow;
	if (stage==-1)
	{
		tableoffs = 0;
		tableoffs2 = 0;
		entries_left = 0;
		currentcolumn = 0;
		currentrow = 0;
		num_entries = 0;
		full_entry = 0;
		prev_tablloc = 0;
		numbercolumns = 0;
		depth = 0;
		row_bitmask = 0;

		printf("%02x <- table offset\n", datvalue);
		tableoffs = datvalue;
		tableoffs2 = 0;
		stage = 0;
	}
	else
	{
	
		UINT8 rawvalue = datvalue;
		UINT8 tableloc = (tableoffs+tableoffs2)&0xff;
		rawvalue ^= puzzli2_level_decode[tableloc];

		tableoffs2++;
		tableoffs2&=0xf;

		if (stage==0)
		{
			stage = 1;

			// this seems to be the first thing returned back when reading the level structure always seems to be 0x8 or 0x7, makes sense, levels would be too difficult otherwise ;-) (actually puzzli2 seems to have one specifying 5 unless it's a decrypt table error?!)
			depth = (rawvalue & 0xf0);
			numbercolumns = (rawvalue & 0x0f);
			numbercolumns++;

			printf("%02x <- Sizes (level depth %01x) (number of columns %01x)", rawvalue, depth>>4, numbercolumns);


			if ((depth != 0x80) && (depth != 0x70) && (depth != 0x50))
				//fatalerror("depth isn't 0x5, 0x7 or 0x8");



			// it seems to use this to specify the number of columns, ie how many data structures follow, so that the ARM knows when to send back the 'finished' flag.
			// it also gets returned at the end with said flag
			if ((numbercolumns != 0x6) && (numbercolumns != 0x7) && (numbercolumns != 0x8))
				//fatalerror("number of columns specified isn't 6,7, or 8");

			printf("\n");

		}
		else if (stage==1)
		{
			printf("%02x <- Number of Entries for this Column (and upper mask) (column is %d) (xor table location is %02x) ", rawvalue, currentcolumn, tableloc);
			stage = 2;
			entries_left = (rawvalue >> 4);
			row_bitmask = (rawvalue & 0x0f)<<8;
			
			full_entry = rawvalue;
			prev_tablloc = tableloc;

			num_entries = entries_left;

			if (num_entries == 0x00)
			{
				printf("0 entries for this column?"); // seems a valid condition based on the data
			}



			printf("\n");

					
		}
		else if (stage==2)
		{	
			printf("%02x <- Mask value equal to number of entries (xor table location is %02x)", rawvalue, tableloc);
			stage = 3;

			row_bitmask |= rawvalue;

			num_mask_bits = count_bits(row_bitmask);

			if (num_mask_bits != num_entries)
				printf(" error - number of mask bits != number of entries - ");

			// 
			if (entries_left == 0)
			{
				// for 0 entries skip back to state 1 instead of 3, because there is nothing following
				stage = 1;
				currentcolumn++;
				currentrow = 0;
				row_bitmask = 0;

				coverage[tableloc] = 1;
				if (rawvalue!=0)
				{
					printf(" invalid mask after 00 length?");
					// attempt to correct the table
					//new_puzzli2_level_decode[tableloc] = new_puzzli2_level_decode[tableloc] ^ rawvalue;

				}
				coverage[prev_tablloc] = 1;
				if (full_entry!=0)
				{
					printf(" previous value wasn't 0x00");
				}

				if (currentcolumn==numbercolumns)
				{
					return 1;
				}
	
			}
			else
			{
				if (num_entries> 0xa)
				{
					printf(" more than 10 entries?");
				}
				else
				{

					// this isn't a strict rule
					// the mask is used so they can specify spaces between elements too without storing the 00 bytes
					coverage[tableloc] = 1;

					desired_mask = 0;

					if (num_entries==0x00) desired_mask = 0x00;
					if (num_entries==0x01) desired_mask = 0x01;
					if (num_entries==0x02) desired_mask = 0x03;
					if (num_entries==0x03) desired_mask = 0x07;
					if (num_entries==0x04) desired_mask = 0x0f;
					if (num_entries==0x05) desired_mask = 0x1f;
					if (num_entries==0x06) desired_mask = 0x3f;
					if (num_entries==0x07) desired_mask = 0x7f;
					if (num_entries==0x08) desired_mask = 0xff;
					if (num_entries==0x09) desired_mask = 0xff;
					if (num_entries==0x0a) desired_mask = 0xff;

					if (rawvalue!=desired_mask)
					{
						printf(" possible wrong mask?");
					}


				}

			}

			printf("\n");

		}
		else if (stage==3)
		{
			UINT16 object_value;

			// return values
			// 0x0100 = normal fish
			// 0x0120 = fish in bubble
			// 0x0140 = fish in egg
			// 0x0160 = buggy fish in egg (displayed as normal fish)
			// 0x0180 = fish on hook
			// 0x01a0 = fish on hook (uncatchable)
			// 0x01c0 - fish on hook (uncatchable)
			// 0x01e0 - fish on hook (uncatchable)

			// fish values
			// 100 101 102 103 104 105 106 107 108 normal
			// 110 - renders as a flashing fish you can't catch? glitches game?
			// 111 - repeat of other fish type you can't catch...

			if (rawvalue<=0x10) // regular fish
			{
				int fishtype = rawvalue;
				printf("%02x <- fish type %d", rawvalue, fishtype);
				object_value = 0x0100 + fishtype; 
				// 0x110 is a flashy fish? might be glitchy and need a special number..
			}
			else if (rawvalue<=0x21) // fish in bubbles
			{
				int fishtype = rawvalue - 0x11;
				printf("%02x <- fish in bubble %d", rawvalue, fishtype);
				object_value = 0x0120 + fishtype; 
				// 0x130 is a flashy fish? might be glitchy and need a special number..
			}
			else if (rawvalue<=0x32) // fish in eggs
			{
				int fishtype = rawvalue - 0x22;
				printf("%02x <- fish in egg %d", rawvalue, fishtype);
				object_value = 0x0140 + fishtype; 
				// 0x150 is a flashy fish? might be glitchy and need a special number..

			}
			else if (rawvalue<=0x43) // fish on hook cases, seem to be base 0x180
			{
				int fishtype = rawvalue - 0x33;
				printf("%02x <- fish on hook %d", rawvalue, fishtype);
				object_value = 0x0180 + fishtype; 
				// 0x190 is a flashy fish? might be glitchy and need a special number..

			}
			////////////////////// special objects follow
			else if (rawvalue==0xd0) {object_value = 0x0200; printf("%02x <- generic bubbles", rawvalue);}

			else if (rawvalue==0xe0) {object_value = 0x8000; printf("%02x <- solid middle", rawvalue);}
			else if (rawvalue==0xe1) {object_value = 0x8020; printf("%02x <- solid top slant down", rawvalue);} // solid slant top down
			else if (rawvalue==0xe2) {object_value = 0x8040; printf("%02x <- solid top slant up", rawvalue);} // solid slant top up
			else if (rawvalue==0xe3) {object_value = 0x8060; printf("%02x <- solid bottom slant up", rawvalue);}
			else if (rawvalue==0xe4) {object_value = 0x8080; printf("%02x <- solid bottom slant down", rawvalue);} // sold slant bottom up


			else                     {object_value = 0xffff; printf("%02x <- unknown object", rawvalue);}

			printf("  (xor table location is %02x)\n",tableloc);

			if (object_value==0xffff)
			{
				object_value = 0x110;
				usrintf_showmessage("unknown object type %02x\n", rawvalue); 
			}

			realrow = get_position_of_bit(row_bitmask, currentrow);

			if (realrow != -1)
				level_structure[currentcolumn][realrow] = object_value;

			currentrow++;

			entries_left--;
			if (entries_left == 0)
			{
				stage = 1;
				currentcolumn++;
				currentrow = 0;
				row_bitmask = 0;

				if (currentcolumn==numbercolumns)
				{
					return 1;
				}

			}
		}
				
	}

	return 0;
}

static WRITE16_HANDLER( puzzli2_asic_w )
{
	int columns = 0;
	int rows = 0;

	if (offset == 0)
	{
		value0 = data;
		return;
	}
	else if (offset == 1)
	{
		UINT16 realkey;
		if ((data >> 8) == 0xff)
			valuekey = 0xff00;
		realkey = valuekey >> 8;
		realkey |= valuekey;
		{
			valuekey += 0x0100;
			valuekey &= 0xff00;
			if (valuekey == 0xff00)
				valuekey =  0x0100;
		}
		data ^= realkey;
		value1 = data;
		value0 ^= realkey;

		ddp3lastcommand = value1 & 0xff;

	switch (ddp3lastcommand)
	{

		case 0x31:
		{
			// how is this selected? command 54?


			/* writes the following sequence before how to play 
			 each level has a different sequence written before it, size of sequence doesn't seem directly connected to level size (unlike the reads)
			 so it's probably compressed somehow as well as scrambled?  68k doesnt know in advance how big each lot of data is either, it only stops
			 writing when it gets a difference response from the MCU.

		
			00138278: 31 00fd | (set xor table offset)
		UNKNOWN - related to depth / number of columns?
			00138278: 31 0087 | value 87, after xor is 75 (table address,value fd,f2)
		COLUMN 1
			00138278: 31 0032 | value 32, after xor is 40 (table address,value fe,72) << 4 is the number of entries in this column
			00138278: 31 0029 | value 29, after xor is 0f (table address,value ff,26) << 0x0f is a mask of 4 bits..

			00138278: 31 0031 | value 31, after xor is 03 (table address,value 00,32)  -> 0x0103
			00138278: 31 003f | value 3f, after xor is 01 (table address,value 01,3e)  -> 0x0101
			00138278: 31 00b0 | value b0, after xor is 02 (table address,value 02,b2)  -> 0x0102
			00138278: 31 0035 | value 35, after xor is 02 (table address,value 03,37)  -> 0x0102
        COLUMN 2
			00138278: 31 0071 | value 71, after xor is 40 (table address,value 04,31) << 4 is the number of entries in this column
			00138278: 31 002d | value 2d, after xor is 0f (table address,value 05,22) << 0x0f is a mask of 4 bits..

			00138278: 31 00d5 | value d5, after xor is 03 (table address,value 06,d6)  -> 0x0103
			00138278: 31 000d | value 0d, after xor is 00 (table address,value 07,0d)  -> 0x0100
			00138278: 31 0034 | value 34, after xor is 01 (table address,value 08,35)  -> 0x0101
			00138278: 31 0059 | value 59, after xor is 05 (table address,value 09,5c)  -> 0x0105
		COLUMN 3
			00138278: 31 00dd | value dd, after xor is 50 (table address,value 0a,8d) << 5 is the number of entries in this column
			00138278: 31 0023 | value 23, after xor is 1f (table address,value 0b,3c) << 0x1f is a mask of 5 bits..
			
			00138278: 31 007a | value 7a, after xor is 00 (table address,value 0c,7a)  -> 0x0100
			00138278: 31 00f3 | value f3, after xor is 01 (table address,value fd,f2)  -> 0x0101
			00138278: 31 0077 | value 77, after xor is 05 (table address,value fe,72)  -> 0x0105
			00138278: 31 0022 | value 22, after xor is 04 (table address,value ff,26)  -> 0x0104
			00138278: 31 0036 | value 36, after xor is 04 (table address,value 00,32)  -> 0x0104	  
		COLUMN 4
			00138278: 31 002e | value 2e, after xor is 10 (table address,value 01,3e) << 1 is the number of entries in this column
			00138278: 31 00b3 | value b3, after xor is 01 (table address,value 02,b2) << 0x01 is a mask of 1 bit..

			00138278: 31 0035 | value 35, after xor is 02 (table address,value 03,37)  -> 0x0102
		COLUMN 5
			00138278: 31 0041 | value 41, after xor is 70 (table address,value 04,31) << 7 is the number of entries in this column
			00138278: 31 005d | value 5d, after xor is 7f (table address,value 05,22) << 0x7f is a mask of 7 bits..
			
			00138278: 31 00d6 | value d6, after xor is 00 (table address,value 06,d6)  -> 0x0100
			00138278: 31 000c | value 0c, after xor is 01 (table address,value 07,0d)  -> 0x0101
			00138278: 31 0036 | value 36, after xor is 03 (table address,value 08,35)  -> 0x0103
			00138278: 31 005e | value 5e, after xor is 02 (table address,value 09,5c)  -> 0x0102
			00138278: 31 0089 | value 89, after xor is 04 (table address,value 0a,8d)  -> 0x0104
			00138278: 31 003c | value 3c, after xor is 00 (table address,value 0b,3c)  -> 0x0100
			00138278: 31 007a | value 7a, after xor is 00 (table address,value 0c,7a)  -> 0x0100
		COLUMN 6
			00138278: 31 00a2 | value a2, after xor is 50 (table address,value fd,f2) << 5 is the number of entries in this column
			00138278: 31 006d | value 6d, after xor is 1f (table address,value fe,72) << 0x1f is a mask of 5 bits..
			
			00138278: 31 0023 | value 23, after xor is 05 (table address,value ff,26)  -> 0x0105
			00138278: 31 0037 | value 37, after xor is 05 (table address,value 00,32)  -> 0x0105
			00138278: 31 003f | value 3f, after xor is 01 (table address,value 01,3e)  -> 0x0101
			00138278: 31 00b3 | value b3, after xor is 01 (table address,value 02,b2)  -> 0x0101
			00138278: 31 0034 | value 34, after xor is 03 (table address,value 03,37)  -> 0x0103
			 ^ (end, returning 630006 as playfield width)

			*/

			if (command_31_write_type==2)
			{
				printf("%08x: %02x %04x | ",pc, ddp3lastcommand, value0);

				// this shouldn't apply to the stuff written on startup, only the level data..

				if (hackcount2==0)
				{
					puzzli2_take_leveldata_value(value0&0xff);

					hack_31_table_offset = value0 & 0xff;
					hack_31_table_offset2 = 0;
					hackcount2++;
					valueresponse = 0x00d20000;

					//printf("(set xor table offset)\n");
				}
				else  // how do we decide end?
				{
					int end = puzzli2_take_leveldata_value(value0&0xff);

					if (!end)
					{

						// always d2 0000 when writing doing level data
						// but different for the writes on startup?
						valueresponse = 0x00d20000;

						//UINT8 tableaddr = (hack_31_table_offset + (hack_31_table_offset2&0xf))&0xff;
						//UINT8 xoredval = m_value0 ^ puzzli2_level_decode[tableaddr];
						//printf("value %02x, after xor is %02x (table address,value %02x,%02x)\n", m_value0, xoredval, tableaddr, puzzli2_level_decode[tableaddr]);
				
						hackcount2++;
						hack_31_table_offset2++;
					}
					else
					{
						hackcount2=0;
	
						// when the ARM detects the end of the stream has been reached it returns a 0x63 status with the number of columns in the data word
						valueresponse = 0x00630000 | numbercolumns;
				
						//UINT8 tableaddr = (hack_31_table_offset + (hack_31_table_offset2&0xf))&0xff;
						//UINT8 xoredval = m_value0 ^ puzzli2_level_decode[tableaddr];
						//printf("value %02x, after xor is %02x (table address,value %02x,%02x) (end, returning %02x as playfield width)\n", m_value0, xoredval, tableaddr, puzzli2_level_decode[tableaddr], m_valueresponse);


					}
				}


			}
			else
			{
				// todo, responses when uploading the startup values are different
				printf("%08x: %02x %04x (for z80 address?)\n ",pc, ddp3lastcommand, value0);

				valueresponse = 0x00d20000 | p2_31_retcounter;
				p2_31_retcounter++; // returns 0xc for the first one, 0x19 for the last one
			}

		}
		break;
	

		// after writing the compressed and scrambled data stream for the level (copied from ROM) with command 0x31
		// the game expects to read back a fully formed level structure from the ARM
		case 0x13:
		{
			//printf("%08x: %02x %04x (READ LEVEL DATA) |",pc, ddp3lastcommand, value0);

			// this is the how to play screen, correctly returned with current code
			/*
			UINT16 retvals[61] = 
			{ 0x0008, // depth (-2?)
			  0x0103, 0x0101, 0x0102, 0x0102, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // first column
			  0x0103, 0x0100, 0x0101, 0x0105, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			  0x0100, 0x0101, 0x0105, 0x0104, 0x0104, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			  0x0102, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			  0x0100, 0x0101, 0x0103, 0x0102, 0x0104, 0x0100 ,0x0100, 0x0000, 0x0000, 0x0000,
			  0x0105, 0x0105, 0x0101, 0x0101, 0x0103, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000  // last column
			};
			*/


			UINT16* leveldata = &level_structure[0][0];
			if (hackcount==0)
			{
				valueresponse = 0x002d0000 | ((depth>>4)+1); // this *seems* to come from upper bits of the first real value written to the device during the level stream (verify, seems wrong for some levels because you get a black bar on the bottom of the screen, but might be bad xors)
				printf("level depth returning %08x\n", valueresponse );
			}
			else if (hackcount<((10*numbercolumns)+1))
			{
				valueresponse = 0x002d0000 | leveldata[hackcount-1];
				printf("level data returning %08x\n", valueresponse );
			}
			else
			{
				hackcount=0;
				valueresponse = 0x00740054;  // 0x0074 0054 is returned after how to play reads above.. where does 0x54 come from?
				printf("END returning %08x\n", valueresponse );

			}

			hackcount++;


			// 2d seems to be used when there is more data available
			// 74 seems to be used when there isn't.. (end of buffer reached?)
			// 2d or 74! (based on?)

		}
		break;



		case 0x38: // Reset
			printf("%08x: %02x %04x (RESET)\n",pc, ddp3lastcommand, value0);
			simregion = readinputport(4);
			valueresponse = 0x780000 | simregion<<8; // this must also return the cart region or the game will act in odd ways when inserting a coin on continue, or during the game on later levels
			valuekey = 0x100;
			puzzli_54_trigger = 0;
			
		break;




		// 47 and 52 are used to get the images during the intro sequence, different each loop
		// also some other gfx?
		// logic here seems correct, not sure where the 0x19 and 0x5 etc. come from tho!
		case 0x47:
			printf("%08x: %02x %04x (GFX OFF PART 1)\n",pc, ddp3lastcommand, value0);

			hack_47_value = value0;

			//hack_47_value = ((m_value0 & 0x0700)>>8) * 0x19;
			//hack_47_value +=((m_value0 & 0x0007)>>0) * 0x05;
			if (value0 & 0xf0f0) printf("unhandled 0x47 bits %04x\n", value0);

			//printf("0x47 value was %04x - using %04x\n", m_value0, hack_47_value);

			valueresponse = 0x00740047;

		break;

		case 0x52:
			printf("%08x: %02x %04x (GFX OFF PART 2)\n",pc, ddp3lastcommand, value0);

			//printf("which %04x\n", m_value0);
			if (value0 & 0xfff0) printf("unhandled 0x52 bits %04x\n", value0);

			// it writes a value of 0x0000 then expects to read back like
			// this for the lower part of the game backgrounds
			if (value0==0x0000)
			{
				int val = ((hack_47_value & 0x0f00)>>8) * 0x19;
				valueresponse = 0x00740000 | (val & 0xffff);
			}
			else
			{

				int val = ((hack_47_value & 0x0f00)>>8) * 0x19;
				val +=((hack_47_value & 0x000f)>>0) * 0x05;
				val += value0 & 0x000f;
				valueresponse = 0x00740000 | (val & 0xffff);

			}


			
		break;



		case 0x61: // ??
			printf("%08x: %02x %04x\n",pc, ddp3lastcommand, value0);
			
			// this command is written before the values used to decrypt the z80 addresses (assumed) are uploaded with command 31
			command_31_write_type = 1;

			valueresponse = 0x36<<16;
			p2_31_retcounter = 0xc;
		break;

		case 0x41: // ASIC status?
			printf("%08x: %02x %04x (UNK)\n",pc, ddp3lastcommand, value0);

			// this command is written after the values used to decrypt the z80 addresses (assumed) are uploaded with command 31
			command_31_write_type = 0;

			//m_valueresponse = 0x74<<16;
			valueresponse = 0x740061;
		break;

		case 0x54: // ??
			printf("%08x: %02x %04x\n",pc, ddp3lastcommand, value0);
			
			// this command is written before uploading the compressed level data stream with command 31

			command_31_write_type = 2;
			stage = -1;
			puzzli_54_trigger = 1;
			hackcount2 = 0;
			hackcount = 0;
			valueresponse = 0x36<<16;
			
			//  clear the return structure
			for (columns=0;columns<8;columns++)
				for (rows=0;rows<10;rows++)
					level_structure[columns][rows] = 0x0000;

		break;

	/* 

  puzzli2 on startup (00148a84)         puzzli2s on startup (0014cf58)
	                                             
		(001489f6: 61 0202  0014ceca: 61 0202)
		                                       always
		: 31 004e a6f7 | : 31 | 0051 14c9       + 26DD2
		: 31 279e 534f | : 31 | 27a0 c121  
		: 31 ab5c a7cf | : 31 | ab5f 15a1  
		: 31 145f 7054 | : 31 | 1461 de26  
		: 31 85a0 7b7f | : 31 | 85a2 e951  
		: 31 7003 c5ab | : 31 | 7006 337d  
		: 31 456d f3aa | : 31 | 4570 617c  

		(00148b34: 41 e2bb  0014d008: 41 706d)

		actual values needed       always
		0x001694a8 / 0x0019027a  + 26DD2
		0x0016cfae / 0x00193D80
		0x0016ebf2 / 0x001959c4
		0x0016faa8 / 0x0019687a
		0x00174416 / 0x0019b1e8

		0x00166178 / 0x0018cf4a
		0x00166e72 / 0x0018dc44

		 as you can see the difference between the values written is always 26dd2, as is the difference between offsets expected
		 this makes it impossible to know which value is for which address without further tests!
	
	
	*/

		// I think the values returned here must be connected to the values written to command 31 on startup
		// 63/67 are used on startup to get the z80 music at least
		case 0x63: // used as a read address by the 68k code (related to previous uploaded values like cave?) should point at a table of ~0x80 in size? seems to use values as further pointers?
			printf("%08x: %02x %04x (Z80 ADDR PART 1)\n",pc, ddp3lastcommand, value0);

			if (!strcmp(Machine->gamedrv->name,"puzzli2"))
			{
				if (value0==0x0000)
				{
					valueresponse = 0x001694a8;
				}
				else if (value0==0x0001)
				{
					valueresponse = 0x0016cfae;
				}
				else if (value0==0x0002)
				{
					valueresponse = 0x0016ebf2; // right for puzzli2 , wrong for puzzli2s, probably calculated from the writes then?
				}
				else if (value0==0x0003) // before 'cast' screen
				{
					valueresponse = 0x0016faa8;
				}
				else if (value0==0x0004) // 2 player demo
				{
					valueresponse = 0x00174416;
				}
				else
				{
					printf("unk case x63\n");
					valueresponse = 0x00600000; // wrong

				}
			}
			else // puzzli2 super
			{
				if (value0==0x0000)
				{
					valueresponse = 0x19027a;
				}
				else if (value0==0x0001)
				{
					valueresponse = 0x193D80;
				}
				else if (value0==0x0002)
				{
					valueresponse = 0x1959c4;
				}
				else if (value0==0x0003)
				{
					valueresponse = 0x19687a;
				}
				else if (value0==0x0004)
				{
					valueresponse = 0x19b1e8;
				}
				else
				{
					printf("unk case x63\n");
					valueresponse = 0x00600000; // wrong
				}
			}
		break;

		case 0x67: // used as a read address by the 68k code (related to previous uploaded values like cave?) directly reads ~0xDBE from the address..
			printf("%08x: %02x %04x (Z80 ADDR PART 2)\n",pc, ddp3lastcommand, value0);

			if (!strcmp(Machine->gamedrv->name,"puzzli2"))
			{
				if ( (value0==0x0000) || (value0==0x0001) || (value0==0x0002) || (value0==0x0003) )
				{
					valueresponse = 0x00166178; // right for puzzli2 , wrong for puzzli2s, probably calculated from the writes then?
				}
				else if ( value0==0x0004 ) // 2 player demo
				{
					valueresponse = 0x00166e72;
				}
				else
				{
					printf("unk case x67\n");
					valueresponse = 0x00400000; // wrong
				}
			}
			else // puzzli2 super
			{
				if ((value0==0x0000) || (value0==0x0001) || (value0==0x0002) ||  (value0==0x0003))
				{
					valueresponse = 0x18cf4a;
				}
				else if ( value0==0x0004 ) // 2 player demo
				{
					valueresponse = 0x0018dc44;
				}
				else
				{
					printf("unk case x67\n");
					valueresponse = 0x00600000; // wrong
				}
			}
		break;

		default:
			printf("%08x: %02x %04x\n",pc, ddp3lastcommand, value0);

			valueresponse = 0x74<<16;
		break;
	}
 }
	else if (offset==2)
	{

	}
}

static READ16_HANDLER( puzzli2_ram_mirror_r )
{
	if (offset == 4)
		return simregion;

	return 0x0000;
}

void install_asic27a_puzzli2(void)
{
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM,  0x500000, 0x500005, 0, 0, ddp3_asic_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500005, 0, 0, puzzli2_asic_w);
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM,  0x4f0000, 0x4f003f, 0, 0, puzzli2_ram_mirror_r);
}

DRIVER_INIT(puzzli2)
{
    data16_t *mem16 = (data16_t *)memory_region(REGION_CPU1);
	pgm_basic_init();
	pgm_puzzli2_decrypt();
    install_asic27a_puzzli2();
	
	hackcount = 0;
	hackcount2 = 0;
	hack_47_value = 0;
	hack_31_table_offset = 0;
	hack_31_table_offset = 0;

    /* use ole IRQ4 disable patch as Puzzli 2 will lock into service mode if enabled */
    mem16[0x100070/2]=0x0012;
	mem16[0x100072/2]=0x5D78;

}

/* photo y2k 2 */

UINT16 py2k2_sprite_pos;
UINT16 py2k2_sprite_base;
UINT16 py2k2_prev_base;
UINT16 extra_ram[0x100];
	
static UINT32 py2k2_sprite_offset(UINT16 base, UINT16 pos)
{
	UINT16 ret = 0;
	UINT16 offset = (base * 16) + (pos & 0xf);

	switch (base & ~0x3f)
	{
		case 0x000: ret = BITSWAP16(offset ^ 0x0030, 15, 14, 13, 12, 11, 10, 0, 2, 3, 9, 5, 4, 8, 7, 6, 1); break;
		case 0x040: ret = BITSWAP16(offset ^ 0x03c0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 1, 2, 0, 5, 3, 4); break;
		case 0x080: ret = BITSWAP16(offset ^ 0x0000, 15, 14, 13, 12, 11, 10, 0, 3, 4, 6, 8, 7, 5, 9, 2, 1); break;
		case 0x0c0: ret = BITSWAP16(offset ^ 0x0001, 15, 14, 13, 12, 11, 10, 6, 5, 4, 3, 2, 1, 9, 8, 7, 0); break;
		case 0x100: ret = BITSWAP16(offset ^ 0x0030, 15, 14, 13, 12, 11, 10, 0, 2, 3, 9, 5, 4, 8, 7, 6, 1); break;
		case 0x140: ret = BITSWAP16(offset ^ 0x01c0, 15, 14, 13, 12, 11, 10, 2, 8, 7, 6, 4, 3, 5, 9, 0, 1); break;
		case 0x180: ret = BITSWAP16(offset ^ 0x0141, 15, 14, 13, 12, 11, 10, 4, 8, 2, 6, 1, 7, 9, 5, 3, 0); break;
		case 0x1c0: ret = BITSWAP16(offset ^ 0x0090, 15, 14, 13, 12, 11, 10, 5, 3, 7, 2, 1, 4, 0, 9, 8, 6); break;
		case 0x200: ret = BITSWAP16(offset ^ 0x02a1, 15, 14, 13, 12, 11, 10, 9, 1, 7, 8, 5, 6, 2, 4, 3, 0); break;
		case 0x240: ret = BITSWAP16(offset ^ 0x0000, 15, 14, 13, 12, 11, 10, 3, 2, 1, 0, 9, 8, 7, 6, 5, 4); break;
		case 0x280: ret = BITSWAP16(offset ^ 0x02a1, 15, 14, 13, 12, 11, 10, 9, 1, 7, 8, 5, 6, 2, 4, 3, 0); break;
		case 0x2c0: ret = BITSWAP16(offset ^ 0x0000, 15, 14, 13, 12, 11, 10, 0, 3, 4, 6, 8, 7, 5, 9, 2, 1); break;
		case 0x300: ret = BITSWAP16(offset ^ 0x03c0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 1, 2, 0, 5, 3, 4); break;
		case 0x340: ret = BITSWAP16(offset ^ 0x0030, 15, 14, 13, 12, 11, 10, 0, 2, 3, 9, 5, 4, 8, 7, 6, 1); break;
		case 0x380: ret = BITSWAP16(offset ^ 0x0001, 15, 14, 13, 12, 11, 10, 6, 5, 4, 3, 2, 1, 9, 8, 7, 0); break;
		case 0x3c0: ret = BITSWAP16(offset ^ 0x0090, 15, 14, 13, 12, 11, 10, 5, 3, 7, 2, 1, 4, 0, 9, 8, 6); break;
		case 0x400: ret = BITSWAP16(offset ^ 0x02a1, 15, 14, 13, 12, 11, 10, 9, 1, 7, 8, 5, 6, 2, 4, 3, 0); break;
		case 0x440: ret = BITSWAP16(offset ^ 0x03c0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 1, 2, 0, 5, 3, 4); break;
		case 0x480: ret = BITSWAP16(offset ^ 0x0141, 15, 14, 13, 12, 11, 10, 4, 8, 2, 6, 1, 7, 9, 5, 3, 0); break;
		case 0x4c0: ret = BITSWAP16(offset ^ 0x01c0, 15, 14, 13, 12, 11, 10, 2, 8, 7, 6, 4, 3, 5, 9, 0, 1); break;
		case 0x500: ret = BITSWAP16(offset ^ 0x0141, 15, 14, 13, 12, 11, 10, 4, 8, 2, 6, 1, 7, 9, 5, 3, 0); break;
		case 0x540: ret = BITSWAP16(offset ^ 0x0030, 15, 14, 13, 12, 11, 10, 0, 2, 3, 9, 5, 4, 8, 7, 6, 1); break;
		case 0x580: ret = BITSWAP16(offset ^ 0x03c0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 1, 2, 0, 5, 3, 4); break;
		case 0x5c0: ret = BITSWAP16(offset ^ 0x0090, 15, 14, 13, 12, 11, 10, 5, 3, 7, 2, 1, 4, 0, 9, 8, 6); break;
		case 0x600: ret = BITSWAP16(offset ^ 0x0000, 15, 14, 13, 12, 11, 10, 0, 3, 4, 6, 8, 7, 5, 9, 2, 1); break;
		case 0x640: ret = BITSWAP16(offset ^ 0x0000, 15, 14, 13, 12, 11, 10, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5); break;
	}

	if (offset >= 0xce80/2 && offset <= 0xceff/2) ret -= 0x0100;
	if (offset >= 0xcf00/2 && offset <= 0xcf7f/2) ret += 0x0100;

	return ret;
}

static WRITE16_HANDLER( py2k2_asic_w )
{
	if (offset == 0)
	{
		value0 = data;
		return;
	}
	else if (offset == 1)
	{
		UINT16 realkey;
		if ((data >> 8) == 0xff)
			valuekey = 0xff00;
		realkey = valuekey >> 8;
		realkey |= valuekey;
		{
			valuekey += 0x0100;
			valuekey &= 0xff00;
			if (valuekey == 0xff00)
				valuekey =  0x0100;
		}
		data ^= realkey;
		value1 = data;
		value0 ^= realkey;

		ddp3lastcommand = value1 & 0xff;
		
	switch (ddp3lastcommand)
	{
		case 0x30:
			valueresponse = py2k2_sprite_offset(py2k2_sprite_base, py2k2_sprite_pos++);
		break;
		case 0x32:
			py2k2_sprite_base = value0;
			py2k2_sprite_pos = 0;
			valueresponse = py2k2_sprite_offset(py2k2_sprite_base, py2k2_sprite_pos++);
		break;
		case 0xba:
			valueresponse = py2k2_prev_base;
			py2k2_prev_base = value0;
		break;
		case 0x99: // reset?
			py2k2_prev_base = value0;
			simregion = readinputport(4);
			valuekey = 0x100;
			valueresponse = 0x00880000 | simregion<<8;
		break;
		case 0xc0:
			printf("%06x command %02x | %04x\n", pc, ddp3lastcommand, value0);
			valueresponse = 0x880000;
			break;
		case 0xc3:
			valueresponse = 0x904000 + ((extra_ram[0xc0] + (value0 * 0x40)) * 4);
		break;
		case 0xd0:
			valueresponse = 0xa01000 + (value0 * 0x20);
		break;
		case 0xdc:
			valueresponse = 0xa00800 + (value0 * 0x40);
		break;
		case 0xe0:
			valueresponse = 0xa00000 + ((value0 & 0x1f) * 0x40);
		break;
		case 0xcb: // Background layer 'x' select (pgm3in1, same as kov)
			valueresponse = 0x880000;
		break;
		case 0xcc: // Background layer offset (pgm3in1, same as kov)
		{
			int y = value0;
			if (y & 0x400) y = -(0x400 - (y & 0x3ff));
			valueresponse = 0x900000 + ((extra_ram[0xcb] + (y * 0x40)) * 4);
		}
		break;
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x37:
		case 0x38:
		default:
			printf("%06x command %02x | %04x\n", pc, ddp3lastcommand, value0);
			valueresponse = 0x880000;
			break;
	}

  }
	else if (offset==2)
	{

	}
}


void install_asic27a_py2k2(void)
{
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM,  0x500000, 0x500005, 0, 0, ddp3_asic_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x500000, 0x500005, 0, 0, py2k2_asic_w);
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM,  0x4f0000, 0x4f003f, 0, 0, puzzli2_ram_mirror_r);
}

DRIVER_INIT(py2k2)
{
	pgm_basic_init();
	pgm_py2k2_decrypt();
    install_asic27a_py2k2();
	
    py2k2_sprite_pos = 0;
	py2k2_sprite_base = 0;
	py2k2_prev_base = 0;

}

/*** Rom Loading *************************************************************/

/* take note of REGION_GFX2 needed for expanding the 32x32x5bpp data and
   REGION_GFX4 needed for expanding the Sprite Colour Data */

/* The Bios - NOT A GAME */
ROM_START( pgm )
	ROM_REGION( 0x520000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x00000, 0x20000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x200000, REGION_GFX1, 0 ) /* 8x8 Text Layer Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) )
ROM_END

ROM_START( orlegend )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0103.rom",    0x100000, 0x200000, CRC(d5e93543) SHA1(f081edc26514ca8354c13c7f6f89aba8e4d3e7d2) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x800000, REGION_GFX1,  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegnde )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0102.rom",    0x100000, 0x200000, CRC(4d0f6cc5) SHA1(8d41f0a712fb11a1da865f5159e5e27447b4388a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x800000, REGION_GFX1,  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegndc )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0101.160",    0x100000, 0x200000, CRC(b24f0c1e) SHA1(a2cf75d739681f091c24ef78ed6fc13aa8cfe0c6) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x800000, REGION_GFX1,  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Oriental Legend / Xi Yo Gi Shi Re Zuang (CHINA 111 Ver.)
(c)1997 IGS

PGM system
IGS PCB NO-0134-1
IGS PCB NO-0135


OLV111CH.U11 [b80ddd3c]
OLV111CH.U6  [5fb86373]
OLV111CH.U7  [6ee79faf]
OLV111CH.U9  [83cf09c8]

T0100.U8


A0100.U5
A0101.U6
A0102.U7
A0103.U8
A0104.U11
A0105.U12

B0100.U9
B0101.U10
B0102.U15

M0100.U1

*/

ROM_START( orld111c )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_BYTE( "olv111ch.u6",     0x100001, 0x080000, CRC(5fb86373) SHA1(2fc58eff1f38754c75819fde666244b867ca4f05) )
	ROM_LOAD16_BYTE( "olv111ch.u9",     0x100000, 0x080000, CRC(83cf09c8) SHA1(959780b45326059517f3008a356657f4f3d2908f) )
	ROM_LOAD16_BYTE( "olv111ch.u7",     0x200001, 0x080000, CRC(6ee79faf) SHA1(039b4b07b8577f0d3022ae01210c00375624cb3c) )
	ROM_LOAD16_BYTE( "olv111ch.u11",    0x200000, 0x080000, CRC(b80ddd3c) SHA1(55c700ce71ffdee392e03fd9d4719542c3527132) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x800000, REGION_GFX1,  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Oriental Legend / Xi Yo Gi Shi Re Zuang (KOREA 105 Ver.)
(c)1997 IGS

PGM system
IGS PCB NO-0134-2
IGS PCB NO-0135


OLV105KO.U11 [40ae4d9e]
OLV105KO.U6  [b86703fe]
OLV105KO.U7  [5712facc]
OLV105KO.U9  [5a108e39]

T0100.U8


A0100.U5
A0101.U6
A0102.U7
A0103.U8
A0104.U11
A0105.U12

B0100.U9
B0101.U10
B0102.U15

M0100.U1

*/

ROM_START( orld105k )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_BYTE( "olv105ko.u6",     0x100001, 0x080000, CRC(b86703fe) SHA1(a3529b45efd400ecd5e76f764b528ebce46e24ab) )
	ROM_LOAD16_BYTE( "olv105ko.u9",     0x100000, 0x080000, CRC(5a108e39) SHA1(2033f4fe3f2dfd725dac535324f58348b9ac3914) )
	ROM_LOAD16_BYTE( "olv105ko.u7",     0x200001, 0x080000, CRC(5712facc) SHA1(2d95ebd1703874e89ac3a206f8c1f0ece6e833e0) )
	ROM_LOAD16_BYTE( "olv105ko.u11",    0x200000, 0x080000, CRC(40ae4d9e) SHA1(62d7a96438b7fe93f74753333f50e077d417971e) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x800000, REGION_GFX1,  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*
Oriental Legend Super
IGS, 1998
This is a cart for the IGS PGM system.
PCB Layout
----------
IGS PCB NO-0191-1
|-----------------------------------------------|
|6264                 8MHz|--------|            |
|6264                     |        |   T0500.U18|
|                         | IGS025 |            |
|                 |-----| |        |   T0501.U19|
|                 | IGS | |--------|            |
|                 | 028 |                       |
|        *1       |-----|           V101.U1     |
|              V101.U2   V101.U4  PAL      PAL  |
|  V101.U6          V101.U3   V101.U5           |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS028       - Custom IGS IC (QFP100)
      IGS025       - Custom IGS IC (PLCC68, labelled "KOREA")
      T0500.U18    - 32MBit MaskROM (SOP44)
      T0501.U19    - 16MBit MaskROM (SOP44)
      V101.U1      - MX27C4096 4MBit EPROM (DIP40)
      V101.U2/3/4/5- MX27C4000 4MBit EPROM (DIP32)
      PALs         - x2, labelled "CW-2 U8", "CW-2 U7"
      6264         - 8K x8 SRAM
      *1           - Unpopulated position for SOP44 MASKROM labelled "P0500"
IGS PCB NO-0135
|-----------------------------------------------|
|A0504.U11        A0506.U13     B0502.U15       |
|         A0505.U12         U14        B0503.U16|
|                                               |
|A0500.U5                       B0500.U9        |
|         A0501.U6       A0503.U8      B0501.U10|
|                 A0502.U7                      |
|                                               |
|74LS138          M0500.U1               74LS139|
|                           U2                  |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      This PCB contains only SOP44 MaskROMS and 2 logic IC's
      U2 and U14 are not populated.
      All are 32MBit except M0500 which is 16MBit.
*/

ROM_START( olds )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )/* (BIOS)*/
	ROM_LOAD16_BYTE( "sp_v101.u2",      0x100001, 0x080000,   CRC(08eb9661) SHA1(105946e72e562adb1a9fd794ca0fd2c91967eb56) )
	ROM_LOAD16_BYTE( "sp_v101.u3",      0x100000, 0x080000,   CRC(0a358c1e) SHA1(95c7c3f069c5d05001e22535750f6b3cd7de105f) )
	ROM_LOAD16_BYTE( "sp_v101.u4",      0x200001, 0x080000,   CRC(766570e0) SHA1(e7c3f5664ec69b662b82c2e1375555db7305390c) )
	ROM_LOAD16_BYTE( "sp_v101.u5",      0x200000, 0x080000,   CRC(58662e12) SHA1(2b39bd847e9c4968a8e77a2f3cec77cf323ceee3) )
	ROM_LOAD16_WORD_SWAP( "sp_v101.u1",0x300000, 0x080000,    CRC(2b2f4f1e) SHA1(67b97cf8cc7f517d67cd45588addd2ad8e24612a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x010000, REGION_USER1, 0 ) /* ASIC25? Protection Data */
	ROM_LOAD( "sp_v101.u6", 0x000000, 0x010000,  CRC(097046bc) SHA1(6d75db85cf4c79b63e837897785c253014b2126d) )
	
	//ROM_REGION( 0x4000, REGION_USER2, ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// clearly not for this revision
	//ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION( 0xc00000, REGION_GFX1,  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) /* (BIOS)*/
	ROM_LOAD( "t0500.rom",    0x400000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x800000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )


	ROM_REGION( 0x600000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) /* (BIOS)*/
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

ROM_START( olds103t )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0500.v103",0x100000, 0x400000, CRC(17e32e14) SHA1(b8f731087af2c59fe5b1da31f1cb055d35c8b440) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */
	
	ROM_REGION( 0xc00000, REGION_GFX1,  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0500.rom",    0x400000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x800000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )
	
	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE )/* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x1000000,  REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

/*

Dragon World 2 (English / World Version)
IGS, 1997

This is a cart for the IGS PGM system.


PCB Layout
----------

IGS PCB NO-0162
|-----------------------------------------------|
| |------|                                      |
| |IGS012|       *1                    T0200.U7 |
| |      |                                      |
| |------|                                      |
|              |--------|                       |
|              |        |                       |
|              | IGS025 |  *2   V-110X.U2       |
| PAL    PAL   |        |                  PAL  |
|              |--------|                       |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS012       - Custom IGS IC (QFP80)

      -- on english version
      IGS025       - Custom IGS IC (PLCC68, labelled "DRAGON II 0006")
      -- on china version
      IGS025       - Custom IGS IC (PLCC68, labelled "DRAGON II 0005")


      T0200.U7     - 32MBit MaskROM (SOP44)

  	  -- on english version
  	  V-110X.U2    - AM27C4096 4MBit EPROM (DIP42, labelled "DRAGON II V-110X")
      -- on china version
      V-110X.U2    - AM27C4096 4MBit EPROM (DIP42, labelled "DRAGON II V-100C")

      PALs         - x3, labelled "CZ U3", "CZ U4", "CZ U6"
      *1           - Unpopulated position for MX23C4100 SOP40 MASKROM
      *2           - Unpopulated position for MX23C4100 DIP40 EPROM/MASKROM


IGS PCB NO-0135
|-----------------------------------------------|
| U11    U12     U13      U14       U15      U16|
|                                               |
|                                               |
|A0200.U5                       B0200.U9        |
|        U6      U7       U8                 U10|
|                                               |
|                                               |
|74LS138         U1       U2             74LS139|
|                                               |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      This PCB contains only SOP44 MASKROMS and 2 logic IC's
      Only U5 and U9 are populated

      glitch on select screen exists on real board.

*/

ROM_START( drgw2 )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "v-110x.u2",    0x100000, 0x080000, CRC(1978106b) SHA1(af8a13d7783b755a58762c98bdc32cab845b2251) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x800000, REGION_GFX1,  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "pgmt0200.u7",    0x400000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x800000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x400000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
ROM_END


ROM_START( drgw2c )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "v-100c.u2",    0x100000, 0x080000, CRC(67467981) SHA1(58af01a3871b6179fe42ff471cc39a2161940043) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x800000, REGION_GFX1,  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "pgmt0200.u7",    0x400000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x800000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x400000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
ROM_END

/*

Dragon World 3
Alta Co./IGS, 1998

Cart for IGS PGM system

Top board of cart contains.....
8MHz Xtal
32.768kHz Xtal
UM6164 (RAM x 2)
MACH211 CPLD
IGS022 ASIC
IGS025 ASIC
1x PAL
2x 27C040 EPROMs (main 68k program)
1x 27C512 EPROM (protection code?)
1x 32MBit smt MASKROM (T0400)

Bottom board contains.....
4x 32MBit smt MASKROMs (A0400, A0401, B0400, M0400)

*/

ROM_START( drgw3 )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_BYTE( "dw3_v100.u12",     0x100001, 0x080000,  CRC(47243906) SHA1(9cd46e3cba97f049bcb238ceb6edf27a760ef831) )
	ROM_LOAD16_BYTE( "dw3_v100.u13",     0x100000, 0x080000,  CRC(b7cded21) SHA1(c1ae2af2e42227503c81bbcd2bd6862aa416bd78) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x010000, REGION_USER1, 0 ) /* Protection Data */
	ROM_LOAD( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "dw3t0400.u18",   0x400000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END

/*

Dragon World 3 (KOREA 106 Ver.)
(c)1998 IGS

PGM system
IGS PCB NO-0189
IGS PCB NO-0178


DW3_V106.U12 [c3f6838b]
DW3_V106.U13 [28284e22]


*/

ROM_START( drgw3k )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_BYTE( "dw3_v106.u12",     0x100001, 0x080000,  CRC(c3f6838b) SHA1(c135b1d4dd62af308139d40d03c29be7508fb1e7) )
	ROM_LOAD16_BYTE( "dw3_v106.u13",     0x100000, 0x080000,  CRC(28284e22) SHA1(4643a69881ddb7383ca10f3eb2aa2cf41be39e9f) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x010000, REGION_USER1, 0 ) /* Protection Data - is it correct for this set? */
	ROM_LOAD( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "dw3t0400.u18",   0x400000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END

ROM_START( kov )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0600.117",    0x100000, 0x400000, CRC(c4d19fe6) SHA1(14ef31539bfbc665e76c9703ee01b12228344052) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kov115 )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0600.115",    0x100000, 0x400000, CRC(527a2924) SHA1(7e3b166dddc5245d7b408e78437c16fd2986d1d9) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x600000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x200000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

/*

Sangoku Senki / Knights of Valour (JPN 100 Ver.)
(c)1999 ALTA / IGS

PGM system
IGS PCB NO-0212-1
IGS PCB NO-0213T


SAV111.U10   [d5536107]
SAV111.U4    [ae2f1b4e]
SAV111.U5    [5fdd4aa8]
SAV111.U7    [95eedf0e]
SAV111.U8    [003cbf49]

T0600.U11


A0600.U2
A0601.U4
A0602.U6
A0603.U9

M0600.U3

B0600.U5
B0601.U7

*/

ROM_START( kovj )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_BYTE( "sav111.u4",      0x100001, 0x080000, CRC(ae2f1b4e) SHA1(2ac9d84f5dee52f374941cfd68e2b98ecad436a8) )
	ROM_LOAD16_BYTE( "sav111.u7",      0x100000, 0x080000, CRC(95eedf0e) SHA1(582a54e9a1eda7ff73e20f0e69d2d50141772378) )
	ROM_LOAD16_BYTE( "sav111.u5",      0x200001, 0x080000, CRC(5fdd4aa8) SHA1(43c96e21ad4f11148e1e94a59c53780b2edd43ba) )
	ROM_LOAD16_BYTE( "sav111.u8",      0x200000, 0x080000, CRC(003cbf49) SHA1(fb5bea47ecae025b1b425af52cd05e061f45e377) )
	ROM_LOAD16_WORD_SWAP( "sav111.u10",0x300000, 0x080000, CRC(d5536107) SHA1(f963e015d99c1621323eecf63e773c0b9f4b6a43) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovplus )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) ) // (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0600.119",    0x100000, 0x400000, CRC(e4b0875d) SHA1(e8382e131b0e431406dc2a05cc1ef128302d987c) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

/*

Sangoku Senki Plus / Knights of Valour Plus (Alt 119 Ver.)
(c)1999 IGS

PGM system
IGS PCB NO-0222
IGS PCB NO-0213


V119.U2      [29588ef2]
V119.U3      [6750388f]
V119.U4      [8200ece6]
V119.U5      [d4101ffd]
V119.U6      [71e28f27]

T0600.U11


A0600.U2
A0601.U4
A0602.U6
A0603.U9

M0600.U3

B0600.U5
B0601.U7

*/

ROM_START( kovplusa )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) ) // (BIOS)
	ROM_LOAD16_BYTE( "v119.u3",     0x100001, 0x080000, CRC(6750388f) SHA1(869f4ad27f2992cc62baa9a78bf7984a43ec4cc5) )
	ROM_LOAD16_BYTE( "v119.u5",     0x100000, 0x080000, CRC(d4101ffd) SHA1(a327fd56eec65b07df9305cd93ef2c46bf8e40f3) )
	ROM_LOAD16_BYTE( "v119.u4",     0x200001, 0x080000, CRC(8200ece6) SHA1(97081d2e8aed2ac6fbe5951890aecea18af5ce2e) )
	ROM_LOAD16_BYTE( "v119.u6",     0x200000, 0x080000, CRC(71e28f27) SHA1(db382807e9185f0dc17124f210165fa1b36ca6ac) )
	ROM_LOAD16_WORD_SWAP( "v119.u2",0x300000, 0x080000, CRC(29588ef2) SHA1(17d1a308d44434cf65224a24360cf4b6e32d28f3) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0600.322",    0x100000, 0x400000, CRC(7c78e5f3) SHA1(9b1e4bd63fb1294ebeb539966842273c8dc7683b) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.320",    0x400000, 0x400000, CRC(164b3c94) SHA1(f00ea66886ca6bff74bbeaa49e7f5c75c275d5d7) ) // bad? its half the size of the kov one

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( photoy2k )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "v104.16m",     0x100000, 0x200000, CRC(e051070f) SHA1(a5a1a8dd7542a30632501af8d02fda07475fd9aa) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x480000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0700.rom",    0x400000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x480000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1080000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0700.rom",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END

/*

Real and Fake / Photo Y2K (JPN 102 Ver.)
(c)1999 ALTA / IGS

PGM system
IGS PCB NO-0220
IGS PCB NO-0221


V102.U4      [a65eda9f]
V102.U5      [9201621b]
V102.U6      [b9ca5504]
V102.U8      [3be22b8f]

T0700.U11


A0700.U2
A0701.U4

SP_V102.U5

B0700.U7

CG_V101.U3
CG_V101.U6

*/

ROM_START( raf102j )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_BYTE( "v102.u4",     0x100001, 0x080000, CRC(a65eda9f) SHA1(6307cacf4a262e781753eff14700a0455837780c) )
	ROM_LOAD16_BYTE( "v102.u6",     0x100000, 0x080000, CRC(b9ca5504) SHA1(058cf01316f233236ca9861349f515935283b75e) )
	ROM_LOAD16_BYTE( "v102.u5",     0x200001, 0x080000, CRC(9201621b) SHA1(1ca3ebe7eec40614bfa8b911657fa2b51f2c51a4) )
	ROM_LOAD16_BYTE( "v102.u8",     0x200000, 0x080000, CRC(3be22b8f) SHA1(03634fbd6a8a8369c6cb1fd6694a3784dac5bf59) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x480000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0700.rom",    0x400000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x480000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1080000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0700.rom",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END

/*
The Killing Blade (English / World Version)
IGS, 1998
This is a cart for the IGS PGM system.

Notes:
      U1           - 32MBit MASKROM (SOP44, labelled "M0300")
      U2           - 32MBit MASKROM (SOP44, labelled "A0307")
      U3           - 16MBit MASKROM (DIP42, labelled "A0302")
      U4           - 16MBit MASKROM (DIP42, labelled "A0304")
      U5           - 16MBit MASKROM (DIP42, labelled "A0305")
      U8           - 16MBit MASKROM (DIP42, labelled "B0301")
      U9           - 32MBit MASKROM (SOP44, labelled "A0300")
      U10          - 32MBit MASKROM (SOP44, labelled "A0301")
      U11          - 32MBit MASKROM (SOP44, labelled "A0303")
      U12          - 32MBit MASKROM (SOP44, labelled "A0306")
      U13          - 32MBit MASKROM (SOP44, labelled "B0300")
      U14          - 32MBit MASKROM (SOP44, labelled "B0302")
      U15          - 32MBit MASKROM (SOP44, labelled "B0303")
	  
the text on the chip are

IGS
PGM P0300 V109
1A0577Y3
J982846

*/

ROM_START( killbld )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0300_v109.u9", 0x100000, 0x200000, CRC(2fcee215) SHA1(855281a9090bfdf3da9f4d50c121765131a13400) )

	ROM_REGION( 0x10000,  REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x010000, REGION_USER1, 0 ) /* Protection Data */
	ROM_LOAD16_WORD_SWAP( "kb_u2.rom", 0x000000, 0x010000,  CRC(de3eae63) SHA1(03af767ef764055bda528b5cc6a24b9e1218cca8) )

	ROM_REGION( 0x800000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0300.u14",    0x400000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION( 0x800000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x2000000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END


ROM_START( puzlstar )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) ) // (BIOS)
	ROM_LOAD16_BYTE( "v100mg.u1",     0x100001, 0x080000, CRC(5788b77d) SHA1(7770aae6e686da92b2623c977d1bc8f019f48267) )
	ROM_LOAD16_BYTE( "v100mg.u2",     0x100000, 0x080000, CRC(4c79d979) SHA1(3b92052a35994f2b3dd164930154184c45d5e2d0) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x4000, REGION_CPU3, ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	/* this has no external rom so the internal rom probably can't be dumped */
//  ROM_LOAD( "puzlstar_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0800.u5",    0x400000, 0x200000, CRC(f9d84e59) SHA1(80ec77025ac5bf355b1a60f2a678dd4c56071f6b) )

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0800.u1",    0x0000000, 0x0400000, CRC(e1e6ec40) SHA1(390432431f144ef63424a426582b311765a61771) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0800.u3",    0x0000000, 0x0200000, CRC(52e7bef5) SHA1(a678251b7e46a1016d0afc1d8d5c9928008ad5b1) )

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0800.u2",    0x400000, 0x400000,  CRC(e1a46541) SHA1(6fe9de5700d8638374734d80551dcedb62975140) )
ROM_END

ROM_START( ket )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_v100.u38", 0x000000, 0x200000, CRC(dfe62f3b) SHA1(baa58d1ce47a707f84f65779ac0689894793e9d9) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION32_LE( 0x400000, REGION_USER1, ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, REGION_GFX1, ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom",    0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",  0x400000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */
	
	ROM_REGION( 0x1000000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1
ROM_END

ROM_START( keta )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_prg_revised.bin", 0x000000, 0x200000, CRC(69fcf5eb) SHA1(f726e251b4daa2f8d717e32000d4d7abc71c710d) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION32_LE( 0x400000, REGION_USER1, ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",   0x400000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1
	
	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1000000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "music-1.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) )
ROM_END

ROM_START( ketb )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_prg_original.bin", 0x000000, 0x200000, CRC(cca5e153) SHA1(b653feaa2004c379312def6b1613c3497f654ddf) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION32_LE( 0x400000, REGION_USER1, ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",   0x400000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1
	
	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1000000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, REGION_SOUND1, 0 )  /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "music-1.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) )
ROM_END

ROM_START( espgal )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "espgaluda_v100.u38", 0x000000, 0x200000, CRC(08ecec34) SHA1(bce2e7fb9105ed51603d09cbd3a9eeb5b8f47ee2) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION32_LE( 0x400000, REGION_USER1, ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, REGION_GFX1, ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom",     0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04801w064.u19",   0x400000, 0x800000, CRC(6021c79e) SHA1(fbc340dafb18aa3094de29b881318a5a9794e4bc) ) //text-1
	
	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1000000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a04801w064.u7",    0x0000000, 0x0800000, CRC(26dd4932) SHA1(9bbabb5a53cb5ba88397cc2c258980f3b70314ce) ) //image-1
	ROM_LOAD( "a04802w064.u8",    0x0800000, 0x0800000, CRC(0e6bf7a9) SHA1(a7541e2b5a0df2bc62a5b347e54dbc2ed1922db2) ) //image-2

	ROM_REGION( 0x0800000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04801w064.u1",    0x0000000, 0x0800000, CRC(98dce13a) SHA1(61d48b7117459f7babc022b68231f6928177a71d) ) //bitmap-1

	ROM_REGION( 0x800000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "w04801b032.u17",    0x400000, 0x400000, CRC(60298536) SHA1(6b7333f16cce778c5725dbdf75a5446f0906397a) ) //music-1
ROM_END

ROM_START( ddp3 )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_v101.u36",  0x100000, 0x200000, CRC(195b5c1e) SHA1(f18d791c034b0a3d85888a92fb5d326ee3deb04f) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION32_LE( 0x400000, REGION_USER1, ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, REGION_GFX1, ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x400000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1
	
	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) )
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) )

	ROM_REGION( 0x1000000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) )
ROM_END

ROM_START( ddp3a )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_d_d_1_0.u36",  0x100000, 0x200000, CRC(5d3f85ba) SHA1(4c24ea206140863d456179750366921442e1d2b8) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION32_LE( 0x400000, REGION_USER1, ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x400000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1
	
	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) )
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) )

	ROM_REGION( 0x1000000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) )
ROM_END

ROM_START( ddp3b )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "dd v100.bin",  0x100000, 0x200000, CRC(7da0c1e4) SHA1(aca2fe35ba0ab3628900fa2aba2d22fc4fd7046d) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION32_LE( 0x400000, REGION_USER1, ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom",  0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x400000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1
	
	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) )
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) )

	ROM_REGION( 0x1000000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) )
ROM_END

ROM_START( ddp3w )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_v101_16m.u36",  0x100000, 0x200000,CRC(fba2180e) SHA1(59c1a76243e587c07215c8a76649401ef0bff7c7) ) // marked (chinese text) 3 V101 16M

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION32_LE( 0x400000, REGION_USER1, ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom",  0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x400000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1
	
	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) )
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) )

	ROM_REGION( 0x1000000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) )
ROM_END

/* this expects Magic values in NVRAM to boot */
ROM_START( ddp3blk )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddb10.u45",  0x100000, 0x200000, CRC(72b35510) SHA1(9a432e5e1ebe61aafd737b6acc905653e5af0d38) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION32_LE( 0x400000, REGION_USER1, ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, REGION_GFX1, ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom",  0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x400000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1
	
	ROM_REGION( 0xc00000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, REGION_GFX3, ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) )
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) )

	ROM_REGION( 0x1000000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom",    0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) )
ROM_END

ROM_START( puzzli2 )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
    ROM_LOAD( "v100.u5",     0x100000, 0x200000, CRC(1abb4595) SHA1(860bb49efc3cb55b6b9846f5ab787d6fd586432d) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x600000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0900.u9",    0x400000, 0x200000, CRC(70615611) SHA1(a46d4aa71396947b427f9ba4ba0e636876c09d6b) )

	ROM_REGION( 0x600000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x400000, REGION_GFX3, 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0900.u3",    0x0000000, 0x0400000, CRC(14911251) SHA1(e0d10ef50c408dbcf0907f81d4f0e49aeb651a6c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0200000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0900.u4",    0x0000000, 0x0200000,  CRC(6f0638b6) SHA1(14b315fe9e80b3314bb63487e6ea9ce04c9703bd) )

	ROM_REGION( 0x1000000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0900.u2",     0x400000, 0x400000, CRC(9ea7af2e) SHA1(d2593d391a93c5cf5a554750c32886dea6599b3d) )
ROM_END

ROM_START( puzzli2s )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_BYTE( "2sp_v200.u3",     0x100001, 0x080000, CRC(2a5ba8a6) SHA1(4c87b849fd6f39152e3e2ef699b78ce24b3fb6d0) )
	ROM_LOAD16_BYTE( "2sp_v200.u4",     0x100000, 0x080000, CRC(fa5c86c1) SHA1(11c219722b891b775c0f7f9bc8276cdd8f74d657) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x600000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0900.u9",    0x400000, 0x200000, CRC(70615611) SHA1(a46d4aa71396947b427f9ba4ba0e636876c09d6b) )

	ROM_REGION( 0x600000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x400000, REGION_GFX3, 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0900.u3",    0x0000000, 0x0400000, CRC(14911251) SHA1(e0d10ef50c408dbcf0907f81d4f0e49aeb651a6c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0200000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0900.u4",    0x0000000, 0x0200000,  CRC(6f0638b6) SHA1(14b315fe9e80b3314bb63487e6ea9ce04c9703bd) )

	ROM_REGION( 0x1000000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0900.u2",     0x400000, 0x400000, CRC(9ea7af2e) SHA1(d2593d391a93c5cf5a554750c32886dea6599b3d) )
ROM_END

ROM_START( py2k2 )
	ROM_REGION( 0x600000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom",  0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "y2k2_m-101xx.u1", 0x100000, 0x200000, CRC(c47795f1) SHA1(5be4af4275571932d7740c3ea0857a1f58a3f6d9) ) // 68k (encrypted) 2nd half empty...

    ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* Z80 - romless */

	ROM_REGION( 0x480000, REGION_GFX1, 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	/* no extra tilemap rom */

	ROM_REGION( 0x480000/5*8, REGION_GFX2, ROMREGION_DISPOSE ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x2000000, REGION_GFX3, 0 ) /* Sprite Colour Data */
	ROM_LOAD( "y2k2_a1100.u6",    0x0000000, 0x0800000, CRC(e32ce499) SHA1(f84c7daa55c25a05da467b5654ebf432b7ce1754) )
	ROM_LOAD( "y2k2_a1101.u7",    0x0800000, 0x0800000, CRC(4e7568bc) SHA1(bf9cc453191bd5ec9fbcce62891809f253a44267) )
	ROM_LOAD( "y2k2_a1102.u8",    0x1000000, 0x0800000, CRC(6da7c143) SHA1(9408ba7722bfc8013f851aadea5e2819f5263129) )
	ROM_LOAD( "y2k2_a1103.u9",    0x1800000, 0x0800000, CRC(0ebebfdc) SHA1(4faad7f97c7e734f179ec934a37e75d8d6adccf4) )

	ROM_REGION( 0x1000000, REGION_GFX4, 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "y2k2_b1100.u4",    0x0000000, 0x0800000,  CRC(fa53d6f6) SHA1(c2da55f4b7e721fa1c63bd7f9528f261643164e8) )
	ROM_LOAD( "y2k2_b1101.u5",    0x0800000, 0x0800000, CRC(001e4c81) SHA1(21119055f8fd7f831529e73ff9c97bca3987a1dc))

	ROM_REGION( 0x880000, REGION_SOUND1, 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "y2k2_m1100.u3",    0x400000, 0x200000, CRC(fb1515f8) SHA1(90e5e5bfdac9a460445bf224952e4a536888dc1b) )
ROM_END

/*** GAME ********************************************************************/

GAMEX( 1997, pgm,      0,          pgm, pgm,   0,          ROT0, "IGS", "PGM (Polygame Master) System BIOS", NOT_A_DRIVER )

GAMEX( 1997, orlegend, pgm,        pgm, pgm,   orlegend,   ROT0, "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 126)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1997, orlegnde, orlegend,   pgm, pgm,   orlegend,   ROT0, "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 112)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1997, orlegndc, orlegend,   pgm, pgm,   orlegend,   ROT0, "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 112, Chinese Board)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1997, orld111c, orlegend,   pgm, pgm,   orlegend,   ROT0, "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 111, Chinese Board)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1997, orld105k, orlegend,   pgm, pgm,   orlegend,   ROT0, "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 105, Korean Board)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAMEX( 1998, olds,     pgm,        olds, olds, olds,       ROT0, "IGS", "Oriental Legend Special - Xi You Shi E Zhuan Super (ver. 101, Korean Board)", GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1998, olds103t, olds,       pgm,  pgm,  olds103t,   ROT0, "IGS", "Oriental Legend Special / Xi You Shi E Zhuan Super (ver. 103, China, Tencent) (unprotected)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

GAMEX( 1997, drgw2,    pgm,        pgm, pgm,   drgw2,      ROT0, "IGS", "Dragon World II (ver. 110X, Export)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1997, drgw2c,   drgw2,      pgm, pgm,   drgw2c,     ROT0, "IGS", "Zhong Guo Long II (ver. 100C, China)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1999, kov,      pgm,        pgm, sango, kov, 	   ROT0, "IGS", "Knights of Valour / Sangoku Senki (ver. 117)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) /* ver # provided by protection? */
GAMEX( 1999, kov115,   kov,        pgm, sango, kov, 	   ROT0, "IGS", "Knights of Valour / Sangoku Senki (ver. 115)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) /* ver # provided by protection? */
GAMEX( 1999, kovj,     kov,        pgm, sango, kov, 	   ROT0, "IGS", "Knights of Valour / Sangoku Senki (ver. 100, Japanese Board)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) /* ver # provided by protection? */
GAMEX( 1999, kovplus,  kov,        pgm, sango, kov, 	   ROT0, "IGS", "Knights of Valour Plus / Sangoku Senki Plus (ver. 119)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1999, kovplusa, kov,        pgm, sango, kov, 	   ROT0, "IGS", "Knights of Valour Plus / Sangoku Senki Plus (alt ver. 119)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1999, photoy2k, pgm,        pgm, photoy2k, djlzz,   ROT0, "IGS", "Photo Y2K", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1999, raf102j,  photoy2k,   pgm, photoy2k, djlzz,   ROT0, "IGS", "Real and Fake / Photo Y2K (ver. 102, Japan Board)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 1999, puzlstar, pgm,        pgm, sango, pstar,      ROT0, "IGS", "Puzzle Star", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION )
GAMEX( 1998, killbld,  pgm,      killbld, killbld, killbld,ROT0, "IGS", "The Killing Blade (ver. 109, Chinese Board)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

// protection simulated, but should be correct
GAMEX( 1999, puzzli2,  pgm,        pgm,  puzzli2, puzzli2, ROT0, "IGS", "Puzzli 2 (ver. 100)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) // ROM label is V100 ( V0001, 11/22/99 09:27:58 in program ROM )
GAMEX( 2001, puzzli2s, puzzli2,    pgm,  puzzli2, puzzli2, ROT0, "IGS", "Puzzli 2 Super (ver. 200)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )  // ( V200, 12/28/01 12:53:34 in program ROM )
GAMEX( 2001, py2k2,    pgm,        pgm,  photoy2k, py2k2,  ROT0, "IGS", "Photo Y2K 2", GAME_IMPERFECT_GRAPHICS )  /* need internal rom of IGS027A */


/* not working */
GAMEX( 1998, drgw3,    pgm,        pgm, sango, dw3,	       ROT0, "IGS", "Dragon World 3", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAMEX( 1998, drgw3k,   drgw3,      pgm, sango, dw3,	       ROT0, "IGS", "Dragon World 3 (Korean Board)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAMEX( 1999, kovsh,    kov,        pgm, sango, kovsh,	   ROT0, "IGS", "Knights of Valour Superheroes / Sangoku Senki Superheroes (ver. 322)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )


/* Cave games on PGM hardware */

GAMEX( 2002, ddp3,    0,         cavepgm,    pgm,     ddp3,      ROT270, "Cave", "DoDonPachi Dai-Ou-Jou", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 2002, ddp3a,   ddp3,      cavepgm,    pgm,     ddp3,      ROT270, "Cave", "DoDonPachi Dai-Ou-Jou (V100, second revision)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) // Displays "2002.04.05.Master Ver"
GAMEX( 2002, ddp3b,   ddp3,      cavepgm,    pgm,     ddp3,      ROT270, "Cave", "DoDonPachi Dai-Ou-Jou (V100, first revision)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) // Displays "2002.04.05 Master Ver"
GAMEX( 2002, ddp3w,   ddp3,      cavepgm,    pgm,     ddp3,      ROT270, "Cave", "DoDonPachi III (World, Master Ver)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 2002, ddp3blk, ddp3,      cavepgm,    pgm,     ddp3blk,   ROT270, "Cave", "DoDonPachi Dai-Ou-Jou (Black Label)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

/* the exact text of the 'version' shows which revision of the game it is; the newest has 2 '.' symbols in the string, the oldest, none. */

GAMEX( 2002, espgal,  0,         cavepgm,    pgm,     espgal,    ROT270, "Cave", "EspGaluda", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAMEX( 2002, ket,     0,         cavepgm,    pgm,     ket,       ROT270, "Cave", "Ketsui: Kizuna Jigoku Tachi", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) /* Displays 2003/01/01. Master Ver */
GAMEX( 2002, keta,    ket,       cavepgm,    pgm,     ket,       ROT270, "Cave", "Ketsui: Kizuna Jigoku Tachi (older)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) /* Displays 2003/01/01 Master Ver */
GAMEX( 2002, ketb,    ket,       cavepgm,    pgm,     ket,       ROT270, "Cave", "Ketsui: Kizuna Jigoku Tachi (first revision)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) /* Displays 2003/01/01 Master Ver */

/*gamezfan updates some parts of the PGM driver plus PGM encryption/protection files to MAME 105.u5
Oriental Legend Special, Real And Fake/Photo Y2K, Puzzle Star, The Killing Blade, DDP3, ESP Galuda, 
Ketsui, Puzzli 2, puzzli 2 Super and Photo Y2K 2 are now playable.

The Killing Blade and the Cave shumps required some changes in the PGM video code i then
updated the ICS2115 soundcore vastly improving the music and sfx in the Cave PGM games and to a lesser
extent all the rest*/
#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()
