#pragma code_seg("C901")
#pragma data_seg("D901")
#pragma bss_seg("B901")
#pragma const_seg("K901")
#pragma comment(linker, "/merge:D901=901")
#pragma comment(linker, "/merge:C901=901")
#pragma comment(linker, "/merge:B901=901")
#pragma comment(linker, "/merge:K901=901")
/***************************************************************************

	Sega pre-System 16 & System 16A hardware

*****************************************************************************/

#include "driver.h"
#include "system16.h"
#include "machine/8255ppi.h"
#include "cpu/i8039/i8039.h"



/*************************************
 *
 *	Statics
 *
 *************************************/

static data16_t *workram;

static UINT8 has_n7759;

static read16_handler custom_io_r;
static write16_handler custom_io_w;

static void (*i8751_vblank_hook)(void);



/*************************************
 *
 *	Prototypes
 *
 *************************************/

extern void fd1094_machine_init(void);
extern void fd1094_driver_init(void);

static READ16_HANDLER( misc_io_r );
static WRITE16_HANDLER( misc_io_w );

static WRITE8_HANDLER( video_control_w );
static WRITE8_HANDLER( sound_command_w );
static WRITE8_HANDLER( sound_control_w );



/*************************************
 *
 *	PPI interfaces
 *
 *************************************/

static ppi8255_interface single_ppi_intf =
{
	1,
	{ NULL },
	{ NULL },
	{ NULL },
	{ sound_command_w },
	{ video_control_w },
	{ sound_control_w }
};



/*************************************
 *
 *	Configuration
 *
 *************************************/

static void system16a_generic_init(void)
{
	/* call the generic init */
	machine_init_sys16_onetime_afighter();

	/* init the FD1094 */
	fd1094_driver_init();

	/* reset the custom handlers and other pointers */
	custom_io_r = NULL;
	custom_io_w = NULL;
	i8751_vblank_hook = NULL;

	/* see if we have an N7759 chip */
	has_n7759 = (mame_find_cpu_index("n7751") != -1);

	/* configure the 8255 interface */
	ppi8255_init(&single_ppi_intf);
}



/*************************************
 *
 *	Initialization & interrupts
 *
 *************************************/

MACHINE_INIT( system16a )
{
	fd1094_machine_init();

	/* if we have a fake i8751 handler, disable the actual 8751 */
	if (i8751_vblank_hook != NULL)
		cpunum_suspend(mame_find_cpu_index("mcu"), SUSPEND_REASON_DISABLE, 1);
}



/*************************************
 *
 *	I/O space
 *
 *************************************/

static READ16_HANDLER( misc_io_r )
{
	offset &= 0x1fff;
	switch (offset & (0x3000/2))
	{
		case 0x0000/2:
			return ppi8255_0_r(offset & 3);

		case 0x1000/2:
			return readinputport(offset & 3);

		case 0x2000/2:
			return readinputport(4 + (offset & 1));
	}
	if (custom_io_r)
		return custom_io_r(offset, mem_mask);
	logerror("%06X:misc_io_r - unknown read access to address %04X\n", activecpu_get_pc(), offset * 2);
	return 0xffff;
}


static WRITE16_HANDLER( misc_io_w )
{
	offset &= 0x1fff;
	switch (offset & (0x3000/2))
	{
		case 0x0000/2:
			if (ACCESSING_LSB)
				ppi8255_0_w(offset & 3, data & 0xff);
			return;
	}
	if (custom_io_w)
	{
		custom_io_w(offset, data, mem_mask);
		return;
	}
	logerror("%06X:misc_io_w - unknown write access to address %04X = %04X & %04X\n", activecpu_get_pc(), offset * 2, data, mem_mask ^ 0xffff);
}



/*************************************
 *
 *	Video control
 *
 *************************************/

static WRITE8_HANDLER( video_control_w )
{
	/*
	 PPI port B

	 D7 : Screen flip (1= flip, 0= normal orientation)
	 D6 : To 8751 pin 13 (/INT1)
	 D5 : To 315-5149 pin 17.
	 D4 : Screen enable (1= display, 0= blank)
	 D3 : Lamp #2 (1= on, 0= off)
	 D2 : Lamp #1 (1= on, 0= off)
	 D1 : Coin meter #2
	 D0 : Coin meter #1
	*/
	system16a_set_screen_flip(data & 0x80);
	system16a_set_draw_enable(data & 0x10);
	set_led_status(1, data & 0x08);
	set_led_status(0, data & 0x04);
	coin_counter_w(1, data & 0x02);
	coin_counter_w(0, data & 0x01);
}



/*************************************
 *
 *	Sound control
 *
 *************************************/

static WRITE8_HANDLER( sound_command_w )
{
	soundlatch_w(0, data);
	cpunum_set_input_line(1, INPUT_LINE_NMI, PULSE_LINE);
}


static WRITE8_HANDLER( sound_control_w )
{
	/*
	 PPI port C

	 D7 : Port A handshaking signal /OBF
	 D6 : Port A handshaking signal ACK
	 D5 : Port A handshaking signal IBF
	 D4 : Port A handshaking signal /STB
	 D3 : Port A handshaking signal INTR
	 D2 : To PAL 315-5107 pin 9 (SCONT1)
	 D1 : To PAL 315-5108 pin 19 (SCONT0)
	 D0 : To MUTE input on MB3733 amplifier.
	      0= Sound is disabled
	      1= sound is enabled
	*/
	system16a_set_colscroll(~data & 0x04);
	system16a_set_rowscroll(~data & 0x02);
}



/*************************************
 *
 *	Sound interaction
 *
 *************************************/

static UINT8 port_8255_c03 = 0;
static UINT8 port_8255_c47 = 0;
static UINT8 port_7751_p27 = 0;
static UINT32 rom_offset = 0;
static UINT32 rom_base = 0;
static UINT32 rom_bank = 0;

static void trigger_7751_sound(int data)
{
	/* I think this is correct for 128k sound roms,
	     it's OK for smaller roms */
	if((data&0xf) == 0xc) rom_bank=0;
	else if((data&0xf) == 0xd) rom_bank=0x4000;
	else if((data&0xf) == 0xb) rom_bank=0xc000;
	else if((data&0xf) == 0xa) rom_bank=0x8000;

	else if((data&0xf) == 0xf) rom_bank=0x1c000;
	else if((data&0xf) == 0xe) rom_bank=0x18000;
	else if((data&0xf) == 0x7) rom_bank=0x14000;
	else if((data&0xf) == 0x6) rom_bank=0x10000;

	port_8255_c03 = (data>>5);

	cpunum_set_input_line(2, 0, PULSE_LINE);
}

// I'm sure this must be wrong, but it seems to work for quartet music.
WRITE8_HANDLER( n7751_audio_8255_w )
{
	logerror("7751: %4x %4x\n",data,data^0xff);

	if (has_n7759 && (data & 0x0f) != 8)
	{
		cpunum_set_input_line(2, INPUT_LINE_RESET, PULSE_LINE);
		timer_set(TIME_IN_USEC(300), data, trigger_7751_sound);
	}
}


READ8_HANDLER( n7751_audio_8255_r )
{
	// Only PC4 is hooked up
	/* 0x00 = BUSY, 0x10 = NOT BUSY */
	return (port_8255_c47 & 0x10);
}

/* read from BUS */
READ8_HANDLER( n7751_rom_r )
{
	UINT8 *sound_rom = memory_region(REGION_SOUND1);
	int size = memory_region_length(REGION_SOUND1);
	return sound_rom[(rom_offset + rom_base) % size];
}

/* read from T1 - Labelled as "TEST", connected to ground */
READ8_HANDLER( n7751_t1_r )
{
	return 0;
}

/* read from P2 - 8255's PC0-2 connects to 7751's S0-2 (P24-P26 on an 8048) */
READ8_HANDLER( n7751_command_r )
{
	return ((port_8255_c03 & 0x07) << 4) | port_7751_p27;
}

/* write to P2 */
WRITE8_HANDLER( n7751_busy_w )
{
	port_8255_c03 = (data & 0x70) >> 4;
	port_8255_c47 = (data & 0x80) >> 3;
	port_7751_p27 = data & 0x80;
	rom_base = rom_bank;
}

/* write to P4 */
WRITE8_HANDLER( n7751_offset_a0_a3_w )
{
	rom_offset = (rom_offset & 0xfff0) | (data & 0x0f);
}

/* write to P5 */
WRITE8_HANDLER( n7751_offset_a4_a7_w )
{
	rom_offset = (rom_offset & 0xff0f) | ((data & 0x0f) << 4);
}

/* write to P6 */
WRITE8_HANDLER( n7751_offset_a8_a11_w )
{
	rom_offset = (rom_offset & 0xf0ff) | ((data & 0x0f) << 8);
}

/* write to P7 */
WRITE8_HANDLER( n7751_rom_select_w )
{
	rom_offset = (rom_offset & 0x0fff) | ((data & 0x03) << 12);
}



/*************************************
 *
 *	I8751 interrupt generation
 *
 *************************************/

static INTERRUPT_GEN( i8751_main_cpu_vblank )
{
	/* if we have a fake 8751 handler, call it on VBLANK */
	if (i8751_vblank_hook != NULL)
		(*i8751_vblank_hook)();
}

/*************************************
 *
 *	Capacitor-backed RAM
 *
 *************************************/

static NVRAM_HANDLER( system16a )
{
	if (read_or_write)
		mame_fwrite(file, workram, 0x4000);
	else if (file)
		mame_fread(file, workram, 0x4000);
}



/*************************************
 *
 *	Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( system16a_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x400000, 0x407fff) AM_READWRITE(MRA16_RAM, segaic16_tileram_w) AM_BASE(&segaic16_tileram)
	AM_RANGE(0x410000, 0x410fff) AM_READWRITE(MRA16_RAM, system16a_textram_w) AM_BASE(&segaic16_textram)
	AM_RANGE(0x440000, 0x4407ff) AM_RAM AM_BASE(&segaic16_spriteram)
	AM_RANGE(0x840000, 0x840fff) AM_READWRITE(paletteram16_word_r, segaic16_paletteram_w) AM_BASE(&paletteram16)
	AM_RANGE(0xc40000, 0xc43fff) AM_READWRITE(misc_io_r, misc_io_w)
	AM_RANGE(0xffc000, 0xffffff) AM_RAM AM_BASE(&workram)

//	AM_RANGE(0x000000, 0x05ffff) AM_MIRROR(0x380000) AM_ROM
//	AM_RANGE(0x400000, 0x407fff) AM_MIRROR(0xb88000) AM_READWRITE(MRA16_RAM, segaic16_tileram_w) AM_BASE(&segaic16_tileram)
//	AM_RANGE(0x410000, 0x410fff) AM_MIRROR(0xb8f000) AM_READWRITE(MRA16_RAM, system16a_textram_w) AM_BASE(&segaic16_textram)
//	AM_RANGE(0x440000, 0x4407ff) AM_MIRROR(0x3bf800) AM_RAM AM_BASE(&segaic16_spriteram)
//	AM_RANGE(0x840000, 0x840fff) AM_MIRROR(0x3bf000) AM_READWRITE(paletteram16_word_r, segaic16_paletteram_w) AM_BASE(&paletteram16)
//	AM_RANGE(0xc40000, 0xc43fff) AM_MIRROR(0x39c000) AM_READWRITE(misc_io_r, misc_io_w)
//	AM_RANGE(0xc70000, 0xc73fff) AM_MIRROR(0x38c000) AM_RAM AM_BASE(&workram)
ADDRESS_MAP_END



/*************************************
 *
 *	Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe800, 0xe800) AM_READ(soundlatch_r)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x3e) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x3e) AM_READWRITE(YM2151_status_port_0_r, YM2151_data_port_0_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3f) AM_WRITE(n7751_audio_8255_w)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x3f) AM_READ(soundlatch_r)
ADDRESS_MAP_END



/*************************************
 *
 *	Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( afighter )
	PORT_START_TAG("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )

	PORT_START_TAG("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("COINAGE")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too) or 1/1" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "Infinite" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "10000 - 20000" )
	PORT_DIPSETTING(    0x20, "20000 - 40000" )
	PORT_DIPSETTING(    0x10, "30000 - 60000" )
	PORT_DIPSETTING(    0x00, "40000 - 80000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x80, 0x80, "Allow_Continue" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END



/*************************************
 *
 *	Sound definitions
 *
 *************************************/


static struct YM2151interface ym2151_interface =
{
	1,
	4000000,
	{ YM3012_VOL(43,MIXER_PAN_LEFT,43,MIXER_PAN_RIGHT) },
	{ NULL },
//	{ n7751_control_w }
};

static struct YM2151interface ym2151_interface_topvolume =
{
	1,
	4000000,
	{ YM3012_VOL(100,MIXER_PAN_LEFT,100,MIXER_PAN_RIGHT) },
	{ 0 }
};


struct DACinterface n7751_dac_interface =
{
	1,
	{ 80 }
};



/*************************************
 *
 *	Graphics definitions
 *
 *************************************/

static struct GfxLayout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &charlayout,	0, 1024 },
	{ -1 }
};



/*************************************
 *
 *	Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( system16a )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(system16a_map,0)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1)

	MDRV_CPU_ADD_TAG("sound", Z80, 4000000)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_IO_MAP(sound_portmap,0)

	MDRV_CPU_ADD_TAG("n7751", N7751, 6000000/15)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
//	MDRV_CPU_PROGRAM_MAP(n7751_map,0)
//	MDRV_CPU_IO_MAP(n7751_portmap,0)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(1000000 * (262 - 224) / (262 * 60))

	MDRV_MACHINE_INIT(system16a)
	MDRV_NVRAM_HANDLER(system16a)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(40*8, 28*8)
	MDRV_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(2048*3)

	MDRV_VIDEO_START(system16a)
	MDRV_VIDEO_UPDATE(system16a)

	/* sound hardware */
	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD_TAG("2151", YM2151, ym2151_interface)
	MDRV_SOUND_ADD_TAG("dac", DAC, n7751_dac_interface)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( system16a_no7751 )
	MDRV_IMPORT_FROM(system16a)
	MDRV_CPU_REMOVE("n7751")
	MDRV_SOUND_REMOVE("dac")
	MDRV_SOUND_REPLACE("2151", YM2151, ym2151_interface_topvolume)
MACHINE_DRIVER_END






/*************************************
 *
 *	ROM definition(s)
 *
 *************************************/

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Action Fighter, Sega System 16A
	CPU: Unprotected
 */
ROM_START( afighter )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr10315.bin", 0x00001, 0x08000, CRC(2ea55eae) SHA1(91d582eaa0483d547d1a37dedb8f029a0fccb526) ) // decrypted version of 10350
	ROM_LOAD16_BYTE( "epr10318.bin", 0x00000, 0x08000, CRC(d05b240d) SHA1(aa4aa7b18b53ac7b533855a44f9a4a0c14e1da2a) )
	ROM_LOAD16_BYTE( "epr10316.bin", 0x10001, 0x08000, CRC(07522474) SHA1(1a4bce4c04defab3516a90b9fa62fad1945a1176) ) // == epr10294.bin
	ROM_LOAD16_BYTE( "epr10319.bin", 0x10000, 0x08000, CRC(e48c4d80) SHA1(9c40972f81e004de84db4199e7dc0ceb6a998e76) ) // == epr10297.bin
	ROM_LOAD16_BYTE( "epr10317.bin", 0x20001, 0x08000, CRC(6e00db36) SHA1(5e3d03f7441515ad7dac411f492f2e159330a90a) )
	ROM_LOAD16_BYTE( "epr10320.bin", 0x20000, 0x08000, CRC(5f97c2fa) SHA1(5fef94bc275d9132dde8b72bfb661f10efdc91b1) )
	
	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "10281", 0x00000, 0x10000, CRC(30e92cda) SHA1(36293a2a8a22dca5350571f19f3d5d04e1b27458) )
	ROM_LOAD( "10282", 0x10000, 0x10000, CRC(b67b8910) SHA1(f3f029a3e6547114cec28e5cf8fda65ef434c353) )
	ROM_LOAD( "10283", 0x20000, 0x10000, CRC(e7dbfd2d) SHA1(91bae3fbc4a3c612dc507eecfa8de1c2e1e7afee) )

	ROM_REGION16_BE( 0x40000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "10285", 0x00001, 0x08000, CRC(98aa3d04) SHA1(1d26d17a72e55281e3444fee9c5af69ffb9e3c69) )
	ROM_LOAD16_BYTE( "10286", 0x10001, 0x08000, CRC(8da050cf) SHA1(c28e8968dbd9c110672581f4486f70d5f45df7f5) )
	ROM_LOAD16_BYTE( "10287", 0x20001, 0x08000, CRC(7989b74a) SHA1(a87acafe82b37a11d8f8b1f2ee4c9b2e1bb8161c) )
	ROM_LOAD16_BYTE( "10288", 0x30001, 0x08000, CRC(d3ce551a) SHA1(0ff2170d9ef89058273025dd8d5e1021094adef1) )
	ROM_LOAD16_BYTE( "10289", 0x00000, 0x08000, CRC(c59d1b98) SHA1(e232f2519234981c0e4ffecdd25c48083d9f93a8) )
	ROM_LOAD16_BYTE( "10290", 0x10000, 0x08000, CRC(39354223) SHA1(d8a73d3f7fc2d83d23bb7434f43bc8804f35cc16) )
	ROM_LOAD16_BYTE( "10291", 0x20000, 0x08000, CRC(6e4b245c) SHA1(1f8cecf7ea2d2dfa5ce18d7ee34b0da2cc40221e) )
	ROM_LOAD16_BYTE( "10292", 0x30000, 0x08000, CRC(cef289a3) SHA1(7ab817b6348c168f79be325fb3cc2cca14ee0f8e) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "10284", 0x00000, 0x8000, CRC(8ff09116) SHA1(8b99b6d2499897cfbd037a7e7cf5bc53bce8a63a) )
ROM_END



/*************************************
 *
 *	Generic driver initialization
 *
 *************************************/

static DRIVER_INIT( afighter )
{
	system16a_generic_init();
}


/*************************************
 *
 *	Game driver(s)
 *
 *************************************/

/* System 16A */
GAME(1986, afighter, 0, system16a_no7751, afighter, afighter, ROT270, "Sega", "Action Fighter (Unprotected)" )
/*
*gamezfan backports the Segas16a driver and video from MAME90, had to remove the pragma calls 
from machine system16 and create a new machine init for this game so that it'll work,

Since this version of Action Fighter is not protected i didn't need the FD1089A code and
i simply removed all calls for it from the driver.
*/
#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()