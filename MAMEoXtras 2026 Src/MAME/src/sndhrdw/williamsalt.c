/***************************************************************************

	Midway/williamsalt Audio Board
	---------------------------

	6809 MEMORY MAP

	Function                                  Address     R/W  Data
	---------------------------------------------------------------
	Program RAM                               0000-07FF   R/W  D0-D7

	Music (YM-2151)                           2000-2001   R/W  D0-D7

	6821 PIA                                  4000-4003   R/W  D0-D7

	HC55516 clock low, digit latch            6000        W    D0
	HC55516 clock high                        6800        W    xx

	Bank select                               7800        W    D0-D2

	Banked Program ROM                        8000-FFFF   R    D0-D7

****************************************************************************/

#include "driver.h"
#include "machine/6821pia.h"
#include "cpu/m6809/m6809.h"
#include "williamsalt.h"

#include <math.h>

/***************************************************************************
	COMPILER SWITCHES
****************************************************************************/

#define DISABLE_FIRQ_SPEEDUP		0
#define DISABLE_LOOP_CATCHERS		0

/***************************************************************************
	CONSTANTS (from HC55516.c)
****************************************************************************/

#define	INTEGRATOR_LEAK_TC		0.001
#define	FILTER_DECAY_TC			0.004
#define	FILTER_CHARGE_TC		0.004
#define	FILTER_MIN				0.0416
#define	FILTER_MAX				1.0954
#define	SAMPLE_GAIN				10000.0

#define WILLIAMSALT_CVSD			0
#define WILLIAMSALT_ADPCM			1
#define WILLIAMSALT_NARC			2


/***************************************************************************
	STRUCTURES
****************************************************************************/

struct ym2151_state
{
	double	timer_base;
	double	timer_period[3];
	UINT16	timer_value[2];
	UINT8	timer_is_active[2];
	UINT8	current_register;
	UINT8	active_timer;
};

struct counter_state
{
	UINT8 *	downcount;
	UINT8 *	divisor;
	UINT8 *	value;
	UINT16	adjusted_divisor;
	UINT16	last_hotspot_counter;
	UINT16	hotspot_hit_count;
	UINT16	hotspot_start;
	UINT16	hotspot_stop;
	UINT16	last_read_pc;
	double	time_leftover;
	void *	update_timer;
	void *	enable_timer;
	UINT8	invalid;
};

struct cvsd_state
{
	UINT8 *	state;
	UINT8 *	address;
	UINT8 *	end;
	UINT8 *	bank;
	UINT32	bits_per_firq;
	UINT32	sample_step;
	UINT32	sample_position;
	UINT16	current_sample;
	UINT8	invalid;
	double	charge;
	double	decay;
	double	leak;
	double	integrator;
	double	filter;
	UINT8	shiftreg;
};

struct dac_state
{
	UINT8 *	state_bank;
	UINT8 *	address;
	UINT8 *	end;
	UINT8 *	volume;
	UINT32	bytes_per_firq;
	UINT32	sample_step;
	UINT32	sample_position;
	UINT16	current_sample;
	UINT8	invalid;
};

/***************************************************************************
	STATIC GLOBALS
****************************************************************************/

UINT8 williamsalt_sound_int_state;

static UINT8 williamsalt_cpunum;
static UINT8 williamsalt_pianum;

static UINT8 williamsalt_audio_type;
static UINT8 adpcm_bank_count;

static struct counter_state counter;
static struct ym2151_state ym2151;
static struct cvsd_state cvsd;
static struct dac_state dac;

static int dac_stream;
static int cvsd_stream;


/***************************************************************************
	PROTOTYPES
****************************************************************************/

static void init_audio_state(int first_time);
static void locate_audio_hotspot(UINT8 *base, UINT16 start);

static int williamsalt_custom_start(const struct MachineSound *msound);


static void williamsalt_cvsd_ym2151_irq(int state);
static void williamsalt_adpcm_ym2151_irq(int state);
static void williamsalt_cvsd_irqa(int state);
static void williamsalt_cvsd_irqb(int state);

static READ8_HANDLER( williamsalt_cvsd_pia_r );
static READ8_HANDLER( williamsalt_ym2151_r );
static READ8_HANDLER( counter_down_r );
static READ8_HANDLER( counter_value_r );

static WRITE8_HANDLER( williamsalt_dac_data_w );
static WRITE8_HANDLER( williamsalt_dac2_data_w );
static WRITE8_HANDLER( williamsalt_cvsd_pia_w );
static WRITE8_HANDLER( williamsalt_ym2151_w );
static WRITE8_HANDLER( williamsalt_cvsd_bank_select_w );

static READ8_HANDLER( williamsalt_adpcm_command_r );
static WRITE8_HANDLER( williamsalt_adpcm_bank_select_w );
static WRITE8_HANDLER( williamsalt_adpcm_6295_bank_select_w );

static READ8_HANDLER( williamsalt_narc_command_r );
static READ8_HANDLER( williamsalt_narc_command2_r );
static WRITE8_HANDLER( williamsalt_narc_command2_w );
static WRITE8_HANDLER( williamsalt_narc_master_bank_select_w );
static WRITE8_HANDLER( williamsalt_narc_slave_bank_select_w );

static void counter_enable(int param);
static WRITE8_HANDLER( counter_divisor_w );
static WRITE8_HANDLER( counter_down_w );
static WRITE8_HANDLER( counter_value_w );

static WRITE8_HANDLER( cvsd_state_w );
static WRITE8_HANDLER( dac_state_bank_w );

static void update_counter(void);
static void cvsd_update(int num, INT16 *buffer, int length);
static void dac_update(int num, INT16 *buffer, int length);

static INT16 get_next_cvsd_sample(int bit);



/***************************************************************************
	PROCESSOR STRUCTURES
****************************************************************************/

/* CVSD readmem/writemem structures */
ADDRESS_MAP_START( williamsalt_cvsd_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x2000, 0x2001) AM_READ(williamsalt_ym2151_r)
	AM_RANGE(0x4000, 0x4003) AM_READ(williamsalt_cvsd_pia_r)
	AM_RANGE(0x8000, 0xffff) AM_READ(MRA8_BANK6)
ADDRESS_MAP_END


ADDRESS_MAP_START( williamsalt_cvsd_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x2000, 0x2001) AM_WRITE(williamsalt_ym2151_w)
	AM_RANGE(0x4000, 0x4003) AM_WRITE(williamsalt_cvsd_pia_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(hc55516_0_digit_clock_clear_w)
	AM_RANGE(0x6800, 0x6800) AM_WRITE(hc55516_0_clock_set_w)
	AM_RANGE(0x7800, 0x7800) AM_WRITE(williamsalt_cvsd_bank_select_w)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END



/* ADPCM readmem/writemem structures */
ADDRESS_MAP_START( williamsalt_adpcm_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x2401, 0x2401) AM_READ(williamsalt_ym2151_r)
	AM_RANGE(0x2c00, 0x2c00) AM_READ(OKIM6295_status_0_r)
	AM_RANGE(0x3000, 0x3000) AM_READ(williamsalt_adpcm_command_r)
	AM_RANGE(0x4000, 0xbfff) AM_READ(MRA8_BANK6)
	AM_RANGE(0xc000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END


ADDRESS_MAP_START( williamsalt_adpcm_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x2000, 0x2000) AM_WRITE(williamsalt_adpcm_bank_select_w)
	AM_RANGE(0x2400, 0x2401) AM_WRITE(williamsalt_ym2151_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(williamsalt_dac_data_w)
	AM_RANGE(0x2c00, 0x2c00) AM_WRITE(OKIM6295_data_0_w)
	AM_RANGE(0x3400, 0x3400) AM_WRITE(williamsalt_adpcm_6295_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_WRITE(MWA8_NOP)/*mk_sound_talkback_w }, -- talkback port? */
	AM_RANGE(0x4000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END



/* NARC master readmem/writemem structures */
ADDRESS_MAP_START( williamsalt_narc_master_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x2001, 0x2001) AM_READ(williamsalt_ym2151_r)
	AM_RANGE(0x3000, 0x3000) AM_READ(MRA8_NOP)
	AM_RANGE(0x3400, 0x3400) AM_READ(williamsalt_narc_command_r)
	AM_RANGE(0x4000, 0xbfff) AM_READ(MRA8_BANK6)
	AM_RANGE(0xc000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END


ADDRESS_MAP_START( williamsalt_narc_master_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x2000, 0x2001) AM_WRITE(williamsalt_ym2151_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(MWA8_NOP)/*mk_sound_talkback_w }, -- talkback port? */
	AM_RANGE(0x2c00, 0x2c00) AM_WRITE(williamsalt_narc_command2_w)
	AM_RANGE(0x3000, 0x3000) AM_WRITE(williamsalt_dac_data_w)
	AM_RANGE(0x3800, 0x3800) AM_WRITE(williamsalt_narc_master_bank_select_w)
	AM_RANGE(0x4000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END



/* NARC slave readmem/writemem structures */
ADDRESS_MAP_START( williamsalt_narc_slave_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_READ(MRA8_RAM)
	AM_RANGE(0x3000, 0x3000) AM_READ(MRA8_NOP)
	AM_RANGE(0x3400, 0x3400) AM_READ(williamsalt_narc_command2_r)
	AM_RANGE(0x4000, 0xbfff) AM_READ(MRA8_BANK5)
	AM_RANGE(0xc000, 0xffff) AM_READ(MRA8_ROM)
ADDRESS_MAP_END


ADDRESS_MAP_START( williamsalt_narc_slave_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0x2000, 0x2000) AM_WRITE(hc55516_0_clock_set_w)
	AM_RANGE(0x2400, 0x2400) AM_WRITE(hc55516_0_digit_clock_clear_w)
	AM_RANGE(0x3000, 0x3000) AM_WRITE(williamsalt_dac2_data_w)
	AM_RANGE(0x3800, 0x3800) AM_WRITE(williamsalt_narc_slave_bank_select_w)
	AM_RANGE(0x3c00, 0x3c00) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x4000, 0xffff) AM_WRITE(MWA8_ROM)
ADDRESS_MAP_END



/* PIA structure */
static struct pia6821_interface williamsalt_cvsd_pia_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ williamsalt_dac_data_w, 0, 0, 0,
	/*irqs   : A/B             */ williamsalt_cvsd_irqa, williamsalt_cvsd_irqb
};


/***************************************************************************
	AUDIO STRUCTURES
****************************************************************************/

/* Custom structure (all non-DCS variants) */
static struct CustomSound_interface williamsalt_custom_interface =
{
	williamsalt_custom_start,0,0
};

/* YM2151 structure (CVSD variant) */
static struct YM2151interface williamsalt_cvsd_ym2151_interface =
{
	1,			/* 1 chip */
	3579580,
	{ YM3012_VOL(10,MIXER_PAN_CENTER,10,MIXER_PAN_CENTER) },
	{ williamsalt_cvsd_ym2151_irq }
};

/* YM2151 structure (ADPCM variant) */
static struct YM2151interface williamsalt_adpcm_ym2151_interface =
{
	1,			/* 1 chip */
	3579580,
	{ YM3012_VOL(10,MIXER_PAN_CENTER,10,MIXER_PAN_CENTER) },
	{ williamsalt_adpcm_ym2151_irq }
};

/* DAC structure (CVSD variant) */
static struct DACinterface williamsalt_cvsd_dac_interface =
{
	1,
	{ 50 }
};

/* DAC structure (ADPCM variant) */
static struct DACinterface williamsalt_adpcm_dac_interface =
{
	1,
	{ 50 }
};

/* DAC structure (NARC variant) */
static struct DACinterface williamsalt_narc_dac_interface =
{
	2,
	{ 50, 50 }
};

/* CVSD structure */
static struct hc55516_interface williamsalt_cvsd_interface =
{
	1,			/* 1 chip */
	{ 80 }
};

/* OKIM6295 structure(s) */
static struct OKIM6295interface williamsalt_adpcm_6295_interface =
{
	1,          	/* 1 chip */
	{ 8000 },       /* 8000 Hz frequency */
	{ REGION_SOUND1 },  /* memory */
	{ 50 }
};


/***************************************************************************
	MACHINE DRIVERS
****************************************************************************/

MACHINE_DRIVER_START(williamsalt_cvsd_sound)
	MDRV_CPU_ADD(M6809, 8000000/4)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
	MDRV_CPU_PROGRAM_MAP(williamsalt_cvsd_readmem,williamsalt_cvsd_writemem)

	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(CUSTOM, williamsalt_custom_interface)
	MDRV_SOUND_ADD(YM2151, williamsalt_cvsd_ym2151_interface)
	MDRV_SOUND_ADD(DAC,    williamsalt_cvsd_dac_interface)
	MDRV_SOUND_ADD(HC55516,williamsalt_cvsd_interface)
MACHINE_DRIVER_END


MACHINE_DRIVER_START(williamsalt_adpcm_sound)
	MDRV_CPU_ADD(M6809, 8000000/4)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
	MDRV_CPU_PROGRAM_MAP(williamsalt_adpcm_readmem,williamsalt_adpcm_writemem)

	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(CUSTOM,  williamsalt_custom_interface)
	MDRV_SOUND_ADD(YM2151,  williamsalt_adpcm_ym2151_interface)
	MDRV_SOUND_ADD(DAC,     williamsalt_adpcm_dac_interface)
	MDRV_SOUND_ADD(OKIM6295,williamsalt_adpcm_6295_interface)
MACHINE_DRIVER_END


MACHINE_DRIVER_START(williamsalt_narc_sound)
	MDRV_CPU_ADD(M6809, 8000000/4)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
	MDRV_CPU_PROGRAM_MAP(williamsalt_narc_master_readmem,williamsalt_narc_master_writemem)

	MDRV_CPU_ADD(M6809, 8000000/4)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
	MDRV_CPU_PROGRAM_MAP(williamsalt_narc_slave_readmem,williamsalt_narc_slave_writemem)

	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(CUSTOM, williamsalt_custom_interface)
	MDRV_SOUND_ADD(YM2151, williamsalt_adpcm_ym2151_interface)
	MDRV_SOUND_ADD(DAC,    williamsalt_narc_dac_interface)
	MDRV_SOUND_ADD(HC55516,williamsalt_cvsd_interface)
MACHINE_DRIVER_END


/***************************************************************************
	INLINES
****************************************************************************/

INLINE UINT16 get_cvsd_address(void)
{
	if (cvsd.address)
		return cvsd.address[0] * 256 + cvsd.address[1];
	else
		return cpunum_get_reg(williamsalt_cpunum, M6809_Y);
}

INLINE void set_cvsd_address(UINT16 address)
{
	if (cvsd.address)
	{
		cvsd.address[0] = address >> 8;
		cvsd.address[1] = address;
	}
	else
		cpunum_set_reg(williamsalt_cpunum, M6809_Y, address);
}

INLINE UINT16 get_dac_address(void)
{
	if (dac.address)
		return dac.address[0] * 256 + dac.address[1];
	else
		return cpunum_get_reg(williamsalt_cpunum, M6809_Y);
}

INLINE void set_dac_address(UINT16 address)
{
	if (dac.address)
	{
		dac.address[0] = address >> 8;
		dac.address[1] = address;
	}
	else
		cpunum_set_reg(williamsalt_cpunum, M6809_Y, address);
}

INLINE UINT8 *get_cvsd_bank_base(int data)
{
	UINT8 *RAM = memory_region(REGION_CPU1+williamsalt_cpunum);
	int bank = data & 3;
	int quarter = (data >> 2) & 3;
	if (bank == 3) bank = 0;
	return &RAM[0x10000 + (bank * 0x20000) + (quarter * 0x8000)];
}

INLINE UINT8 *get_adpcm_bank_base(int data)
{
	UINT8 *RAM = memory_region(REGION_CPU1+williamsalt_cpunum);
	int bank = data & 7;
	return &RAM[0x10000 + (bank * 0x8000)];
}

INLINE UINT8 *get_narc_master_bank_base(int data)
{
	UINT8 *RAM = memory_region(REGION_CPU1+williamsalt_cpunum);
	int bank = data & 3;
	if (!(data & 4)) bank = 0;
	return &RAM[0x10000 + (bank * 0x8000)];
}

INLINE UINT8 *get_narc_slave_bank_base(int data)
{
	UINT8 *RAM = memory_region(REGION_CPU1+williamsalt_cpunum + 1);
	int bank = data & 7;
	return &RAM[0x10000 + (bank * 0x8000)];
}


/***************************************************************************
	INITIALIZATION
****************************************************************************/

void williamsalt_cvsd_init(int cpunum, int pianum)
{
	UINT16 entry_point;
	UINT8 *RAM;

	/* configure the CPU */
	williamsalt_cpunum = cpunum;
	williamsalt_audio_type = WILLIAMSALT_CVSD;

	/* configure the PIA */
	williamsalt_pianum = pianum;
	pia_config(pianum, PIA_STANDARD_ORDERING, &williamsalt_cvsd_pia_intf);

	/* initialize the global variables */
	init_audio_state(1);

	/* reset the chip */
	williamsalt_cvsd_reset_w(1);
	williamsalt_cvsd_reset_w(0);

	/* determine the entry point; from there, we can choose the speedup addresses */
	RAM = memory_region(REGION_CPU1+cpunum);
	entry_point = RAM[0x17ffe] * 256 + RAM[0x17fff];
	switch (entry_point)
	{
		/* Joust 2 case */
		case 0x8045:
			counter.downcount = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x217, 0x217, 0, 0, counter_down_w);
			counter.divisor = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x216, 0x216, 0, 0, counter_divisor_w);
			counter.value 	= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x214, 0x215, 0, 0, counter_value_w);
			

			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x217, 0x217, 0, 0, counter_down_r);
			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x214, 0x215, 0, 0, counter_value_r);

			cvsd.state		= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x220, 0x221, 0, 0, cvsd_state_w);
			cvsd.address	= NULL;	/* in Y */
			cvsd.end		= &RAM[0x21d];
			cvsd.bank		= &RAM[0x21f];
			cvsd.bits_per_firq = 1;

			dac.state_bank	= NULL;
			dac.address		= NULL;
			dac.end			= NULL;
			dac.volume		= NULL;
			dac.bytes_per_firq = 0;
			break;

		/* Arch Rivals case */
		case 0x8067:
			counter.downcount = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x239, 0x239, 0, 0, counter_down_w);
			counter.divisor = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x238, 0x238, 0, 0, counter_divisor_w);
			counter.value 	= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x236, 0x237, 0, 0, counter_value_w);
			

			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x239, 0x239, 0, 0, counter_down_r);
			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x236, 0x237, 0, 0, counter_value_r);

			cvsd.state		= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x23e, 0x23f, 0, 0, cvsd_state_w);
			cvsd.address	= NULL;	/* in Y */
			cvsd.end		= &RAM[0x242];
			cvsd.bank		= &RAM[0x22b];
			cvsd.bits_per_firq = 1;

			dac.state_bank	= NULL;
			dac.address		= NULL;
			dac.end			= NULL;
			dac.volume		= NULL;
			dac.bytes_per_firq = 0;
			break;

		/* General CVSD case */
		case 0x80c8:
			counter.downcount = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x23a, 0x23a, 0, 0, counter_down_w);
			counter.divisor = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x238, 0x238, 0, 0, counter_divisor_w);
			counter.value 	= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x236, 0x237, 0, 0, counter_value_w);
			

			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x23a, 0x23a, 0, 0, counter_down_r);
			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x236, 0x237, 0, 0, counter_value_r);

			cvsd.state		= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x23f, 0x240, 0, 0, cvsd_state_w);
			cvsd.address	= &RAM[0x241];
			cvsd.end		= &RAM[0x243];
			cvsd.bank		= &RAM[0x22b];
			cvsd.bits_per_firq = 4;

			dac.state_bank	= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x22c, 0x22c, 0, 0, dac_state_bank_w);
			dac.address		= NULL;	/* in Y */
			dac.end			= &RAM[0x234];
			dac.volume		= &RAM[0x231];
			dac.bytes_per_firq = 2;
			break;
	}

	/* find the hotspot for optimization */
	locate_audio_hotspot(&RAM[0x8000], 0x8000);

	/* reset the IRQ state */
	pia_set_input_ca1(williamsalt_pianum, 1);

	/* initialize the global variables */
//	init_audio_state(1);
}

void williamsalt_adpcm_init(int cpunum)
{
	UINT16 entry_point;
	UINT8 *RAM;
	int i;

	/* configure the CPU */
	williamsalt_cpunum = cpunum;
	williamsalt_audio_type = WILLIAMSALT_ADPCM;

	/* install the fixed ROM */
	RAM = memory_region(REGION_CPU1+cpunum);
	memcpy(&RAM[0xc000], &RAM[0x4c000], 0x4000);

	/* initialize the global variables */
	init_audio_state(1);

	/* reset the chip */
	williamsalt_adpcm_reset_w(1);
	williamsalt_adpcm_reset_w(0);

	/* determine the entry point; from there, we can choose the speedup addresses */
	entry_point = RAM[0xfffe] * 256 + RAM[0xffff];
	switch (entry_point)
	{
		/* General ADPCM case */
		case 0xdc51:
		case 0xdd51:
		case 0xdd55:
			counter.downcount = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x238, 0x238, 0, 0, counter_down_w);
			counter.divisor = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x236, 0x236, 0, 0, counter_divisor_w);
			counter.value 	= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x234, 0x235, 0, 0, counter_value_w);
			

			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x238, 0x238, 0, 0, counter_down_r);
			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x234, 0x235, 0, 0, counter_value_r);

			cvsd.state		= NULL;
			cvsd.address	= NULL;
			cvsd.end		= NULL;
			cvsd.bank		= NULL;
			cvsd.bits_per_firq = 0;

			dac.state_bank	= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x22a, 0x22a, 0, 0, dac_state_bank_w);
			dac.address		= NULL;	/* in Y */
			dac.end			= &RAM[0x232];
			dac.volume		= &RAM[0x22f];
			dac.bytes_per_firq = 1;
			break;

		/* Unknown case */
		default:
			break;
	}

	/* find the number of banks in the ADPCM space */
	for (i = 0; i < MAX_SOUND; i++)
		if (Machine->drv->sound[i].sound_type == SOUND_OKIM6295)
		{
			struct OKIM6295interface *intf = (struct OKIM6295interface *)Machine->drv->sound[i].sound_interface;
			adpcm_bank_count = memory_region_length(intf->region[0]) / 0x40000;
		}

	/* find the hotspot for optimization */
//	locate_audio_hotspot(&RAM[0x40000], 0xc000);

	/* initialize the global variables */
//	init_audio_state(1);
}

void williamsalt_narc_init(int cpunum)
{
	UINT16 entry_point;
	UINT8 *RAM;

	/* configure the CPU */
	williamsalt_cpunum = cpunum;
	williamsalt_audio_type = WILLIAMSALT_NARC;

	/* install the fixed ROM */
	RAM = memory_region(REGION_CPU1+cpunum + 1);
	memcpy(&RAM[0xc000], &RAM[0x4c000], 0x4000);
	RAM = memory_region(REGION_CPU1+cpunum);
	memcpy(&RAM[0xc000], &RAM[0x2c000], 0x4000);

	/* initialize the global variables */
	init_audio_state(1);

	/* reset the chip */
	williamsalt_narc_reset_w(1);
	williamsalt_narc_reset_w(0);

	/* determine the entry point; from there, we can choose the speedup addresses */
	entry_point = RAM[0xfffe] * 256 + RAM[0xffff];
	switch (entry_point)
	{
		/* General ADPCM case */
		case 0xc060:
			counter.downcount = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x249, 0x249, 0, 0, counter_down_w);
			counter.divisor = memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x248, 0x248, 0, 0, counter_divisor_w);
			counter.value 	= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x246, 0x247, 0, 0, counter_value_w);
			

			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x249, 0x249, 0, 0, counter_down_r);
			memory_install_read8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x246, 0x247, 0, 0, counter_value_r);

			cvsd.state		= NULL;
			cvsd.address	= NULL;
			cvsd.end		= NULL;
			cvsd.bank		= NULL;
			cvsd.bits_per_firq = 0;

			dac.state_bank	= memory_install_write8_handler(cpunum, ADDRESS_SPACE_PROGRAM, 0x23c, 0x23c, 0, 0, dac_state_bank_w);
			dac.address		= NULL;	/* in Y */
			dac.end			= &RAM[0x244];
			dac.volume		= &RAM[0x241];
			dac.bytes_per_firq = 1;
			break;

		/* Unknown case */
		default:
			break;
	}

	/* find the hotspot for optimization */
	locate_audio_hotspot(&RAM[0x0000], 0xc000);

	/* initialize the global variables */
//	init_audio_state(1);
}


static void init_audio_state(int first_time)
{
	/* reset the YM2151 state */
	ym2151.timer_base = 1.0 / (3579580.0 / 64.0);
	ym2151.timer_period[0] = ym2151.timer_period[1] = ym2151.timer_period[2] = 1.0;
	ym2151.timer_value[0] = ym2151.timer_value[1] = 0;
	ym2151.timer_is_active[0] = ym2151.timer_is_active[1] = 0;
	ym2151.current_register = 0x13;
	ym2151.active_timer = 2;
	YM2151_sh_reset();

	/* reset the counter state */
	counter.adjusted_divisor = 256;
	counter.last_hotspot_counter = 0xffff;
	counter.hotspot_hit_count = 0;
	counter.time_leftover = 0;
	counter.last_read_pc = 0x0000;
	if (first_time)
		counter.update_timer = timer_alloc(NULL);
	counter.invalid = 1;
	if (first_time)
		counter.enable_timer = timer_alloc(counter_enable);
	timer_adjust(counter.enable_timer, TIME_IN_SEC(3), 0, 0);

	/* reset the CVSD generator */
	cvsd.sample_step = 0;
	cvsd.sample_position = 0x10000;
	cvsd.current_sample = 0;
	cvsd.invalid = 1;
	cvsd.charge = pow(exp(-1), 1.0 / (FILTER_CHARGE_TC * 16000.0));
	cvsd.decay = pow(exp(-1), 1.0 / (FILTER_DECAY_TC * 16000.0));
	cvsd.leak = pow(exp(-1), 1.0 / (INTEGRATOR_LEAK_TC * 16000.0));
	cvsd.integrator = 0;
	cvsd.filter = FILTER_MIN;
	cvsd.shiftreg = 0xaa;

	/* reset the DAC generator */
	dac.sample_step = 0;
	dac.sample_position = 0x10000;
	dac.current_sample = 0;
	dac.invalid = 1;

	/* clear all the interrupts */
	williamsalt_sound_int_state = 0;
	cpunum_set_input_line(williamsalt_cpunum, M6809_FIRQ_LINE, CLEAR_LINE);
	cpunum_set_input_line(williamsalt_cpunum, M6809_IRQ_LINE, CLEAR_LINE);
	cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_NMI, CLEAR_LINE);
	if (williamsalt_audio_type == WILLIAMSALT_NARC)
	{
	cpunum_set_input_line(williamsalt_cpunum + 1, M6809_FIRQ_LINE, CLEAR_LINE);
	cpunum_set_input_line(williamsalt_cpunum + 1, M6809_IRQ_LINE, CLEAR_LINE);
	cpunum_set_input_line(williamsalt_cpunum + 1, INPUT_LINE_NMI, CLEAR_LINE);
	}
}

static void locate_audio_hotspot(UINT8 *base, UINT16 start)
{
	int i;

	/* search for the loop that kills performance so we can optimize it */
	for (i = start; i < 0x10000; i++)
	{
		if (base[i + 0] == 0x1a && base[i + 1] == 0x50 &&			/* 1A 50       ORCC  #$0050  */
			base[i + 2] == 0x93 &&									/* 93 xx       SUBD  $xx     */
			base[i + 4] == 0xe3 && base[i + 5] == 0x4c &&			/* E3 4C       ADDD  $000C,U */
			base[i + 6] == 0x9e && base[i + 7] == base[i + 3] &&	/* 9E xx       LDX   $xx     */
			base[i + 8] == 0xaf && base[i + 9] == 0x4c &&			/* AF 4C       STX   $000C,U */
			base[i +10] == 0x1c && base[i +11] == 0xaf)				/* 1C AF       ANDCC #$00AF  */
		{
			counter.hotspot_start = i;
			counter.hotspot_stop = i + 12;
			logerror("Found hotspot @ %04X", i);
			return;
		}
	}
	logerror("Found no hotspot!");
}


/***************************************************************************
	CUSTOM SOUND INTERFACES
****************************************************************************/

static int williamsalt_custom_start(const struct MachineSound *msound)
{
	(void)msound;

	/* allocate a DAC stream */
	dac_stream = stream_init("Accelerated DAC", 50, Machine->sample_rate, 0, dac_update);

	/* allocate a CVSD stream */
	cvsd_stream = stream_init("Accelerated CVSD", 40, Machine->sample_rate, 0, cvsd_update);

	return 0;
}

/***************************************************************************
	CVSD IRQ GENERATION CALLBACKS
****************************************************************************/

static void williamsalt_cvsd_ym2151_irq(int state)
{
	pia_set_input_ca1(williamsalt_pianum, !state);
}

static void williamsalt_cvsd_irqa(int state)
{
	cpunum_set_input_line(williamsalt_cpunum, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

static void williamsalt_cvsd_irqb(int state)
{
	cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}


/***************************************************************************
	ADPCM IRQ GENERATION CALLBACKS
****************************************************************************/

static void williamsalt_adpcm_ym2151_irq(int state)
{
	cpunum_set_input_line(williamsalt_cpunum, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


/***************************************************************************
	CVSD BANK SELECT
****************************************************************************/

WRITE8_HANDLER( williamsalt_cvsd_bank_select_w )
{
	cpu_setbank(6, get_cvsd_bank_base(data));
}


/***************************************************************************
	ADPCM BANK SELECT
****************************************************************************/

WRITE8_HANDLER( williamsalt_adpcm_bank_select_w )
{
	cpu_setbank(6, get_adpcm_bank_base(data));
}

WRITE8_HANDLER( williamsalt_adpcm_6295_bank_select_w )
{
	if (adpcm_bank_count <= 3)
	{
		if (!(data & 0x04))
			OKIM6295_set_bank_base(0, 0x00000);
		else if (data & 0x01)
			OKIM6295_set_bank_base(0, 0x40000);
		else
			OKIM6295_set_bank_base(0, 0x80000);
	}
	else
	{
		data &= 7;
		if (data != 0)
			OKIM6295_set_bank_base(0, (data - 1) * 0x40000);
	}
}


/***************************************************************************
	NARC BANK SELECT
****************************************************************************/

WRITE8_HANDLER( williamsalt_narc_master_bank_select_w )
{
	cpu_setbank(6, get_narc_master_bank_base(data));
}

WRITE8_HANDLER( williamsalt_narc_slave_bank_select_w )
{
	cpu_setbank(5, get_narc_slave_bank_base(data));
}



/***************************************************************************
	CVSD COMMUNICATIONS
****************************************************************************/

static void williamsalt_cvsd_delayed_data_w(int param)
{
	pia_set_input_b(williamsalt_pianum, param & 0xff);
	pia_set_input_cb1(williamsalt_pianum, param & 0x100);
	pia_set_input_cb2(williamsalt_pianum, param & 0x200);
}

void williamsalt_cvsd_data_w(int data)
{
	timer_set(TIME_NOW, data, williamsalt_cvsd_delayed_data_w);
}

void williamsalt_cvsd_reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		williamsalt_cvsd_bank_select_w(0, 0);
		init_audio_state(0);
		cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
		cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_RESET, CLEAR_LINE);
}


/***************************************************************************
	ADPCM COMMUNICATIONS
****************************************************************************/

READ8_HANDLER( williamsalt_adpcm_command_r )
{
	cpunum_set_input_line(williamsalt_cpunum, M6809_IRQ_LINE, CLEAR_LINE);
	williamsalt_sound_int_state = 0;
	return soundlatch_r(0);
}

void williamsalt_adpcm_data_w(int data)
{
	soundlatch_w(0, data & 0xff);
	if (!(data & 0x200))
	{
		cpunum_set_input_line(williamsalt_cpunum, M6809_IRQ_LINE, ASSERT_LINE);
		williamsalt_sound_int_state = 1;
	}
}

void williamsalt_adpcm_reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		williamsalt_adpcm_bank_select_w(0, 0);
		init_audio_state(0);
		cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
		cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_RESET, CLEAR_LINE);
}


/***************************************************************************
	NARC COMMUNICATIONS
****************************************************************************/

READ8_HANDLER( williamsalt_narc_command_r )
{
	cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_NMI, CLEAR_LINE);
	cpunum_set_input_line(williamsalt_cpunum, M6809_IRQ_LINE, CLEAR_LINE);
	williamsalt_sound_int_state = 0;
	return soundlatch_r(0);
}

void williamsalt_narc_data_w(int data)
{
	soundlatch_w(0, data & 0xff);
	if (!(data & 0x100))
		cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_NMI, ASSERT_LINE);
	if (!(data & 0x200))
	{
		cpunum_set_input_line(williamsalt_cpunum, M6809_IRQ_LINE, ASSERT_LINE);
		williamsalt_sound_int_state = 1;
	}
}

void williamsalt_narc_reset_w(int state)
{
	/* going high halts the CPU */
	if (state)
	{
		williamsalt_narc_master_bank_select_w(0, 0);
		williamsalt_narc_slave_bank_select_w(0, 0);
		init_audio_state(0);
		cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_RESET, ASSERT_LINE);
		cpunum_set_input_line(williamsalt_cpunum + 1, INPUT_LINE_RESET, ASSERT_LINE);
	}
	/* going low resets and reactivates the CPU */
	else
	{
		cpunum_set_input_line(williamsalt_cpunum, INPUT_LINE_RESET, CLEAR_LINE);
		cpunum_set_input_line(williamsalt_cpunum + 1, INPUT_LINE_RESET, CLEAR_LINE);
	}
}

READ8_HANDLER( williamsalt_narc_command2_r )
{
	cpunum_set_input_line(williamsalt_cpunum + 1, M6809_FIRQ_LINE, CLEAR_LINE);
	return soundlatch2_r(0);
}

WRITE8_HANDLER( williamsalt_narc_command2_w )
{
	soundlatch2_w(0, data & 0xff);
	cpunum_set_input_line(williamsalt_cpunum + 1, M6809_FIRQ_LINE, ASSERT_LINE);
}



/***************************************************************************
	YM2151 INTERFACES
****************************************************************************/

static READ8_HANDLER( williamsalt_ym2151_r )
{
	return YM2151_status_port_0_r(offset);
}

static WRITE8_HANDLER( williamsalt_ym2151_w )
{
	if (offset & 1)
	{
		/* handle timer registers here */
		switch (ym2151.current_register)
		{
			case 0x10:	/* timer A, upper 8 bits */
				update_counter();
				ym2151.timer_value[0] = (ym2151.timer_value[0] & 0x003) | (data << 2);
				ym2151.timer_period[0] = ym2151.timer_base * (double)(1024 - ym2151.timer_value[0]);
				break;

			case 0x11:	/* timer A, upper 8 bits */
				update_counter();
				ym2151.timer_value[0] = (ym2151.timer_value[0] & 0x3fc) | (data & 0x03);
				ym2151.timer_period[0] = ym2151.timer_base * (double)(1024 - ym2151.timer_value[0]);
				break;

			case 0x12:	/* timer B */
				update_counter();
				ym2151.timer_value[1] = data;
				ym2151.timer_period[1] = ym2151.timer_base * (double)((256 - ym2151.timer_value[1]) << 4);
				break;

			case 0x14:	/* timer control */

				/* enable/disable timer A */
				if ((data & 0x01) && (data & 0x04) && !ym2151.timer_is_active[0])
				{
					update_counter();
					ym2151.timer_is_active[0] = 1;
					ym2151.active_timer = 0;
				}
				else if (!(data & 0x01) && ym2151.timer_is_active[0])
				{
					ym2151.timer_is_active[0] = 0;
					ym2151.active_timer = ym2151.timer_is_active[1] ? 1 : 2;
				}

				/* enable/disable timer B */
				if ((data & 0x02) && (data & 0x08) && !ym2151.timer_is_active[1])
				{
					update_counter();
					ym2151.timer_is_active[1] = 1;
					ym2151.active_timer = 1;
				}
				else if (!(data & 0x02) && ym2151.timer_is_active[1])
				{
					ym2151.timer_is_active[1] = 0;
					ym2151.active_timer = ym2151.timer_is_active[0] ? 0 : 2;
				}
				break;

			default:	/* pass through everything else */
				YM2151_data_port_0_w(offset, data);
				break;
		}
	}
	else
	{
		if (!DISABLE_FIRQ_SPEEDUP)
			ym2151.current_register = data;

		/* only pass through register writes for non-timer registers */
		if (DISABLE_FIRQ_SPEEDUP || ym2151.current_register < 0x10 || ym2151.current_register > 0x14)
			YM2151_register_port_0_w(offset, data);
	}
}


/***************************************************************************
	PIA INTERFACES
****************************************************************************/

static READ8_HANDLER( williamsalt_cvsd_pia_r )
{
	return pia_read(williamsalt_pianum, offset);
}

static WRITE8_HANDLER( williamsalt_cvsd_pia_w )
{
	pia_write(williamsalt_pianum, offset, data);
}


/***************************************************************************
	DAC INTERFACES
****************************************************************************/

static WRITE8_HANDLER( williamsalt_dac_data_w )
{
	DAC_data_w(0, data);
}

static WRITE8_HANDLER( williamsalt_dac2_data_w )
{
	DAC_data_w(1, data);
}


/***************************************************************************
	SPEEDUP KLUDGES
****************************************************************************/

static void counter_enable(int param)
{
	counter.invalid = 0;
	timer_adjust(counter.update_timer, TIME_NEVER, 0, 0);

	/* the counter routines all reset the bank, but NARC is the only one that cares */
	if (williamsalt_audio_type == WILLIAMSALT_NARC)
		williamsalt_narc_master_bank_select_w(0, 5);
}

static void update_counter(void)
{
	double time_since_update, timer_period = ym2151.timer_period[ym2151.active_timer];
	int firqs_since_update, complete_ticks, downcounter;

	if (DISABLE_FIRQ_SPEEDUP || ym2151.active_timer == 2 || counter.invalid)
		return;

	/* compute the time since we last updated; if it's less than one period, do nothing */
	time_since_update = timer_timeelapsed(counter.update_timer) + counter.time_leftover;
	if (time_since_update < timer_period)
		return;

	/* compute the integral number of FIRQ interrupts that have occured since the last update */
	firqs_since_update = (int)(time_since_update / timer_period);

	/* keep track of any additional time */
	counter.time_leftover = time_since_update - (double)firqs_since_update * timer_period;

	/* reset the timer */
	timer_adjust(counter.update_timer, TIME_NEVER, 0, 0);

	/* determine the number of complete ticks that have occurred */
	complete_ticks = firqs_since_update / counter.adjusted_divisor;

	/* subtract any remainder from the down counter, and carry if appropriate */
	downcounter = counter.downcount[0] - (firqs_since_update - complete_ticks * counter.adjusted_divisor);
	if (downcounter < 0)
	{
		downcounter += counter.adjusted_divisor;
		complete_ticks++;
	}

	/* add in the previous value of the counter */
	complete_ticks += counter.value[0] * 256 + counter.value[1];

	/* store the results */
	counter.value[0] = complete_ticks >> 8;
	counter.value[1] = complete_ticks;
	counter.downcount[0] = downcounter;
}

static WRITE8_HANDLER( counter_divisor_w )
{
	update_counter();
	counter.divisor[offset] = data;
	counter.adjusted_divisor = data ? data : 256;
}

static READ8_HANDLER( counter_down_r )
{
	update_counter();
	return counter.downcount[offset];
}

static WRITE8_HANDLER( counter_down_w )
{
	update_counter();
	counter.downcount[offset] = data;
}

static READ8_HANDLER( counter_value_r )
{
	UINT16 pc = activecpu_get_previouspc();

	/* only update the counter on the MSB */
	/* also, don't update it if we just read from it a few instructions back */
	if (offset == 0 && (pc <= counter.last_read_pc || pc > counter.last_read_pc + 16))
		update_counter();
	counter.last_read_pc = pc;

	/* on the second LSB read in the hotspot, check to see if we will be looping */
	if (offset == 1 && pc == counter.hotspot_start + 6 && !DISABLE_LOOP_CATCHERS)
	{
		UINT16 new_counter = counter.value[0] * 256 + counter.value[1];

		/* count how many hits in a row we got the same value */
		if (new_counter == counter.last_hotspot_counter)
			counter.hotspot_hit_count++;
		else
			counter.hotspot_hit_count = 0;
		counter.last_hotspot_counter = new_counter;

		/* more than 3 hits in a row and we optimize */
		if (counter.hotspot_hit_count > 3)
			activecpu_adjust_icount(-50);
	}

	return counter.value[offset];
}

static WRITE8_HANDLER( counter_value_w )
{
	/* only update the counter after the LSB is written */
	if (offset == 1)
		update_counter();
	counter.value[offset] = data;
	counter.hotspot_hit_count = 0;
}


/***************************************************************************
	CVSD KLUDGES
****************************************************************************/

static void cvsd_start(int param)
{
	double sample_rate;
	int start, end;

	/* if interrupts are disabled, try again later */
	if (cpunum_get_reg(williamsalt_cpunum, M6809_CC) & 0x40)
	{
		timer_set(TIME_IN_MSEC(1), 0, cvsd_start);
		return;
	}

	/* determine the start and end addresses */
	start = get_cvsd_address();
	end = cvsd.end[0] * 256 + cvsd.end[1];

	/* compute the effective sample rate */
	sample_rate = (double)cvsd.bits_per_firq / ym2151.timer_period[ym2151.active_timer];
	cvsd.sample_step = (int)(sample_rate * 65536.0 / (double)Machine->sample_rate);
	cvsd.invalid = 0;
}

static WRITE8_HANDLER( cvsd_state_w )
{
	/* if we write a value here with a non-zero high bit, prepare to start playing */
	stream_update(cvsd_stream, 0);
	if (offset == 0 && !(data & 0x80) && ym2151.active_timer != 2)
	{
		cvsd.invalid = 1;
		timer_set(TIME_IN_MSEC(1), 0, cvsd_start);
	}

	cvsd.state[offset] = data;
}


/***************************************************************************
	DAC KLUDGES
****************************************************************************/

static void dac_start(int param)
{
	double sample_rate;
	int start, end;

	/* if interrupts are disabled, try again later */
	if (cpunum_get_reg(williamsalt_cpunum, M6809_CC) & 0x40)
	{
		timer_set(TIME_IN_MSEC(1), 0, dac_start);
		return;
	}

	/* determine the start and end addresses */
	start = get_dac_address();
	end = dac.end[0] * 256 + dac.end[1];

	/* compute the effective sample rate */
	sample_rate = (double)dac.bytes_per_firq / ym2151.timer_period[ym2151.active_timer];
	dac.sample_step = (int)(sample_rate * 65536.0 / (double)Machine->sample_rate);
	dac.invalid = 0;
}

static WRITE8_HANDLER( dac_state_bank_w )
{
	/* if we write a value here with a non-zero high bit, prepare to start playing */
	stream_update(dac_stream, 0);
	if (!(data & 0x80) && ym2151.active_timer != 2)
	{
		dac.invalid = 1;
		timer_set(TIME_IN_MSEC(1), 0, dac_start);
	}

	dac.state_bank[offset] = data;
}


/***************************************************************************
	SOUND GENERATION
****************************************************************************/

static void cvsd_update(int num, INT16 *buffer, int length)
{
	UINT8 *bank_base, *source, *end;
	UINT32 current;
	int i;

	/* CVSD generation */
	if (cvsd.state && !(cvsd.state[0] & 0x80))
	{
		UINT8 bits_left = cvsd.state[0];
		UINT8 current_byte = cvsd.state[1];

		/* determine start and end points */
		bank_base = get_cvsd_bank_base(cvsd.bank[0]) - 0x8000;
		source = bank_base + get_cvsd_address();
		end = bank_base + cvsd.end[0] * 256 + cvsd.end[1];

		current = cvsd.sample_position;

		/* fill in with samples until we hit the end or run out */
		for (i = 0; i < length; i++)
		{
			/* if we overflow to the next sample, process it */
			while (current > 0xffff)
			{
				if (bits_left-- == 0)
				{
					if (source >= end)
					{
						bits_left |= 0x80;
						cvsd.sample_position = 0x10000;
						cvsd.integrator = 0;
						cvsd.filter = FILTER_MIN;
						cvsd.shiftreg = 0xaa;
						memset(buffer, 0, (length - i) * sizeof(INT16));
						goto finished;
					}
					current_byte = *source++;
					bits_left = 7;
				}
				cvsd.current_sample = get_next_cvsd_sample(current_byte & (0x80 >> bits_left));
				current -= 0x10000;
			}

			*buffer++ = cvsd.current_sample;
			current += cvsd.sample_step;
		}

		/* update the final values */
	finished:
		set_cvsd_address(source - bank_base);
		cvsd.sample_position = current;
		cvsd.state[0] = bits_left;
		cvsd.state[1] = current_byte;
	}
	else
		memset(buffer, 0, length * sizeof(INT16));
}

static void dac_update(int num, INT16 *buffer, int length)
{
	UINT8 *bank_base, *source, *end;
	UINT32 current;
	int i;

	/* DAC generation */
	if (dac.state_bank && !(dac.state_bank[0] & 0x80))
	{
		UINT8 volume = dac.volume[0] / 2;

		/* determine start and end points */
		if (williamsalt_audio_type == WILLIAMSALT_CVSD)
			bank_base = get_cvsd_bank_base(dac.state_bank[0]) - 0x8000;
		else if (williamsalt_audio_type == WILLIAMSALT_ADPCM)
			bank_base = get_adpcm_bank_base(dac.state_bank[0]) - 0x4000;
		else
			bank_base = get_narc_master_bank_base(dac.state_bank[0]) - 0x4000;
		source = bank_base + get_dac_address();
		end = bank_base + dac.end[0] * 256 + dac.end[1];

		current = dac.sample_position;

		/* fill in with samples until we hit the end or run out */
		for (i = 0; i < length; i++)
		{
			/* if we overflow to the next sample, process it */
			while (current > 0xffff)
			{
				if (source >= end)
				{
					dac.state_bank[0] |= 0x80;
					dac.sample_position = 0x10000;
					memset(buffer, 0, (length - i) * sizeof(INT16));
					goto finished;
				}
				dac.current_sample = *source++ * volume;
				current -= 0x10000;
			}

			*buffer++ = dac.current_sample;
			current += dac.sample_step;
		}

		/* update the final values */
	finished:
		set_dac_address(source - bank_base);
		dac.sample_position = current;
	}
	else
		memset(buffer, 0, length * sizeof(INT16));
}

/***************************************************************************
	CVSD DECODING (cribbed from HC55516.c)
****************************************************************************/

INT16 get_next_cvsd_sample(int bit)
{
	double temp;

	/* move the estimator up or down a step based on the bit */
	if (bit)
	{
		cvsd.shiftreg = ((cvsd.shiftreg << 1) | 1) & 7;
		cvsd.integrator += cvsd.filter;
	}
	else
	{
		cvsd.shiftreg = (cvsd.shiftreg << 1) & 7;
		cvsd.integrator -= cvsd.filter;
	}

	/* simulate leakage */
	cvsd.integrator *= cvsd.leak;

	/* if we got all 0's or all 1's in the last n bits, bump the step up */
	if (cvsd.shiftreg == 0 || cvsd.shiftreg == 7)
	{
		cvsd.filter = FILTER_MAX - ((FILTER_MAX - cvsd.filter) * cvsd.charge);
		if (cvsd.filter > FILTER_MAX)
			cvsd.filter = FILTER_MAX;
	}

	/* simulate decay */
	else
	{
		cvsd.filter *= cvsd.decay;
		if (cvsd.filter < FILTER_MIN)
			cvsd.filter = FILTER_MIN;
	}

	/* compute the sample as a 32-bit word */
	temp = cvsd.integrator * SAMPLE_GAIN;

	/* compress the sample range to fit better in a 16-bit word */
	if (temp < 0)
		return (int)(temp / (-temp * (1.0 / 32768.0) + 1.0));
	else
		return (int)(temp / (temp * (1.0 / 32768.0) + 1.0));
}