#pragma code_seg("C910")
#pragma data_seg("D910")
#pragma bss_seg("B910")
#pragma const_seg("K910")
#pragma comment(linker, "/merge:D910=910")
#pragma comment(linker, "/merge:C910=910")
#pragma comment(linker, "/merge:B910=910")
#pragma comment(linker, "/merge:K910=910")
/***************************************************************************

    Sega System 32/Multi 32 hardware

****************************************************************************

    Still to do:
        * fix protection
        * fix jpark correctly

****************************************************************************

Title Fight (Export Revision)
Sega, 1992

This game runs on Sega System Multi32 hardware.

PCB Layout
----------
837-8676  171-6253C (C) SEGA 1992
834-9324-02 (STICKER)
|-------------------------------------------------------------------------------------------|
|             CNF                                     CNH                          8464     |
|8464   HM53461ZP-12 (x16)      |-----------|    HM53461ZP-12 (x16)                8464     |
|8464                           | SEGA      |                                      8464     |
|8464                           | 315-5386A |                                      8464     |
|8464                           |           |                                               |
|                               |           |                                               |
|                               |           |                                               |
|   |---------|  |---------|    |-----------|                    |---------|  |---------|   |
|   |SEGA     |  |SEGA     |                                     |SEGA     |  |SEGA     |   |
|   |315-5388 |  |315-5387 |  HM53461ZP-12 (x8)                  |315-5388 |  |315-5242 |   |
|   |         |  |         |                                     |         |  |         |   |
|   |         |  |         |                                     |         |  |         |   |
|   |---------|  |---------|                                     |---------|  |---------|   |
|             CND                                     CNE                                   |
| |---------| MB3771 MB3771                      315-5596              |-------| M5M5278P-25|
| |SEGA     |                                                          |SEGA   |            |
| |315-5242 |      8464                                                |315-   | M5M5278P-25|
| |         |BAT_3.3V                                                  |5591   |            |
| |         |BAT_5.5V                     |---------|                  |-------| M5M5278P-25|
| |---------|         Z80    50MHz        |SEGA     |     |-----------|                 DSW1|
|                            32MHz        |315-5385 |     |NEC        |          M5M5278P-25|
| |------|                                |         |     |D70632R-20 |                 SW4 |
| |SEGA  |                   |-------|    |         |     |9314X9 V70 |    40MHz        SW3 |
| |315-  |                   |SEGA   |    |---------|     |(C)NEC 1986|                     |
| |5560  | YM2612            |315-   |                    |           |                     |
| |------| LC78820           |5296   |                    |-----------|              93C45  |
|TDA1518                     |-------|                                   |-------|          |
|                                       SW1    SW2                       |SEGA   |       CNM|
|  VOL                                                                   |315-   |          |
|                                                                        |5296   |          |
| UPC844 UPC4064                                                         |-------|          |
|                                                                                           |
|CNC                                                      LED                               |
--|            JAMMA             |----|     CNJ      |----|            JAMMA             |--|
  |------------------------------|    |--------------|    |------------------------------|
Notes:
      V70 CPU running at 20.00MHz [40/2]
      Z80 CPU running at 8.000MHz [32/4]
      YM2612 running at 8.000MHz [32/4]
      315-5560 running at 8.000MHz [32/4]
      CND/E/F/H: Multi-pin connectors for connection of ROM Board
      CNC      : 4 pin connector for 2nd Speaker for Stereo Output
      CNJ      : 32 pin connector (purpose unknown)
      CNM      : 30 pin connector (purpose unknown)
      SW1      : push-button TEST switch
      SW2      : push button SERVICE switch
      SW3/4    : push button switches (purpose unknown)
      DSW1     : 4-position DIP Switch

      SEGA Custom ICs:
                      315-5596  Lattice GAL16V8A (DIP20)
                      315-5386A (QFP184)
                      315-5388  (x2, QFP160)
                      315-5387  (QFP160)
                      315-5242  (x2, custom ceramic DIP32 containing a small QFP IC and some capacitors/resistors etc)
                      315-5296  (x2, QFP100)
                      315-5560  (QFP80)
                      315-5385  (QFP128)
                      315-5591  (QFP100)


ROM Board
---------
837-8890
|-------------------------------------------------------------------------------------------|
|             CN3                                     CN4                                   |
|                                                                                           |
|             IC3                          IC14            IC15              IC10           |
| IC1   IC2                   JP1-5                                                         |
|                                                                                           |
|             IC11            JP6-10       IC22            IC23              IC18           |
|                                                                                           |
|                                                                                           |
|                                          IC36            IC37                             |
|             315-5598                                                       IC38           |
|                  CN5                                                                      |
| IC30  IC31                               IC39            IC40                             |
|                                                                CN6         IC41           |
|JP11-16      CN1                                     CN2                                   |
|-------------------------------------------------------------------------------------------|

Notes:
CN1/2/3/4 : Multi-pin connectors (below PCB) for connection of ROM Board to Main Board
CN5       : Custom Sega connector for connection of protection daughterboard (not used)
CN6       : Custom Sega connector (purpose unknown)
315-5598  : Lattice GAL16V8A (DIP20)

Jumpers: (to configure ROM sizes used on Title Fight. Other games may vary depending on ROM types used.)
JP1: 1-2
JP2: 1-2
JP3: 2-3
JP4: 2-3
JP5: 2-3
JP6: 1-2
JP7: 2-3
JP8: 2-3
JP9: 1-2
JP10: 1-2
JP11: OPEN
JP12: OPEN
JP13: OPEN
JP14: OPEN
JP15: 1-2
JP16: 2-3

ROM Locations (All known System Multi32 games shown for reference)
-------------
PCB Label     CPU P0    CPU P1    CPU D0    CPU D1    SOUND     SCR0      SCR1      OBJ0      OBJ1      OBJ2      OBJ3      OBJ4      OBJ5      OBJ6      OBJ7      PCM D0    PCM D1    PCM EP/S
IC#           IC37      IC40      IC36      IC39      IC31      IC3       IC11      IC14      IC22      IC15      IC23      IC10      IC18      IC38      IC41      IC1       IC2       IC30

IC Type       27C2048   27C2048   -         -         27C1000   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   -         -
Title Fight   EPR15388  EPR15389  -         -         EPR15384  MPR15381  MPR15382  MPR15379  MPR15380  MPR15375  MPR15376  MPR15371  MPR15372  MPR15373  MPR15374  MPR15385  -         -

IC Type       27C1024   27C1024   27C4200   27C4200   27C040    8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   -
OutRunners    EPR15618  EPR15619  EPR15538  EPR15539  EPR15550  MPR15548  MPR15549  MPR15540  MPR15541  MPR15542  MPR15543  MPR15544  MPR15545  MPR15546  MPR15547  MPR15551  MPR15552  -

IC Type       27C2048   27C2048   -         -         27C1000   534200    534200    8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   8316200   -
Hard Dunk     EPR16512  EPR16509  -         -         EPR16505  MPR16503  MPR16504  MPR16495  MPR16496  MPR16497  MPR16498  MPR16499  MPR16500  MPR16501  MPR16502  MPR16506  MPR16507  -
Hard Dunk (J) EPR16508  EPR16509  -         -         EPR16505  MPR16503  MPR16504  MPR16495  MPR16496  MPR16497  MPR16498  MPR16499  MPR16500  MPR16501  MPR16502  MPR16506  MPR16507  -

IC Type       27C2048   27C2048   27C4200   27C4200   27C1000   27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    27C160    -
Stadium Cross EPR15093  EPR15094  EPR15018  EPR15019  EPR15192  EPR15020  EPR15021  EPR15022  EPR15023  EPR15024  EPR15025  EPR15026  EPR15027  EPR15028  EPR15029  EPR15031  EPR15032  -


***************************************************************************/

#include "driver.h"
#include "segas32.h"
#include "vidhrdw/generic.h"
#include "machine/eeprom.h"
#include "machine/random.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK		32215900
#define RFC_CLOCK			50000000// incorrect for the older sound core im using the RFC clockspeed from system32

#define TIMER_0_CLOCK		((MASTER_CLOCK/2)/2048)	/* confirmed */
#define TIMER_1_CLOCK		((RFC_CLOCK/16)/256)	/* confirmed */

#define MAIN_IRQ_VBSTART	0
#define MAIN_IRQ_VBSTOP		1
#define MAIN_IRQ_SOUND		2
#define MAIN_IRQ_TIMER0		3
#define MAIN_IRQ_TIMER1		4

#define SOUND_IRQ_YM3438	0
#define SOUND_IRQ_V60		1



/*************************************
 *
 *  Global variables
 *
 *************************************/

data8_t *ga2b_dpram;
data16_t *system32b_workram;
data16_t *system32b_protram;



/*************************************
 *
 *  Statics
 *
 *************************************/

static const UINT8 *system32b_default_eeprom;
static data8_t *z80_shared_ram;

/* V60 interrupt controller */
static UINT8 v60_irq_control[0x10];
static mame_timer *v60_irq_timer[2];

/* sound interrupt controller */
static UINT8 sound_irq_control[4];
static UINT8 sound_irq_input;
static UINT8 sound_dummy_value;
static UINT16 sound_bank;

/* I/O chips and custom I/O */
static UINT8 misc_io_data[2][0x10];
static read16_handler custom_io_r[2];
static write16_handler custom_io_w[2];
static UINT8 analog_bank;
static UINT8 analog_value[4];
static UINT8 sonic_last[6];

static void (*system32b_prot_vblank)(void);


/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void signal_v60_irq(int which);
static void signal_sound_irq(int which);



/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_INIT( system32b )
{
	/* initialize the interrupt controller */
	memset(v60_irq_control, 0xff, sizeof(v60_irq_control));

	/* allocate timers */
	v60_irq_timer[0] = timer_alloc(signal_v60_irq);
	v60_irq_timer[1] = timer_alloc(signal_v60_irq);

	/* clear IRQ lines */
	cpunum_set_input_line(0, 0, CLEAR_LINE);
}



/*************************************
 *
 *  Interrupt controller
 *
 *************************************/

static void update_irq_state(void)
{
	UINT8 effirq = v60_irq_control[7] & ~v60_irq_control[6] & 0x1f;
	int vector;

	/* loop over interrupt vectors, finding the highest priority one with */
	/* an unmasked interrupt pending */
	for (vector = 0; vector < 5; vector++)
		if (effirq & (1 << vector))
		{
			cpunum_set_input_line_and_vector(0, 0, ASSERT_LINE, vector);
			break;
		}

	/* if we didn't find any, clear the interrupt line */
	if (vector == 5)
		cpunum_set_input_line(0, 0, CLEAR_LINE);
}


static void signal_v60_irq(int which)
{
	int i;

	/* see if this interrupt input is mapped to any vectors; if so, mark them */
	for (i = 0; i < 5; i++)
		if (v60_irq_control[i] == which)
			v60_irq_control[7] |= 1 << i;
	update_irq_state();
}


static void int_control_w(int offset, UINT8 data)
{
	int duration;

//  logerror("%06X:int_control_w(%X) = %02X\n", activecpu_get_pc(), offset, data);
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:			/* vectors */
			v60_irq_control[offset] = data;
			break;

		case 5:			/* unknown */
			v60_irq_control[offset] = data;
			break;

		case 6:			/* mask */
			v60_irq_control[offset] = data;
			update_irq_state();
			break;

		case 7:			/* acknowledge */
			v60_irq_control[offset] &= data;
			update_irq_state();
			break;

		case 8:
		case 9:			/* timer 0 count */
			v60_irq_control[offset] = data;
			duration = v60_irq_control[8] + ((v60_irq_control[9] << 8) & 0xf00);
			if (duration)
				timer_adjust(v60_irq_timer[0], TIME_IN_HZ(TIMER_0_CLOCK) * duration, MAIN_IRQ_TIMER0, TIME_NEVER);
			break;

		case 10:
		case 11:		/* timer 1 count */
			v60_irq_control[offset] = data;
			duration = v60_irq_control[10] + ((v60_irq_control[11] << 8) & 0xf00);
			if (duration)
				timer_adjust(v60_irq_timer[1], TIME_IN_HZ(TIMER_1_CLOCK) * duration, MAIN_IRQ_TIMER1, TIME_NEVER);
			break;

		case 12:
		case 13:
		case 14:
		case 15:		/* signal IRQ to sound CPU */
			signal_sound_irq(SOUND_IRQ_V60);
			break;
	}
}


static READ16_HANDLER( interrupt_control_16_r )
{
	switch (offset)
	{
		case 8/2:
			/* fix me - should return timer count down value */
			break;

		case 10/2:
			/* fix me - should return timer count down value */
			break;
	}

	/* return all F's for everything except timer values */
	return 0xffff;
}


static WRITE16_HANDLER( interrupt_control_16_w )
{
	if (ACCESSING_LSB)
		int_control_w(offset*2+0, data);
	if (ACCESSING_MSB)
		int_control_w(offset*2+1, data >> 8);
}


static READ32_HANDLER( interrupt_control_32_r )
{
	switch (offset)
	{
		case 8/4:
			/* fix me - should return timer count down value */
			break;
	}

	/* return all F's for everything except timer values */
	return 0xffffffff;
}


static WRITE32_HANDLER( interrupt_control_32_w )
{
	if (!(mem_mask & 0x000000ff))
		int_control_w(offset*4+0, data);
	if (!(mem_mask & 0x0000ff00))
		int_control_w(offset*4+1, data >> 8);
	if (!(mem_mask & 0x00ff0000))
		int_control_w(offset*4+2, data >> 16);
	if (!(mem_mask & 0xff000000))
		int_control_w(offset*4+3, data >> 24);
}


static void end_of_vblank_int(int param)
{
	signal_v60_irq(MAIN_IRQ_VBSTOP);
	system32b_set_vblank(0);
}


static INTERRUPT_GEN( start_of_vblank_int )
{
	signal_v60_irq(MAIN_IRQ_VBSTART);
	system32b_set_vblank(1);
	timer_set(cpu_getscanlinetime(0), 0, end_of_vblank_int);
		if (system32b_prot_vblank)
		(*system32b_prot_vblank)();
}



/*************************************
 *
 *  I/O chip
 *
 *************************************/

static data16_t common_io_chip_r(int which, offs_t offset, data16_t mem_mask)
{
	offset &= 0x1f/2;

	switch (offset)
	{
		/* I/O ports */
		case 0x00/2:
		case 0x02/2:
		case 0x04/2:
		case 0x06/2:
		case 0x08/2:
		case 0x0c/2:
		case 0x0e/2:
			/* if the port is configured as an output, return the last thing written */
			if (misc_io_data[which][0x1e/2] & (1 << offset))
				return misc_io_data[which][offset];

			/* otherwise, return an input port */
			return readinputport(which*8 + offset);

		/* I/O port with EEPROM data bit */
		case 0x0a/2:
			/* if the port is configured as an output, return the last thing written */
			if (misc_io_data[which][0x1e/2] & (1 << offset))
				return misc_io_data[which][offset];

			/* otherwise, return an input port */
			return readinputport(which*8 + offset) | (EEPROM_read_bit() << 7);

		/* 'SEGA' protection */
		case 0x10/2:
			return 'S';
		case 0x12/2:
			return 'E';
		case 0x14/2:
			return 'G';
		case 0x16/2:
			return 'A';

		/* CNT register & mirror */
		case 0x18/2:
		case 0x1c/2:
			return misc_io_data[which][0x1c/2];

		/* port direction register & mirror */
		case 0x1a/2:
		case 0x1e/2:
			return misc_io_data[which][0x1e/2];
	}
	return 0xffff;
}


static void common_io_chip_w(int which, offs_t offset, data16_t data, data16_t mem_mask)
{
	UINT8 old;

	/* only LSB matters */
	if (!ACCESSING_LSB)
		return;

	/* generic implementation */
	offset &= 0x1f/2;
	old = misc_io_data[which][offset];
	misc_io_data[which][offset] = data;

	switch (offset)
	{
		/* I/O ports */
		case 0x00/2:
		case 0x02/2:
		case 0x04/2:
		case 0x08/2:
		case 0x0a/2:
		case 0x0c/2:
			break;

		/* miscellaneous output */
		case 0x06/2:
			if (which == 0)
			{
				EEPROM_write_bit(data & 0x80);
				EEPROM_set_cs_line((data & 0x20) ? CLEAR_LINE : ASSERT_LINE);
				EEPROM_set_clock_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
			}
/*            coin_lockout_w(1 + 2*which, data & 0x08);
            coin_lockout_w(0 + 2*which, data & 0x04);*/
			coin_counter_w(1 + 2*which, data & 0x02);
			coin_counter_w(0 + 2*which, data & 0x01);
			break;

		/* tile banking */
		case 0x0e/2:
			if (which == 0)
				system32b_tilebank_external = data;
			break;

		/* CNT register */
		case 0x1c/2:
			system32b_displayenable[which] = (data & 0x02);
			if (which == 0)
				cpunum_set_input_line(1, INPUT_LINE_RESET, (data & 0x04) ? CLEAR_LINE : ASSERT_LINE);
			break;
	}
}


static READ16_HANDLER( io_chip_r )
{
	return common_io_chip_r(0, offset, mem_mask);
}


static WRITE16_HANDLER( io_chip_w )
{
	common_io_chip_w(0, offset, data, mem_mask);
}


static READ32_HANDLER( io_chip_0_r )
{
	return common_io_chip_r(0, offset*2+0, mem_mask) |
	      (common_io_chip_r(0, offset*2+1, mem_mask >> 16) << 16);
}


static WRITE32_HANDLER( io_chip_0_w )
{
	if ((mem_mask & 0x0000ffff) != 0x0000ffff)
		common_io_chip_w(0, offset*2+0, data, mem_mask);
	if ((mem_mask & 0xffff0000) != 0xffff0000)
		common_io_chip_w(0, offset*2+1, data >> 16, mem_mask >> 16);
}


static READ32_HANDLER( io_chip_1_r )
{
	return common_io_chip_r(1, offset*2+0, mem_mask) |
	      (common_io_chip_r(1, offset*2+1, mem_mask >> 16) << 16);
}


static WRITE32_HANDLER( io_chip_1_w )
{
	if ((mem_mask & 0x0000ffff) != 0x0000ffff)
		common_io_chip_w(1, offset*2+0, data, mem_mask);
	if ((mem_mask & 0xffff0000) != 0xffff0000)
		common_io_chip_w(1, offset*2+1, data >> 16, mem_mask >> 16);
}



/*************************************
 *
 *  I/O expansion range
 *
 *************************************/

static READ16_HANDLER( io_expansion_r )
{
	if (custom_io_r[0])
		return (*custom_io_r[0])(offset, mem_mask);
	else
		logerror("%06X:io_expansion_r(%X)\n", activecpu_get_pc(), offset);
	return 0xffff;
}


static WRITE16_HANDLER( io_expansion_w )
{
	/* only LSB matters */
	if (!ACCESSING_LSB)
		return;

	if (custom_io_w[0])
		(*custom_io_w[0])(offset, data, mem_mask);
	else
		logerror("%06X:io_expansion_w(%X) = %02X\n", activecpu_get_pc(), offset, data & 0xff);
}


static READ32_HANDLER( io_expansion_0_r )
{
	if (custom_io_r[0])
		return (*custom_io_r[0])(offset*2+0, mem_mask) |
			  ((*custom_io_r[0])(offset*2+1, mem_mask >> 16) << 16);
	else
		logerror("%06X:io_expansion_r(%X)\n", activecpu_get_pc(), offset);
	return 0xffffffff;
}


static WRITE32_HANDLER( io_expansion_0_w )
{
	/* only LSB matters */
	if ((mem_mask & 0x000000ff) != 0x000000ff)
	{
		if (custom_io_w[0])
			(*custom_io_w[0])(offset*2+0, data, mem_mask);
		else
			logerror("%06X:io_expansion_w(%X) = %02X\n", activecpu_get_pc(), offset, data & 0xff);
	}
	if ((mem_mask & 0x00ff0000) != 0x00ff0000)
	{
		if (custom_io_w[0])
			(*custom_io_w[0])(offset*2+1, data >> 16, mem_mask >> 16);
		else
			logerror("%06X:io_expansion_w(%X) = %02X\n", activecpu_get_pc(), offset, data & 0xff);
	}
}


static READ32_HANDLER( io_expansion_1_r )
{
	if (custom_io_r[1])
		return (*custom_io_r[1])(offset*2+0, mem_mask) |
			  ((*custom_io_r[1])(offset*2+1, mem_mask >> 16) << 16);
	else
		logerror("%06X:io_expansion_r(%X)\n", activecpu_get_pc(), offset);
	return 0xffffffff;
}


static WRITE32_HANDLER( io_expansion_1_w )
{
	/* only LSB matters */
	if ((mem_mask & 0x000000ff) != 0x000000ff)
	{
		if (custom_io_w[1])
			(*custom_io_w[1])(offset*2+0, data, mem_mask);
		else
			logerror("%06X:io_expansion_w(%X) = %02X\n", activecpu_get_pc(), offset, data & 0xff);
	}
	if ((mem_mask & 0x00ff0000) != 0x00ff0000)
	{
		if (custom_io_w[1])
			(*custom_io_w[1])(offset*2+1, data >> 16, mem_mask >> 16);
		else
			logerror("%06X:io_expansion_w(%X) = %02X\n", activecpu_get_pc(), offset, data & 0xff);
	}
}



/*************************************
 *
 *  Game-specific custom I/O
 *
 *************************************/

static READ16_HANDLER( analog_custom_io_r )
{
	data16_t result;
	switch (offset)
	{
		case 0x10/2:
		case 0x12/2:
		case 0x14/2:
		case 0x16/2:
			result = analog_value[offset & 3] | 0x7f;
			analog_value[offset & 3] <<= 1;
			return result;
	}
	logerror("%06X:unknown analog_custom_io_r(%X) & %04X\n", activecpu_get_pc(), offset*2, mem_mask ^ 0xffff);
	return 0xffff;
}


static WRITE16_HANDLER( analog_custom_io_w )
{
	static const char *names[] = { "ANALOG1", "ANALOG2", "ANALOG3", "ANALOG4" };
	switch (offset)
	{
		case 0x10/2:
		case 0x12/2:
		case 0x14/2:
		case 0x16/2:
			analog_value[offset & 3] = readinputportbytag(names[offset & 3]);
			return;
	}
	logerror("%06X:unknown analog_custom_io_w(%X) = %04X & %04X\n", activecpu_get_pc(), offset*2, data, mem_mask ^ 0xffff);
}


static READ16_HANDLER( extra_custom_io_r )
{
	static const char *names[] = { "EXTRA1", "EXTRA2", "EXTRA3", "EXTRA4" };
	switch (offset)
	{
		case 0x20/2:
		case 0x22/2:
		case 0x24/2:
		case 0x26/2:
			return readinputportbytag(names[offset & 3]);
	}

	logerror("%06X:unknown extra_custom_io_r(%X) & %04X\n", activecpu_get_pc(), offset*2, mem_mask ^ 0xffff);
	return 0xffff;
}

static READ16_HANDLER( sonicp_custom_io_r )
{
	static const char *names[] = { "TRACKX1", "TRACKY1", "TRACKX2", "TRACKY2", "TRACKX3", "TRACKY3" };

	switch (offset)
	{
		case 0x00/2:
		case 0x04/2:
		case 0x08/2:
		case 0x0c/2:
		case 0x10/2:
		case 0x14/2:
			return (UINT8)(readinputportbytag(names[offset/2]) - sonic_last[offset/2]);
	}

	logerror("%06X:unknown sonic_custom_io_r(%X) & %04X\n", activecpu_get_pc(), offset*2, mem_mask ^ 0xffff);
	return 0xffff;
}


static WRITE16_HANDLER( sonicp_custom_io_w )
{
	static const char *names[] = { "TRACKX1", "TRACKY1", "TRACKX2", "TRACKY2", "TRACKX3", "TRACKY3" };

	switch (offset)
	{
		case 0x00/2:
		case 0x08/2:
		case 0x10/2:
			sonic_last[offset/2 + 0] = readinputportbytag(names[offset/2 + 0]);
			sonic_last[offset/2 + 1] = readinputportbytag(names[offset/2 + 1]);
			return;
	}

	logerror("%06X:unknown sonic_custom_io_w(%X) = %04X & %04X\n", activecpu_get_pc(), offset*2, data, mem_mask ^ 0xffff);
}



/*************************************
 *
 *  Random number generator
 *
 *************************************/

static WRITE16_HANDLER( random_number_16_w )
{
//  printf("%06X:random_seed_w(%04X) = %04X & %04X\n", activecpu_get_pc(), offset*2, data, mem_mask  ^ 0xffff);
}

static READ16_HANDLER( random_number_16_r )
{
	return rand();
}

static WRITE32_HANDLER( random_number_32_w )
{
//  printf("%06X:random_seed_w(%04X) = %04X & %04X\n", activecpu_get_pc(), offset*2, data, mem_mask  ^ 0xffff);
}

static READ32_HANDLER( random_number_32_r )
{
	return rand() | (rand() << 16);
}



/*************************************
 *
 *  Sound communications
 *
 *************************************/

static READ16_HANDLER( shared_ram_16_r )
{
	return z80_shared_ram[offset*2+0] | (z80_shared_ram[offset*2+1] << 8);
}


static WRITE16_HANDLER( shared_ram_16_w )
{
	if (ACCESSING_LSB)
		z80_shared_ram[offset*2+0] = data;
	if (ACCESSING_MSB)
		z80_shared_ram[offset*2+1] = data >> 8;
}


static READ32_HANDLER( shared_ram_32_r )
{
	return z80_shared_ram[offset*4+0] | (z80_shared_ram[offset*4+1] << 8) |
	      (z80_shared_ram[offset*4+2] << 16) | (z80_shared_ram[offset*4+3] << 24);
}


static WRITE32_HANDLER( shared_ram_32_w )
{
	if (!(mem_mask & 0x000000ff))
		z80_shared_ram[offset*4+0] = data;
	if (!(mem_mask & 0x0000ff00))
		z80_shared_ram[offset*4+1] = data >> 8;
	if (!(mem_mask & 0x00ff0000))
		z80_shared_ram[offset*4+2] = data >> 16;
	if (!(mem_mask & 0xff000000))
		z80_shared_ram[offset*4+3] = data >> 24;
}



/*************************************
 *
 *  Sound interrupt controller
 *
 *************************************/

static void update_sound_irq_state(void)
{
	UINT8 effirq = sound_irq_input & ~sound_irq_control[3] & 0x07;
	int vector;

	/* loop over interrupt vectors, finding the highest priority one with */
	/* an unmasked interrupt pending */
	for (vector = 0; vector < 3; vector++)
		if (effirq & (1 << vector))
		{
			cpunum_set_input_line_and_vector(1, 0, ASSERT_LINE, 2 * vector);
			break;
		}

	/* if we didn't find any, clear the interrupt line */
	if (vector == 3)
		cpunum_set_input_line(1, 0, CLEAR_LINE);
}


static void signal_sound_irq(int which)
{
	int i;

	/* see if this interrupt input is mapped to any vectors; if so, mark them */
	for (i = 0; i < 3; i++)
		if (sound_irq_control[i] == which)
			sound_irq_input |= 1 << i;
	update_sound_irq_state();
}


static void clear_sound_irq(int which)
{
	int i;
	for (i = 0; i < 3; i++)
		if (sound_irq_control[i] == which)
			sound_irq_input &= ~(1 << i);
	update_sound_irq_state();
}


static WRITE8_HANDLER( sound_int_control_lo_w )
{
	/* odd offsets are interrupt acks */
	if (offset & 1)
	{
		sound_irq_input &= data;
		update_sound_irq_state();
	}

	/* high offsets signal an IRQ to the v60 */
	if (offset & 4)
		signal_v60_irq(MAIN_IRQ_SOUND);
}


static WRITE8_HANDLER( sound_int_control_hi_w )
{
	sound_irq_control[offset] = data;
	update_sound_irq_state();
}


static void ym3438_irq_handler(int state)
{
	if (state)
		signal_sound_irq(SOUND_IRQ_YM3438);
	else
		clear_sound_irq(SOUND_IRQ_YM3438);
}



/*************************************
 *
 *  Sound banking
 *
 *************************************/

static WRITE8_HANDLER( sound_bank_lo_w )
{
	sound_bank = (sound_bank & ~0x3f) | (data & 0x3f);
	memory_set_bankptr(1, memory_region(REGION_CPU2) + 0x100000 + 0x2000 * sound_bank);
}


static WRITE8_HANDLER( sound_bank_hi_w )
{
	sound_bank = (sound_bank & 0x3f) | ((data & 0x04) << 4) | ((data & 0x03) << 7);
	memory_set_bankptr(1, memory_region(REGION_CPU2) + 0x100000 + 0x2000 * sound_bank);
}



/*************************************
 *
 *  Sound hack (not protection)
 *
 *************************************/

static READ8_HANDLER( sound_dummy_r )
{
	return sound_dummy_value;
}


static WRITE8_HANDLER( sound_dummy_w )
{
	sound_dummy_value = data;
}



/*************************************
 *
 *  NVRAM handler
 *
 *************************************/

static NVRAM_HANDLER( system32b )
{
	if (read_or_write)
		EEPROM_save(file);
	else
	{
		EEPROM_init(&eeprom_interface_93C46);

		if (file)
			EEPROM_load(file);
		else if (system32b_default_eeprom != NULL)
			EEPROM_set_data(system32b_default_eeprom, 0x80);
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( system32b_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_MIRROR(0x0f0000) AM_RAM	AM_BASE(&system32b_workram)
	AM_RANGE(0x300000, 0x31ffff) AM_MIRROR(0x0e0000) AM_READWRITE(system32b_videoram_r, system32b_videoram_w) AM_BASE(&system32b_videoram)
	AM_RANGE(0x400000, 0x41ffff) AM_MIRROR(0x0e0000) AM_READWRITE(system32b_spriteram_r, system32b_spriteram_w) AM_BASE(&system32b_spriteram)
	AM_RANGE(0x500000, 0x50000f) AM_MIRROR(0x0ffff0) AM_READWRITE(system32b_sprite_control_r, system32b_sprite_control_w)
	AM_RANGE(0x600000, 0x60ffff) AM_MIRROR(0x0e0000) AM_READWRITE(system32b_paletteram_r, system32b_paletteram_w) AM_BASE(&system32b_paletteram[0])
	AM_RANGE(0x610000, 0x61007f) AM_MIRROR(0x0eff80) AM_READWRITE(system32b_mixer_r, system32b_mixer_w)
	AM_RANGE(0x700000, 0x701fff) AM_MIRROR(0x0fe000) AM_READWRITE(shared_ram_16_r, shared_ram_16_w)
	AM_RANGE(0xc00000, 0xc0001f) AM_MIRROR(0x0fff80) AM_READWRITE(io_chip_r, io_chip_w)
	AM_RANGE(0xc00040, 0xc0007f) AM_MIRROR(0x0fff80) AM_READWRITE(io_expansion_r, io_expansion_w)
	AM_RANGE(0xd00000, 0xd0000f) AM_MIRROR(0x07fff0) AM_READWRITE(interrupt_control_16_r, interrupt_control_16_w)
	AM_RANGE(0xd80000, 0xdfffff) AM_READWRITE(random_number_16_r, random_number_16_w)
	AM_RANGE(0xf00000, 0xffffff) AM_ROM AM_REGION(REGION_CPU1, 0)
ADDRESS_MAP_END


/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( system32b_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM AM_REGION(REGION_CPU2, 0x100000)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK(1)
	AM_RANGE(0xc000, 0xc00f) AM_MIRROR(0x0ff0) AM_WRITE(RF5C68_reg_w)
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(RF5C68_r, RF5C68_w)
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_BASE(&z80_shared_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( system32b_sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) | AMEF_ABITS(8) )
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x0c) AM_READWRITE(YM2612_status_port_0_A_r, YM2612_control_port_0_A_w)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x0c) AM_WRITE(YM2612_data_port_0_A_w)
	AM_RANGE(0x82, 0x82) AM_MIRROR(0x0c) AM_WRITE(YM2612_control_port_0_B_w)
	AM_RANGE(0x83, 0x83) AM_MIRROR(0x0c) AM_WRITE(YM2612_data_port_0_B_w)
	AM_RANGE(0x90, 0x90) AM_MIRROR(0x0c) AM_READWRITE(YM2612_status_port_1_A_r, YM2612_control_port_1_A_w)
	AM_RANGE(0x91, 0x91) AM_MIRROR(0x0c) AM_WRITE(YM2612_data_port_1_A_w)
	AM_RANGE(0x92, 0x92) AM_MIRROR(0x0c) AM_WRITE(YM2612_control_port_1_B_w)
	AM_RANGE(0x93, 0x93) AM_MIRROR(0x0c) AM_WRITE(YM2612_data_port_1_B_w)
	AM_RANGE(0xa0, 0xaf) AM_WRITE(sound_bank_lo_w)
	AM_RANGE(0xb0, 0xbf) AM_WRITE(sound_bank_hi_w)
	AM_RANGE(0xc0, 0xcf) AM_WRITE(sound_int_control_lo_w)
	AM_RANGE(0xd0, 0xd3) AM_MIRROR(0x04) AM_WRITE(sound_int_control_hi_w)
	AM_RANGE(0xf1, 0xf1) AM_READWRITE(sound_dummy_r, sound_dummy_w)
ADDRESS_MAP_END




/*************************************
 *
 *  GA2 Protection CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( ga2b_v25_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x0ffff) AM_ROM
	AM_RANGE(0x10000, 0x1ffff) AM_RAM AM_BASE(&ga2b_dpram)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION(REGION_CPU3, 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Generic port definitions
 *
 *************************************/
 

static INPUT_PORTS_START( darkedgeb )
	PORT_START_TAG("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )

	PORT_START_TAG("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )

	PORT_START_TAG("PORTC")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("PORTD")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("SERVICE12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("SERVICE34")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	/* sometimes mirrors SERVICE1 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 )	/* tends to also work as a test switch */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* EEPROM data */

	PORT_START_TAG("EXTRA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 | IPF_PLAYER1 )

	PORT_START_TAG("PORTG")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("PORTH")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ga2 )
	PORT_START_TAG("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )

	PORT_START_TAG("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	
	PORT_START_TAG("PORTC")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("PORTD")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	
	PORT_START_TAG("SERVICE12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("EXTRA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER3 )

	PORT_START_TAG("EXTRA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER4 )

	PORT_START_TAG("EXTRA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
	
	PORT_START_TAG("EXTRA4")

	PORT_START_TAG("SERVICE34")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	/* sometimes mirrors SERVICE1 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 )	/* tends to also work as a test switch */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* EEPROM data */

	PORT_START_TAG("PORTG")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("PORTH")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( ga2u )
	PORT_START_TAG("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )

	PORT_START_TAG("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER2 )
	
	PORT_START_TAG("PORTC")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("PORTD")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	
	PORT_START_TAG("SERVICE12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("EXTRA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER3 )

	PORT_START_TAG("EXTRA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_PLAYER4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_PLAYER4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_PLAYER4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_PLAYER4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_PLAYER4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_PLAYER4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_PLAYER4 )

	PORT_START_TAG("EXTRA3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
	
	PORT_START_TAG("EXTRA4")

	PORT_START_TAG("SERVICE34")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	/* sometimes mirrors SERVICE1 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 )	/* tends to also work as a test switch */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* EEPROM data */

	PORT_START_TAG("PORTG")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("PORTH")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( slipstrm )

	PORT_START_TAG("P1")
	PORT_BITX(0x01, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_TOGGLE, "Gear Change", IP_KEY_DEFAULT, IP_JOY_DEFAULT )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("PORTC")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("PORTD")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("SERVICE12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("SERVICE34")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	/* sometimes mirrors SERVICE1 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 )	/* tends to also work as a test switch */
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* EEPROM data */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	
	PORT_START_TAG("PORTG")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("PORTH")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("ANALOG1")
	PORT_ANALOG( 0xff, 0x80, IPT_PADDLE, 15, 10, 0, 255)

	PORT_START_TAG("ANALOG2")
	PORT_ANALOG( 0xff, 0x00, IPT_PEDAL, 30, 10, 0, 255)

	PORT_START_TAG("ANALOG3")
	PORT_ANALOG( 0xff, 0x00, IPT_PEDAL2, 30, 10, 0, 255)
	
	PORT_START_TAG("ANALOG4")
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static struct GfxLayout bgcharlayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0, 4, 16, 20, 8, 12, 24, 28, 32, 36, 48, 52, 40, 44, 56, 60  },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	  8*64, 9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};


static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &bgcharlayout,   0x00, 0x3ff  },
	{ -1 } /* end of array */
};



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/
 
// Older RFC System32 driver Clock value otherwise it's too fast

struct RF5C68interface sys32b_rf5c68_interface =
{
  9000000,		/* pitch matches real board, real speed is 8 MHz */
  55
};

struct YM2612interface sys32b_ym3438_interface =
{
	2,		/* 2 chips */
	MASTER_CLOCK/4,	/* verified on real PCB */
	{ 40,40 },
	{ 0 },	{ 0 },	{ 0 },	{ 0 },
	{ ym3438_irq_handler }
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( system32b )

	/* basic machine hardware */
    MDRV_CPU_ADD(V60, 10000000 )// should be 16mhz but make it this to speed things up for the Xbox
	MDRV_CPU_PROGRAM_MAP(system32b_map,0)
	MDRV_CPU_VBLANK_INT(start_of_vblank_int,1)

	MDRV_CPU_ADD_TAG("sound", Z80, MASTER_CLOCK/4)
	MDRV_CPU_PROGRAM_MAP(system32b_sound_map,0)
	MDRV_CPU_IO_MAP(system32b_sound_portmap,0)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(1000000 * (262 - 224) / (262 * 60))

	MDRV_MACHINE_INIT(system32b)
	MDRV_NVRAM_HANDLER(system32b)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_NEEDS_6BITS_PER_GUN | VIDEO_RGB_DIRECT)
	MDRV_SCREEN_SIZE(52*8, 28*8)
	MDRV_VISIBLE_AREA(0*8, 52*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x4000)

	MDRV_VIDEO_START(system32b)
	MDRV_VIDEO_UPDATE(system32b)

	/* sound hardware */
	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(YM3438, sys32b_ym3438_interface)
	MDRV_SOUND_ADD(RF5C68, sys32b_rf5c68_interface)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( system32b_v20 )
	MDRV_IMPORT_FROM(system32b)

	/* add a V25 for protection */
	MDRV_CPU_ADD(V20, 10000000)
	MDRV_CPU_PROGRAM_MAP(ga2b_v25_map,0)
MACHINE_DRIVER_END




/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

#define ROM_LOAD_x2(name, base, length, crc) \
	ROM_LOAD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 1 * length, length )

#define ROM_LOAD_x4(name, base, length, crc) \
	ROM_LOAD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 1 * length, length ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 3 * length, length )

#define ROM_LOAD_x8(name, base, length, crc) \
	ROM_LOAD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 1 * length, length ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 3 * length, length ) \
	ROM_RELOAD(     base + 4 * length, length ) \
	ROM_RELOAD(     base + 5 * length, length ) \
	ROM_RELOAD(     base + 6 * length, length ) \
	ROM_RELOAD(     base + 7 * length, length )

#define ROM_LOAD_x16(name, base, length, crc) \
	ROM_LOAD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 1 * length, length ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 3 * length, length ) \
	ROM_RELOAD(     base + 4 * length, length ) \
	ROM_RELOAD(     base + 5 * length, length ) \
	ROM_RELOAD(     base + 6 * length, length ) \
	ROM_RELOAD(     base + 7 * length, length ) \
	ROM_RELOAD(     base + 8 * length, length ) \
	ROM_RELOAD(     base + 9 * length, length ) \
	ROM_RELOAD(     base + 10 * length, length ) \
	ROM_RELOAD(     base + 11 * length, length ) \
	ROM_RELOAD(     base + 12 * length, length ) \
	ROM_RELOAD(     base + 13 * length, length ) \
	ROM_RELOAD(     base + 14 * length, length ) \
	ROM_RELOAD(     base + 15 * length, length )

#define ROM_LOAD16_BYTE_x2(name, base, length, crc) \
	ROM_LOAD16_BYTE( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 2 * length, length )

#define ROM_LOAD16_BYTE_x4(name, base, length, crc) \
	ROM_LOAD16_BYTE( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 4 * length, length ) \
	ROM_RELOAD(     base + 6 * length, length )

#define ROM_LOAD32_WORD_x2(name, base, length, crc) \
	ROM_LOAD32_WORD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 2 * length, length )

#define ROM_LOAD32_WORD_x4(name, base, length, crc) \
	ROM_LOAD32_WORD( name, base + 0 * length, length, crc ) \
	ROM_RELOAD(     base + 2 * length, length ) \
	ROM_RELOAD(     base + 4 * length, length ) \
	ROM_RELOAD(     base + 6 * length, length )


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Dark Edge
    protected via FD1149 317-0204
*/
ROM_START( darkedge )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* v60 code + data */
	ROM_LOAD_x2( "epr15246.8", 0x000000, 0x080000, CRC(c0bdceeb) SHA1(9cf670cf9a8691f259c75c1d9c6cb14e8a70bb72) )

	ROM_REGION( 0x500000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD_x8( "epr15243.36", 0x100000, 0x020000, CRC(08ca5f11) SHA1(c2c48d2f02770941a93794f82cb407d6264904d2) )
	ROM_LOAD( "mpr15242.35",    0x200000, 0x100000, CRC(ffb7d917) SHA1(bfeae1a2bd7250edb695b7034f6b1f851f6fd48a) )
	ROM_LOAD( "mpr15241.34",    0x300000, 0x100000, CRC(8eccc4fe) SHA1(119724b9b6d2b51ad4f065ebf74d200960090e68) )
	ROM_LOAD( "mpr15240.24",    0x400000, 0x100000, CRC(867d59e8) SHA1(fb1c0d26dbb1bde9d8bc86419cd911b8e37bf923) )

	ROM_REGION( 0x100000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD16_BYTE( "mpr15248", 0x000000, 0x080000, CRC(185b308b) SHA1(a49c1b752b3c4355560e0cd712fb9a096140e37b) )
	ROM_LOAD16_BYTE( "mpr15247", 0x000001, 0x080000, CRC(be21548c) SHA1(2e315aadc2a0b781c3ee3fe71c75eb1f43514eff) )

	ROM_REGION32_BE( 0x1000000, REGION_GFX2, 0 ) /* sprites */
	ROMX_LOAD( "mpr15249.32", 0x000000, 0x200000, CRC(2b4371a8) SHA1(47f448bfbc068f2d0cdedd81bcd280823d5758a3) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15251.30", 0x000002, 0x200000, CRC(efe2d689) SHA1(af22153ea3afdde3732f881087c642170f91d745) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15253.28", 0x000004, 0x200000, CRC(8356ed01) SHA1(a28747813807361c7d0c722a94e194caea8bfab6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15255.26", 0x000006, 0x200000, CRC(ff04a5b0) SHA1(d4548f9da014ba5249c2f75d654a2a88c095aaf8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15250.31", 0x800000, 0x200000, CRC(c5cab71a) SHA1(111c69c40a39c3fceef38f5876e1dcf7ac2fbee2) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15252.29", 0x800002, 0x200000, CRC(f8885fea) SHA1(ef944df5f6fd64813734056ad2a150f518c75459) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15254.27", 0x800004, 0x200000, CRC(7765424b) SHA1(7cd4c275f6333beeea62dd65a769e11650c68923) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr15256.25", 0x800006, 0x200000, CRC(44c36b62) SHA1(4c7f2cc4347ef2126dcbf43e8dce8500e52b5f8e) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Golden Axe: The Return of Death Adder
    protected via a custom V20 with encrypted code
*/
ROM_START( ga2 )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr14961.b",        0x000000, 0x020000, CRC(d9cd8885) SHA1(dc9d1f01770bd23ba5959e300badbc5093a149bc) )
	ROM_LOAD_x4( "epr14958.b",        0x080000, 0x020000, CRC(0be324a3) SHA1(5e5f457548906453eaa8d326c353b47353eab73d) )
	ROM_LOAD16_BYTE_x2( "epr15148.b", 0x100000, 0x040000, CRC(c477a9fd) SHA1(a9d60f801c12fd067e5ad1801a92c84edd13bd08) )
	ROM_LOAD16_BYTE_x2( "epr15147.b", 0x100001, 0x040000, CRC(1bb676ea) SHA1(125ffd13204f48be23e20b281c42c2307888c40b) )

	ROM_REGION( 0x500000, REGION_CPU2, 0 ) /* sound CPU + banks */
	ROM_LOAD_x16( "epr14945", 0x100000, 0x010000, CRC(4781d4cb) SHA1(bd1b774b3cd0c3e0290c55e426f66d6820d21d0f) )
	ROM_LOAD( "mpr14944",     0x200000, 0x100000, CRC(fd4d4b86) SHA1(e14b9cd6004bf9ecd902e37b433b828241361b46) )
	ROM_LOAD( "mpr14943",     0x300000, 0x100000, CRC(24d40333) SHA1(38faf8f3eac317a163e93bd2247fe98189b13d2d) )
	ROM_LOAD( "mpr14942",     0x400000, 0x100000, CRC(a89b0e90) SHA1(e14c62418eb7f9a2deb2a6dcf635bedc1c73c253) )

	ROM_REGION( 0x100000, REGION_CPU3, 0 ) /* Protection CPU */
	ROM_LOAD( "epr14468", 0x00000, 0x10000, CRC(77634daa) SHA1(339169d164b9ed7dc3787b084d33effdc8e9efc1) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD16_BYTE( "mpr14948", 0x000000, 0x200000, CRC(75050d4a) SHA1(51d6bc9935abcf30af438e69c2cf4e09f57a803f) )
	ROM_LOAD16_BYTE( "mpr14947", 0x000001, 0x200000, CRC(b53e62f4) SHA1(5aa0f198e6eb070b77b0d180d30c0228a9bc691e) )

	ROM_REGION32_BE( 0x1000000, REGION_GFX2, 0 ) /* sprites */
	ROMX_LOAD( "mpr14949", 0x000000, 0x200000, CRC(152c716c) SHA1(448d16ea036b66e886119c00af543dfa5e53fd84) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14951", 0x000002, 0x200000, CRC(fdb1a534) SHA1(3126b595bf69bf9952fedf8f9c6743eb10489dc6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14953", 0x000004, 0x200000, CRC(33bd1c15) SHA1(4e16562e3357d4db54b20543073e8f1fd6f74b1f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14955", 0x000006, 0x200000, CRC(e42684aa) SHA1(12e0f18a11edb46f09e2e8c5c4ba14170d0cf00d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14950", 0x800000, 0x200000, CRC(15fd0026) SHA1(e918984bd60ad63172fe273b31cc9019100228c8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14952", 0x800002, 0x200000, CRC(96f96613) SHA1(4c9808866032dab0401de322c28242e8a8775457) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14954", 0x800004, 0x200000, CRC(39b2ac9e) SHA1(74f4c81d85ab9b4c5e8ae4b4d2c6dff766c482ca) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14956", 0x800006, 0x200000, CRC(298fca50) SHA1(16e09b19cc7be3dfc8e82b45348e6d1cf2ed5621) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Golden Axe: The Return of Death Adder (U.S.)
    protected via a custom V20 with encrypted code
*/
ROM_START( ga2u )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr14960.b",        0x000000, 0x020000, CRC(87182fea) SHA1(bb669ea7091f1ea34589a565490effa934ca44a3) )
	ROM_LOAD_x4( "epr14957.b",        0x080000, 0x020000, CRC(ab787cf4) SHA1(7e19bb3e5d587b5009efc9f9fa52aecaef0eedc4) )
	ROM_LOAD16_BYTE_x2( "epr15146.b", 0x100000, 0x040000, CRC(7293d5c3) SHA1(535a8b4b4a05546b321cee8de6733edfc1f71589) )
	ROM_LOAD16_BYTE_x2( "epr15145.b", 0x100001, 0x040000, CRC(0da61782) SHA1(f0302d747e5d55663095bb38732af423104c33ea) )

	ROM_REGION( 0x500000, REGION_CPU2, 0 ) /* sound CPU + banks */
	ROM_LOAD_x16( "epr14945", 0x100000, 0x010000, CRC(4781d4cb) SHA1(bd1b774b3cd0c3e0290c55e426f66d6820d21d0f) )
	ROM_LOAD( "mpr14944",     0x200000, 0x100000, CRC(fd4d4b86) SHA1(e14b9cd6004bf9ecd902e37b433b828241361b46) )
	ROM_LOAD( "mpr14943",     0x300000, 0x100000, CRC(24d40333) SHA1(38faf8f3eac317a163e93bd2247fe98189b13d2d) )
	ROM_LOAD( "mpr14942",     0x400000, 0x100000, CRC(a89b0e90) SHA1(e14c62418eb7f9a2deb2a6dcf635bedc1c73c253) )

	ROM_REGION( 0x100000, REGION_CPU3, 0 ) /* Protection CPU */
	ROM_LOAD( "epr14468", 0x00000, 0x10000, CRC(77634daa) SHA1(339169d164b9ed7dc3787b084d33effdc8e9efc1) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD16_BYTE( "mpr14948", 0x000000, 0x200000, CRC(75050d4a) SHA1(51d6bc9935abcf30af438e69c2cf4e09f57a803f) )
	ROM_LOAD16_BYTE( "mpr14947", 0x000001, 0x200000, CRC(b53e62f4) SHA1(5aa0f198e6eb070b77b0d180d30c0228a9bc691e) )

	ROM_REGION32_BE( 0x1000000, REGION_GFX2, 0 ) /* sprites */
	ROMX_LOAD( "mpr14949", 0x000000, 0x200000, CRC(152c716c) SHA1(448d16ea036b66e886119c00af543dfa5e53fd84) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14951", 0x000002, 0x200000, CRC(fdb1a534) SHA1(3126b595bf69bf9952fedf8f9c6743eb10489dc6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14953", 0x000004, 0x200000, CRC(33bd1c15) SHA1(4e16562e3357d4db54b20543073e8f1fd6f74b1f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14955", 0x000006, 0x200000, CRC(e42684aa) SHA1(12e0f18a11edb46f09e2e8c5c4ba14170d0cf00d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14950", 0x800000, 0x200000, CRC(15fd0026) SHA1(e918984bd60ad63172fe273b31cc9019100228c8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14952", 0x800002, 0x200000, CRC(96f96613) SHA1(4c9808866032dab0401de322c28242e8a8775457) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14954", 0x800004, 0x200000, CRC(39b2ac9e) SHA1(74f4c81d85ab9b4c5e8ae4b4d2c6dff766c482ca) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14956", 0x800006, 0x200000, CRC(298fca50) SHA1(16e09b19cc7be3dfc8e82b45348e6d1cf2ed5621) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END

/**************************************************************************************************************************
    Golden Axe: The Return of Death Adder (Japan)
    protected via a custom V20 with encrypted code
*/
ROM_START( ga2j )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* v60 code + data */
	ROM_LOAD_x4( "epr14956.17",        0x000000, 0x020000, CRC(f1929177) SHA1(7dc39c40eff9fb46c2e51d1e83478cd6970e3951) )
	ROM_LOAD_x4( "epr14946.8",         0x080000, 0x020000, CRC(eacafe94) SHA1(d41a7e1ee2df9e053b559be0a1a6d2ae520fd3e4) )
	ROM_LOAD16_BYTE_x2( "epr14941.18", 0x100000, 0x040000, CRC(0ffb8203) SHA1(b27dce634d203af8abb6ddfb656d4c48eb54af01) )
	ROM_LOAD16_BYTE_x2( "epr14940.9",  0x100001, 0x040000, CRC(3b5b3084) SHA1(ea17f6b7fd413fe3808f822cec84c993c9b75aa2) )

	ROM_REGION( 0x500000, REGION_CPU2, 0 ) /* sound CPU + banks */
	ROM_LOAD_x16( "epr14945", 0x100000, 0x010000, CRC(4781d4cb) SHA1(bd1b774b3cd0c3e0290c55e426f66d6820d21d0f) )
	ROM_LOAD( "mpr14944",     0x200000, 0x100000, CRC(fd4d4b86) SHA1(e14b9cd6004bf9ecd902e37b433b828241361b46) )
	ROM_LOAD( "mpr14943",     0x300000, 0x100000, CRC(24d40333) SHA1(38faf8f3eac317a163e93bd2247fe98189b13d2d) )
	ROM_LOAD( "mpr14942",     0x400000, 0x100000, CRC(a89b0e90) SHA1(e14c62418eb7f9a2deb2a6dcf635bedc1c73c253) )

	ROM_REGION( 0x100000, REGION_CPU3, 0 ) /* Protection CPU */
	ROM_LOAD( "epr1446802.3", 0x00000, 0x10000, CRC(77634daa) SHA1(339169d164b9ed7dc3787b084d33effdc8e9efc1) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD16_BYTE( "mpr14948", 0x000000, 0x200000, CRC(75050d4a) SHA1(51d6bc9935abcf30af438e69c2cf4e09f57a803f) )
	ROM_LOAD16_BYTE( "mpr14947", 0x000001, 0x200000, CRC(b53e62f4) SHA1(5aa0f198e6eb070b77b0d180d30c0228a9bc691e) )

	ROM_REGION32_BE( 0x1000000, REGION_GFX2, 0 ) /* sprites */
	ROMX_LOAD( "mpr14949", 0x000000, 0x200000, CRC(152c716c) SHA1(448d16ea036b66e886119c00af543dfa5e53fd84) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14951", 0x000002, 0x200000, CRC(fdb1a534) SHA1(3126b595bf69bf9952fedf8f9c6743eb10489dc6) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14953", 0x000004, 0x200000, CRC(33bd1c15) SHA1(4e16562e3357d4db54b20543073e8f1fd6f74b1f) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14955", 0x000006, 0x200000, CRC(e42684aa) SHA1(12e0f18a11edb46f09e2e8c5c4ba14170d0cf00d) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14950", 0x800000, 0x200000, CRC(15fd0026) SHA1(e918984bd60ad63172fe273b31cc9019100228c8) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14952", 0x800002, 0x200000, CRC(96f96613) SHA1(4c9808866032dab0401de322c28242e8a8775457) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14954", 0x800004, 0x200000, CRC(39b2ac9e) SHA1(74f4c81d85ab9b4c5e8ae4b4d2c6dff766c482ca) , ROM_SKIP(6)|ROM_GROUPWORD )
	ROMX_LOAD( "mpr14956", 0x800006, 0x200000, CRC(298fca50) SHA1(16e09b19cc7be3dfc8e82b45348e6d1cf2ed5621) , ROM_SKIP(6)|ROM_GROUPWORD )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Slipstream
*/
ROM_START( slipstrm )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* v60 code + data */
	ROM_LOAD_x2( "slipstrm.u6",     0x000000, 0x080000, CRC(7d066307) SHA1(d87e04167263b435b77830db02ed58651ccc020c) )
	ROM_LOAD16_BYTE( "slipstrm.u14",0x100000, 0x080000, CRC(c3ff6309) SHA1(dcc857736fe0f15aa7909c3ee88a7e239c8f0228) )
	ROM_LOAD16_BYTE( "slipstrm.u7", 0x100001, 0x080000, CRC(0e605c81) SHA1(47c64195cab9a07b234d5a375d26168e53ffaa17) )

	ROM_REGION( 0x500000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD_x8( "slipstrm.u35", 0x100000, 0x020000, CRC(0fee2278) SHA1(7533a03c3fc46d65dfdd07bddf1e6e0bbc368752) )
	ROM_LOAD_x2( "slipstrm.u31", 0x200000, 0x080000, CRC(ae7be5f2) SHA1(ba089355e64864435bcc3b0c208e4bce1ea66295) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD32_BYTE( "slipstrm.u38", 0x000000, 0x080000, CRC(3cbb2d0b) SHA1(b94006347b72cd60a889b0e279f62f677cedfd2e) )
	ROM_LOAD32_BYTE( "slipstrm.u34", 0x000002, 0x080000, CRC(4167be55) SHA1(96b34d311b318c00c3fad917e341589a70ba0a15) )
	ROM_LOAD32_BYTE( "slipstrm.u29", 0x000001, 0x080000, CRC(52c4bb85) SHA1(4fbee1072a19c75c25b5fd269acc75640923d69c) )
	ROM_LOAD32_BYTE( "slipstrm.u25", 0x000003, 0x080000, CRC(4948604a) SHA1(d5a1b9781fef7976a59a0af9b755a04fcacf9381) )

	ROM_REGION32_BE( 0x400000, REGION_GFX2, 0 ) /* sprites */
	ROMX_LOAD( "slipstrm.u36", 0x000000, 0x80000, CRC(cffe9e0d) SHA1(5272d54ff142de927a9abd61f3646e963c7d22c4) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u32", 0x000001, 0x80000, CRC(4ebd1383) SHA1(ce35f4d15e7904bfde55e58cdde925cba8002763) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u27", 0x000002, 0x80000, CRC(b3cf4fe2) SHA1(e13199522e1e3e8b9cfe72cc29b33f25dad542ef) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u23", 0x000003, 0x80000, CRC(c6345391) SHA1(155758097911ffca0c5c0b2a24a8033339dcfcbb) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u37", 0x000004, 0x80000, CRC(2de4288e) SHA1(8e794f79f506293edb7609187a7908516ce76849) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u33", 0x000005, 0x80000, CRC(6cfb74fb) SHA1(b74c886959910cd069427418525b23300a9b7b18) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u28", 0x000006, 0x80000, CRC(53234bf4) SHA1(1eca538dcb86e44c31310ab1ab42a2b66b69c8fe) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u24", 0x000007, 0x80000, CRC(22c129cf) SHA1(0f64680511a357038f6a556253c13fbb5417dd1a) , ROM_SKIP(7) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Slipstream Hispanic
*/
ROM_START( slipstrh )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* v60 code + data */
	ROM_LOAD_x2( "s32h_prg01.ic6",    0x000000, 0x080000, CRC(ab778297) SHA1(e440d44b20f2f7478ef7d86af90af5eb7b9a545a) )
	ROM_LOAD16_BYTE( "slipstrm.u14",0x100000, 0x080000, CRC(c3ff6309) SHA1(dcc857736fe0f15aa7909c3ee88a7e239c8f0228) )
	ROM_LOAD16_BYTE( "slipstrm.u7", 0x100001, 0x080000, CRC(0e605c81) SHA1(47c64195cab9a07b234d5a375d26168e53ffaa17) )

	ROM_REGION( 0x500000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD_x8( "slipstrm.u35", 0x100000, 0x020000, CRC(0fee2278) SHA1(7533a03c3fc46d65dfdd07bddf1e6e0bbc368752) )
	ROM_LOAD_x2( "slipstrm.u31", 0x200000, 0x080000, CRC(ae7be5f2) SHA1(ba089355e64864435bcc3b0c208e4bce1ea66295) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD32_BYTE( "slipstrm.u38", 0x000000, 0x080000, CRC(3cbb2d0b) SHA1(b94006347b72cd60a889b0e279f62f677cedfd2e) )
	ROM_LOAD32_BYTE( "slipstrm.u34", 0x000002, 0x080000, CRC(4167be55) SHA1(96b34d311b318c00c3fad917e341589a70ba0a15) )
	ROM_LOAD32_BYTE( "slipstrm.u29", 0x000001, 0x080000, CRC(52c4bb85) SHA1(4fbee1072a19c75c25b5fd269acc75640923d69c) )
	ROM_LOAD32_BYTE( "slipstrm.u25", 0x000003, 0x080000, CRC(4948604a) SHA1(d5a1b9781fef7976a59a0af9b755a04fcacf9381) )

	ROM_REGION32_BE( 0x400000, REGION_GFX2, 0 ) /* sprites */
	ROMX_LOAD( "slipstrm.u36", 0x000000, 0x80000, CRC(cffe9e0d) SHA1(5272d54ff142de927a9abd61f3646e963c7d22c4) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u32", 0x000001, 0x80000, CRC(4ebd1383) SHA1(ce35f4d15e7904bfde55e58cdde925cba8002763) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u27", 0x000002, 0x80000, CRC(b3cf4fe2) SHA1(e13199522e1e3e8b9cfe72cc29b33f25dad542ef) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u23", 0x000003, 0x80000, CRC(c6345391) SHA1(155758097911ffca0c5c0b2a24a8033339dcfcbb) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u37", 0x000004, 0x80000, CRC(2de4288e) SHA1(8e794f79f506293edb7609187a7908516ce76849) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u33", 0x000005, 0x80000, CRC(6cfb74fb) SHA1(b74c886959910cd069427418525b23300a9b7b18) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u28", 0x000006, 0x80000, CRC(53234bf4) SHA1(1eca538dcb86e44c31310ab1ab42a2b66b69c8fe) , ROM_SKIP(7) )
	ROMX_LOAD( "slipstrm.u24", 0x000007, 0x80000, CRC(22c129cf) SHA1(0f64680511a357038f6a556253c13fbb5417dd1a) , ROM_SKIP(7) )
ROM_END


/*************************************
 *
 *  Common driver inits
 *
 *************************************/

static void common_init(read16_handler custom_r, write16_handler custom_w, const UINT8 *default_eeprom)
{
	/* reset the custom handlers and other pointers */
	custom_io_r[0] = custom_r;
	custom_io_w[0] = custom_w;
	system32b_default_eeprom = default_eeprom;
	system32b_prot_vblank = NULL;
}



/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/
 

static DRIVER_INIT( darkedge )
{
	common_init(extra_custom_io_r, NULL, NULL);

	/* install protection handlers */
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, 0xa00000, 0xa7ffff, 0, 0, darkedge_protection_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0xa00000, 0xa7ffff, 0, 0, darkedge_protection_w);
	system32b_prot_vblank = darkedge_fd1149_vblank;
}
 

static DRIVER_INIT( ga2 )
{
	common_init(extra_custom_io_r, NULL, NULL);

	decrypt_ga2b_protrom();
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0xa00000, 0xa00fff, 0, 0, ga2b_dpram_w);
	memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM,  0xa00000, 0xa00fff, 0, 0, ga2b_dpram_r);

}


static DRIVER_INIT( slipstrm )
{
	common_init(analog_custom_io_r, analog_custom_io_w, NULL);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/
GAMEX(1992, darkedge, 0,        system32b,     darkedgeb,darkedge, ROT0, "Sega",   "Dark Edge (World)", GAME_IMPERFECT_GRAPHICS )
GAMEX(1992, ga2,      0,        system32b_v20, ga2,      ga2,      ROT0, "Sega",   "Golden Axe: The Revenge of Death Adder (World)", GAME_IMPERFECT_GRAPHICS )
GAMEX(1992, ga2u,     ga2,      system32b_v20, ga2u,     ga2,      ROT0, "Sega",   "Golden Axe: The Revenge of Death Adder (US)", GAME_IMPERFECT_GRAPHICS )
GAMEX(1992, ga2j,     ga2,      system32b_v20, ga2,      ga2,      ROT0, "Sega",   "Golden Axe: The Revenge of Death Adder (Japan)", GAME_IMPERFECT_GRAPHICS )
GAMEX(1995, slipstrm, 0,        system32b,     slipstrm, slipstrm, ROT0, "Capcom", "Slipstream", GAME_IMPERFECT_GRAPHICS )
GAMEX(1995, slipstrh, slipstrm, system32b,     slipstrm, slipstrm, ROT0, "Capcom", "Slipstream (Hispanic)", GAME_IMPERFECT_GRAPHICS )
/*gamezfan partially ports the segas32 driver plus machine and video from MAME98 then added prot code for Dark Edge i then 
refactored much of the code to keep it seperate from the excisting system and multi32 drivers also had to massively underclock 
the main CPU to speed these games up for Xbox use luckly it doesn't affect anything all games in this driver are now playable*/
#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()