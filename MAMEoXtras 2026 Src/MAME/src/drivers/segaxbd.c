#pragma code_seg("C907")
#pragma data_seg("D907")
#pragma bss_seg("B907")
#pragma const_seg("K907")
#pragma comment(linker, "/merge:D907=907")
#pragma comment(linker, "/merge:C907=907")
#pragma comment(linker, "/merge:B907=907")
#pragma comment(linker, "/merge:K907=907")
/***************************************************************************

	Sega X-board hardware

	Special thanks to Charles MacDonald for his priceless assistance

***************************************************************************/

#include "driver.h"
#include "system16.h"
#include "cpu/m68000/m68000.h"
#include "machine/segaic16.h"



/*************************************
 *
 *	Statics
 *
 *************************************/

static UINT8 iochip_regs[2][8];
static UINT8 iochip_force_input;
static data8_t (*iochip_custom_io_r[2])(offs_t offset, data8_t portdata);
static void (*iochip_custom_io_w[2])(offs_t offset, data8_t data);

static UINT8 adc_reverse[8];

static UINT8 vblank_irq_state;
static UINT8 timer_irq_state;

static data16_t *backupram1, *backupram2;



/*************************************
 *
 *	Prototypes
 *
 *************************************/

extern void fd1094_machine_init(void);
extern void fd1094_driver_init(void);



/*************************************
 *
 *	Configuration
 *
 *************************************/

static void xboard_generic_init(void)
{
	/* init the FD1094 */
	fd1094_driver_init();

	/* set the default road priority */
	xboard_set_road_priority(1);

	/* reset the custom handlers and other pointers */
	memset(iochip_custom_io_r, 0, sizeof(iochip_custom_io_r));
	memset(iochip_custom_io_w, 0, sizeof(iochip_custom_io_w));
	memset(adc_reverse, 0, sizeof(adc_reverse));
	
}



/*************************************
 *
 *	Initialization & interrupts
 *
 *************************************/

static void update_main_irqs(void)
{
	int irq = 0;

	/* the IRQs are effectively ORed together */
	if (vblank_irq_state)
		irq |= 4;
	if (timer_irq_state)
		irq |= 2;

	/* assert the lines that are live, or clear everything if nothing is live */
	if (irq != 0)
		cpunum_set_input_line(0, irq, ASSERT_LINE);
	else
		cpunum_set_input_line(0, 7, CLEAR_LINE);
}


static void scanline_callback(int scanline)
{
	/* clock the timer and set the IRQ if something happened */
	if (compare_timer_clock())
		timer_irq_state = 1;

	/* on scanline 223 generate VBLANK for both CPUs */
	if (scanline == 223)
	{
		vblank_irq_state = 1;
		cpunum_set_input_line(1, 4, HOLD_LINE);
		cpu_boost_interleave(0, TIME_IN_USEC(50));
	}

	/* update IRQs on the main CPU */
	update_main_irqs();

	/* come back in 2 scanlines */
	scanline = (scanline + 2) % 262;
	timer_set(cpu_getscanlinetime(scanline), scanline, scanline_callback);
}


static void timer_ack_callback(void)
{
	timer_irq_state = 0;
	update_main_irqs();
}


static void sound_data_w(data8_t data)
{
	soundlatch_w(0, data);
	cpunum_set_input_line(2, INPUT_LINE_NMI, PULSE_LINE);
}


static void xboard_reset(void)
{
	cpunum_set_input_line(1, INPUT_LINE_RESET, PULSE_LINE);
}


static int main_irq_callback(int irq)
{
	extern int fd1094_int_callback(int irq);

	if (irq & 4)
	{
		vblank_irq_state = 0;
		update_main_irqs();
	}

	if (memory_region(REGION_USER1))
		return fd1094_int_callback(irq);
	return MC68000_INT_ACK_AUTOVECTOR;
}


MACHINE_INIT( xboard )
{
	fd1094_machine_init();
	xboard_reset_video();

	/* hook the RESET line, which resets CPU #1 */
	cpunum_set_info_fct(0, CPUINFO_PTR_M68K_RESET_CALLBACK, (genf *)xboard_reset);

	cpu_set_irq_callback(0, main_irq_callback);

	compare_timer_init(sound_data_w, timer_ack_callback);
	timer_set(cpu_getscanlinetime(1), 1, scanline_callback);
}


static void sound_cpu_irq(int state)
{
	cpunum_set_input_line(2, 0, state);
}



/*************************************
 *
 *	Input handlers
 *
 *************************************/

static READ16_HANDLER( adc_r )
{
	static const char *ports[] = { "ADC0", "ADC1", "ADC2", "ADC3", "ADC4", "ADC5", "ADC6", "ADC7" };
	int which = (iochip_regs[0][2] >> 2) & 7;

	/* on the write, latch the selected input port and stash the value */
	int value = readinputportbytag(ports[which]);

	/* reverse some port values */
	if (adc_reverse[which])
		value = 255 - value;

	/* return the previously latched value */
	return value;
}


static WRITE16_HANDLER( adc_w )
{
}


INLINE data16_t iochip_r(int which, int port, int inputval)
{
	data16_t result = iochip_regs[which][port];

	/* if there's custom I/O, do that to get the input value */
	if (iochip_custom_io_r[which])
		inputval = (*iochip_custom_io_r[which])(port, inputval);

	/* for ports 0-3, the direction is controlled 4 bits at a time by register 6 */
	if (port <= 3)
	{
		if (iochip_force_input || ((iochip_regs[which][6] >> (2*port+0)) & 1))
			result = (result & ~0x0f) | (inputval & 0x0f);
		if (iochip_force_input || ((iochip_regs[which][6] >> (2*port+1)) & 1))
			result = (result & ~0xf0) | (inputval & 0xf0);
	}

	/* for port 4, the direction is controlled 1 bit at a time by register 7 */
	else
	{
		if ((iochip_regs[which][7] >> 0) & 1)
			result = (result & ~0x01) | (inputval & 0x01);
		if ((iochip_regs[which][7] >> 1) & 1)
			result = (result & ~0x02) | (inputval & 0x02);
		if ((iochip_regs[which][7] >> 2) & 1)
			result = (result & ~0x04) | (inputval & 0x04);
		if ((iochip_regs[which][7] >> 3) & 1)
			result = (result & ~0x08) | (inputval & 0x08);
		result &= 0x0f;
	}
	return result;
}


static READ16_HANDLER( iochip_0_r )
{
	switch (offset & 7)
	{
		case 0:
			/* Input port:
				D7: (Not connected)
				D6: /INTR of ADC0804
				D5-D0: CN C pin 24-19 (switch state 0= open, 1= closed)
			*/
			return iochip_r(0, 0, readinputport(0));

		case 1:
			/* I/O port: CN C pins 17,15,13,11,9,7,5,3 */
			return iochip_r(0, 1, readinputport(1));

		case 2:
			/* Output port */
			return iochip_r(0, 2, 0);

		case 3:
			/* Output port */
			return iochip_r(0, 3, 0);

		case 4:
			/* Unused */
			return iochip_r(0, 4, 0);
	}

	/* everything else returns 0 */
	return 0;
}


static WRITE16_HANDLER( iochip_0_w )
{
	UINT8 oldval;

	/* access is via the low 8 bits */
	if (!ACCESSING_LSB)
		return;
	offset &= 7;
	data &= 0xff;

	/* swap in the new value and remember the previous value */
	oldval = iochip_regs[0][offset];
	iochip_regs[0][offset] = data;

	/* certain offsets have common effects */
	switch (offset)
	{
		case 2:
			/* Output port:
				D7: (Not connected)
				D6: (/WDC) - watchdog reset
				D5: Screen display (1= blanked, 0= displayed)
				D4-D2: (ADC2-0)
				D1: (CONT) - affects sprite hardware
				D0: Sound section reset (1= normal operation, 0= reset)
			*/
			xboard_set_draw_enable((data >> 5) & 1);
//			if ((oldval ^ data) & 2) printf("CONT = %d\n", (data >> 1) & 1);
//			if ((oldval ^ data) & 1) cpunum_set_input_line(2, INPUT_LINE_RESET, PULSE_LINE);
			break;

		case 3:
			/* Output port:
				D7: Amplifier mute control (1= sounding, 0= muted)
				D6-D0: CN D pin A17-A23 (output level 1= high, 0= low)
			*/
			break;
	}

	if (offset <= 4)
		logerror("I/O chip 0, port %c write = %02X\n", 'A' + offset, data);
}


static READ16_HANDLER( iochip_1_r )
{
	switch (offset & 7)
	{
		case 0:
			/* Input port: switches, CN D pin A1-8 (switch state 1= open, 0= closed) */
			return iochip_r(1, 0, readinputport(2));

		case 1:
			/* Input port: switches, CN D pin A9-16 (switch state 1= open, 0= closed) */
			return iochip_r(1, 1, readinputport(3));

		case 2:
			/* Input port: DIP switches (1= off, 0= on) */
			return iochip_r(1, 2, readinputport(4));

		case 3:
			/* Input port: DIP switches (1= off, 0= on) */
			return iochip_r(1, 3, readinputport(5));

		case 4:
			/* Unused */
			return iochip_r(1, 4, 0);
	}

	/* everything else returns 0 */
	return 0;
}


static WRITE16_HANDLER( iochip_1_w )
{
	UINT8 oldval;

	/* access is via the low 8 bits */
	if (!ACCESSING_LSB)
		return;
	offset &= 7;
	data &= 0xff;

	/* swap in the new value and remember the previous value */
	oldval = iochip_regs[1][offset];
	iochip_regs[1][offset] = data;

	if (offset <= 4)
		logerror("I/O chip 1, port %c write = %02X\n", 'A' + offset, data);
}


static WRITE16_HANDLER( iocontrol_w )
{
	if (ACCESSING_LSB)
	{
		logerror("I/O chip force input = %d\n", data & 1);
		/* Racing Hero and ABCop set this and fouls up their output ports */
		/*iochip_force_input = data & 1;*/
	}
}



/*************************************
 *
 *	Line of Fire Custom I/O
 *
 *************************************/
 
static VIDEO_UPDATE( loffire )
{
	int x1 = readinputportbytag("ADC0");
	int y1 = readinputportbytag("ADC1");
	int x2 = readinputportbytag("ADC2");
	int y2 = readinputportbytag("ADC3");
	video_update_xboard(bitmap, cliprect);
	draw_crosshair(bitmap, x1 * (Machine->drv->screen_width - 1) / 255, y1 * (Machine->drv->screen_height - 1) / 255, cliprect);
	draw_crosshair(bitmap, x2 * (Machine->drv->screen_width - 1) / 255, y2 * (Machine->drv->screen_height - 1) / 255, cliprect);
}



/*************************************
 *
 *	Capacitor-backed RAM
 *
 *************************************/

static NVRAM_HANDLER( xboard )
{
	if (read_or_write)
	{
		mame_fwrite(file, backupram1, 0x4000);
		mame_fwrite(file, backupram2, 0x4000);
	}
	else if (file)
	{
		mame_fread(file, backupram1, 0x4000);
		mame_fread(file, backupram2, 0x4000);
	}
}



/*************************************
 *
 *	Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) | AMEF_ABITS(22) )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE(1) AM_BASE(&backupram1)
	AM_RANGE(0x0a0000, 0x0a3fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE(2) AM_BASE(&backupram2)
	AM_RANGE(0x0c0000, 0x0cffff) AM_READWRITE(MRA16_RAM, segaic16_tileram_w) AM_BASE(&segaic16_tileram)
	AM_RANGE(0x0d0000, 0x0d0fff) AM_MIRROR(0x00f000) AM_READWRITE(MRA16_RAM, xboard_textram_w) AM_BASE(&segaic16_textram)
	AM_RANGE(0x0e0000, 0x0e0007) AM_MIRROR(0x003ff8) AM_READWRITE(segaic16_multiply_0_r, segaic16_multiply_0_w)
	AM_RANGE(0x0e4000, 0x0e401f) AM_MIRROR(0x003fe0) AM_READWRITE(segaic16_divide_0_r, segaic16_divide_0_w)
	AM_RANGE(0x0e8000, 0x0e801f) AM_MIRROR(0x003fe0) AM_READWRITE(segaic16_compare_timer_0_r, segaic16_compare_timer_0_w)
	AM_RANGE(0x100000, 0x100fff) AM_MIRROR(0x00f000) AM_RAM AM_BASE(&segaic16_spriteram)
	AM_RANGE(0x110000, 0x11ffff) AM_WRITE(xboard_render_start_w)
	AM_RANGE(0x120000, 0x123fff) AM_MIRROR(0x00c000) AM_READWRITE(MRA16_RAM, segaic16_paletteram_w) AM_BASE(&paletteram16)
	AM_RANGE(0x130000, 0x13ffff) AM_READWRITE(adc_r, adc_w)
	AM_RANGE(0x140000, 0x14ffff) AM_READWRITE(iochip_0_r, iochip_0_w)
	AM_RANGE(0x150000, 0x15ffff) AM_READWRITE(iochip_1_r, iochip_1_w)
	AM_RANGE(0x160000, 0x16ffff) AM_WRITE(iocontrol_w)
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION(REGION_CPU2, 0x00000)
	AM_RANGE(0x280000, 0x283fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE(3)
	AM_RANGE(0x2a0000, 0x2a3fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE(4)
	AM_RANGE(0x2e0000, 0x2e0007) AM_MIRROR(0x003ff8) AM_READWRITE(segaic16_multiply_1_r, segaic16_multiply_1_w)
	AM_RANGE(0x2e4000, 0x2e401f) AM_MIRROR(0x003fe0) AM_READWRITE(segaic16_divide_1_r, segaic16_divide_1_w)
	AM_RANGE(0x2e8000, 0x2e800f) AM_MIRROR(0x003ff0) AM_READWRITE(segaic16_compare_timer_1_r, segaic16_compare_timer_1_w)
	AM_RANGE(0x2ec000, 0x2ecfff) AM_MIRROR(0x001000) AM_RAM AM_SHARE(5) AM_BASE(&segaic16_roadram)
	AM_RANGE(0x2ee000, 0x2effff) AM_READWRITE(xboard_road_latch_r, xboard_road_control_w)
	AM_RANGE(0x3f8000, 0x3fbfff) AM_RAM AM_SHARE(1)
	AM_RANGE(0x3fc000, 0x3fffff) AM_RAM AM_SHARE(2)
ADDRESS_MAP_END



/*************************************
 *
 *	Sub CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sub_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) | AMEF_ABITS(20) )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE(3)
	AM_RANGE(0x0a0000, 0x0a3fff) AM_MIRROR(0x01c000) AM_RAM AM_SHARE(4)
	AM_RANGE(0x0e0000, 0x0e0007) AM_MIRROR(0x003ff8) AM_READWRITE(segaic16_multiply_1_r, segaic16_multiply_1_w)
	AM_RANGE(0x0e4000, 0x0e401f) AM_MIRROR(0x003fe0) AM_READWRITE(segaic16_divide_1_r, segaic16_divide_1_w)
	AM_RANGE(0x0e8000, 0x0e800f) AM_MIRROR(0x003ff0) AM_READWRITE(segaic16_compare_timer_1_r, segaic16_compare_timer_1_w)
	AM_RANGE(0x0ec000, 0x0ecfff) AM_MIRROR(0x001000) AM_RAM AM_SHARE(5)
	AM_RANGE(0x0ee000, 0x0effff) AM_READWRITE(xboard_road_latch_r, xboard_road_control_w)
//	AM_RANGE(0x0f0000, 0x0f3fff) AM_READWRITE(excs_r, excs_w)
ADDRESS_MAP_END



/*************************************
 *
 *	Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf0ff) AM_MIRROR(0x0700) AM_READWRITE(SegaPCM_r, SegaPCM_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x3e) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x3e) AM_READWRITE(YM2151_status_port_0_r, YM2151_data_port_0_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_READ(soundlatch_r)
ADDRESS_MAP_END



/*************************************
 *
 *	Generic port definitions
 *
 *************************************/

/*
	aburner chip 0, port A: motor status (R)
			chip 0, port B: motor power (W)
			chip 0, port C: unknown (W)
			chip 0, port D: lamp (W)
			chip 1, port A: buttons
			chip 1, port B: ---
			chip 2, port C: DIPs
			chip 3, port D: DIPs
*/

static INPUT_PORTS_START( abcop )
	PORT_START_TAG("IO0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )	/* /INTR of ADC0804 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IO0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IO1PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IO1PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IO1PORTC")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
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
	PORT_DIPSETTING(    0x50, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START_TAG("IO1PORTD")
	PORT_DIPNAME( 0x01, 0x01, "Credits" )
	PORT_DIPSETTING(    0x01, "1 to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x00, "2 to Start, 1 to Continue" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Allow_Continue" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Time" )
	PORT_DIPSETTING(    0x10, "Easy" )
	PORT_DIPSETTING(    0x30, "Normal" )
	PORT_DIPSETTING(    0x20, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0xc0, "Normal" )
	PORT_DIPSETTING(    0x80, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("ADC0")	/* steering */
	PORT_ANALOG( 0xff, 0x7f, IPT_PADDLE | IPF_REVERSE, 100, 4, 0x00,0xff)

	PORT_START_TAG("ADC1")	/* accelerator */
	PORT_ANALOG( 0xff, 0x00, IPT_PEDAL, 10, 15, 0x00,0xff)
	
	//dummy handlers seems to need this or it wont boot
	PORT_START_TAG("ADC2")
	PORT_START_TAG("ADC3")
	PORT_START_TAG("ADC4")
	PORT_START_TAG("ADC5")
	PORT_START_TAG("ADC6")
	PORT_START_TAG("ADC7")
INPUT_PORTS_END

static INPUT_PORTS_START( rachero )
	PORT_START_TAG("IO0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )	/* /INTR of ADC0804 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IO0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IO1PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IO1PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IO1PORTC")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
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
	PORT_DIPSETTING(    0x50, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START_TAG("IO1PORTD")
	PORT_DIPNAME( 0x01, 0x01, "Credits" )
	PORT_DIPSETTING(    0x01, "1 to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x00, "2 to Start, 1 to Continue" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Allow_Continue" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Time" )
	PORT_DIPSETTING(    0x10, "Easy" )
	PORT_DIPSETTING(    0x30, "Normal" )
	PORT_DIPSETTING(    0x20, "Hard" )
	PORT_DIPSETTING(    0x00, "Very_Hard" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0xc0, "Normal" )
	PORT_DIPSETTING(    0x80, "Hard" )
	PORT_DIPSETTING(    0x00, "Very_Hard" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	
	PORT_START_TAG("ADC0")	/* steering */
	PORT_ANALOG( 0xff, 0x7f, IPT_PADDLE | IPF_REVERSE, 100, 4, 0x00,0xff)

	PORT_START_TAG("ADC1")	/* gas pedal */
	PORT_ANALOG( 0xff, 0x00, IPT_PEDAL, 10, 15, 0x00,0xff)

	PORT_START_TAG("ADC2")	/* brake */
	PORT_ANALOG( 0xff, 0x00, IPT_PEDAL2, 10, 25, 0x00,0xff)
	
	//dummy handlers seems to need this or it wont boot
	PORT_START_TAG("ADC3")
	PORT_START_TAG("ADC4")
	PORT_START_TAG("ADC5")
	PORT_START_TAG("ADC6")
	PORT_START_TAG("ADC7")
INPUT_PORTS_END

static INPUT_PORTS_START( smgp )
	PORT_START_TAG("IO0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )	/* /INTR of ADC0804 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IO0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IO1PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IO1PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IO1PORTC")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
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
	PORT_DIPSETTING(    0x50, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START_TAG("IO1PORTD")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x07, 0x07, "Machine ID" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, "Number of Machines" )
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, "Cockpit" )
	PORT_DIPSETTING(    0xc0, "Deluxe" )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
//	PORT_DIPSETTING(    0x00, "Deluxe" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("ADC0")	/* steering */
	PORT_ANALOG( 0xff, 0x7f, IPT_PADDLE, 50, 4, 0x00,0xff)

	PORT_START_TAG("ADC1")	/* gas pedal */
	PORT_ANALOG( 0xff, 0x00, IPT_PEDAL, 10, 15, 0x00,0xff) 

	PORT_START_TAG("ADC2")	/* brake */
	PORT_ANALOG( 0xff, 0x00, IPT_PEDAL2, 10, 25, 0x00,0xff)
	
	//dummy handlers seems to need this or it wont boot
	PORT_START_TAG("ADC3")
	PORT_START_TAG("ADC4")
	PORT_START_TAG("ADC5")
	PORT_START_TAG("ADC6")
	PORT_START_TAG("ADC7")
INPUT_PORTS_END


static INPUT_PORTS_START( thndrbld )
	
	PORT_START_TAG("IO0PORTA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )	/* /INTR of ADC0804 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IO0PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IO1PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START_TAG("IO1PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IO1PORTC")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
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
	PORT_DIPSETTING(    0x50, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START_TAG("IO1PORTD")
	PORT_DIPNAME( 0x01, 0x01, "Cabinet Type" )
	PORT_DIPSETTING(    0x01, "Econ Upright" )
	PORT_DIPSETTING(    0x00, "Mini Upright" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Time" )
	PORT_DIPSETTING(    0x04, "30 sec" )
	PORT_DIPSETTING(    0x00, "0 sec" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Number of Ships" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, "Allow_Continue" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, "Easy" )
	PORT_DIPSETTING(    0xc0, "Normal" )
	PORT_DIPSETTING(    0x80, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )


	PORT_START_TAG("ADC0")	/* stick X */
	PORT_ANALOG( 0xff, 0x7f, IPT_AD_STICK_X | IPF_REVERSE, 100, 4, 0x00, 0xff)

	PORT_START_TAG("ADC1")	/* "slottle" */
	PORT_ANALOG( 0xff, 0x80, IPT_AD_STICK_Z | IPF_REVERSE, 100, 79, 0x00, 0xff)

	PORT_START_TAG("ADC2")	/* stick Y */
	PORT_ANALOG( 0xff, 0x7f, IPT_AD_STICK_Y, 100, 4, 0x00, 0xff)

	//dummy handlers seems to need this or it wont boot
	PORT_START_TAG("ADC3")
	PORT_START_TAG("ADC4")
	PORT_START_TAG("ADC5")
	PORT_START_TAG("ADC6")
	PORT_START_TAG("ADC7")
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
	{ YM3012_VOL(80,MIXER_PAN_LEFT,80,MIXER_PAN_RIGHT) },
	{ sound_cpu_irq }
};


static struct SEGAPCMinterface segapcm_interface =
{
	SEGAPCM_SAMPLE15K,
	BANK_512,
	REGION_SOUND1,
	100
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
 *	Generic machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( xboard )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(main_map,0)

	MDRV_CPU_ADD_TAG("sub", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(sub_map,0)

	MDRV_CPU_ADD_TAG("sound", Z80, 4000000)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_IO_MAP(sound_portmap,0)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(1000000 * (262 - 224) / (262 * 60))

	MDRV_MACHINE_INIT(xboard)
	MDRV_NVRAM_HANDLER(xboard)
	MDRV_INTERLEAVE(10)// make it this to speed things up

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(40*8, 28*8)
	MDRV_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(8192*3)

	MDRV_VIDEO_START(xboard)
	MDRV_VIDEO_UPDATE(xboard)

	/* sound hardware */
	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD_TAG("2151", YM2151, ym2151_interface)
	MDRV_SOUND_ADD_TAG("pcm", SEGAPCM, segapcm_interface)
MACHINE_DRIVER_END


/*************************************
 *
 *	ROM definition(s)
 *
 *************************************/

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	AB Cop, Sega X-board
	CPU: FD1094 (317-0169b)
*/
ROM_START( abcop )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr13568b.58", 0x00000, 0x20000, CRC(f88db35b) SHA1(7d85c1194a2aa08427333d2ffc2a8d4f7e1beff0) )
	ROM_LOAD16_BYTE( "epr13556b.63", 0x00001, 0x20000, CRC(337bf32e) SHA1(dafb9d9b3baf79ca76355278e8a14294f186790a) )
	ROM_LOAD16_BYTE( "epr13559.57",  0x40000, 0x20000, CRC(4588bf19) SHA1(6a8b3d4450ac0bc41b46e6a4e1b44d82112fcd64) )
	ROM_LOAD16_BYTE( "epr13558.62",  0x40001, 0x20000, CRC(11259ed4) SHA1(e7de174a0bdb1d1111e5e419f1d501ab5be1d32d) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0169b.key", 0x0000, 0x2000, CRC(058da36e) SHA1(ab3f68a90725063c68fc5d0f8dbece1f8940dc7d) )

	ROM_REGION( 0x80000, REGION_CPU2, 0 ) /* 2nd 68000 code */
	ROM_LOAD16_BYTE( "epr13566.20", 0x00000, 0x20000, CRC(22e52f32) SHA1(c67a4ccb88becc58dddcbfea0a1ac2017f7b2929) )
	ROM_LOAD16_BYTE( "epr13565.29", 0x00001, 0x20000, CRC(a21784bd) SHA1(b40ba0ef65bbfe514625253f6aeec14bf4bcf08c) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "opr13553.154", 0x00000, 0x10000, CRC(8c418837) SHA1(e325db39fae768865e20d2cd1ee2b91a9b0165f5) )
	ROM_LOAD( "opr13554.153", 0x10000, 0x10000, CRC(4e3df9f0) SHA1(8b481c2cd25c58612ac8ac3ffb7eeae9ca247d2e) )
	ROM_LOAD( "opr13555.152", 0x20000, 0x10000, CRC(6c4a1d42) SHA1(6c37b045b21173f1e2f7bd19d01c00979b8107fb) )

	ROM_REGION32_LE( 0x200000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD32_BYTE( "opr13552.90",  0x000000, 0x20000, CRC(cc2cf706) SHA1(ad39c22e652ebcd90ffb5e17ae35985645f93c71) )
	ROM_LOAD32_BYTE( "opr13551.94",  0x000001, 0x20000, CRC(d6f276c1) SHA1(9ec68157ea460e09ef4b69aa8ea17687dc47ea59) )
	ROM_LOAD32_BYTE( "opr13550.98",  0x000002, 0x20000, CRC(f16518dd) SHA1(a5f1785cd28f03069cb238ac92c6afb5a26cbd37) )
	ROM_LOAD32_BYTE( "opr13549.102", 0x000003, 0x20000, CRC(cba407a7) SHA1(e7684d3b40baa6d832b887fd85ad67fbad8aa7de) )
	ROM_LOAD32_BYTE( "opr13548.91",  0x080000, 0x20000, CRC(080fd805) SHA1(e729565815a3a37462cfee460b7392d2f08e96e5) )
	ROM_LOAD32_BYTE( "opr13547.95",  0x080001, 0x20000, CRC(42d4dd68) SHA1(6ae1f3585ebb20fd2908456d6fa41a893261277e) )
	ROM_LOAD32_BYTE( "opr13546.99",  0x080002, 0x20000, CRC(ca6fbf3d) SHA1(49c3516d87f1546fa7efe785fc5c064d90b1cb8e) )
	ROM_LOAD32_BYTE( "opr13545.103", 0x080003, 0x20000, CRC(c9e58dd2) SHA1(ace2e1630d8df2454183ffdbe26d8cb6d199e940) )
	ROM_LOAD32_BYTE( "opr13544.92",  0x100000, 0x20000, CRC(9c1436d9) SHA1(5156e1b5c7461f6dc0d449b86b6b72153b290a4c) )
	ROM_LOAD32_BYTE( "opr13543.96",  0x100001, 0x20000, CRC(2c1c8f0e) SHA1(19c9fd4272a3db18381f435ed6cd01f994c655e7) )
	ROM_LOAD32_BYTE( "opr13542.100", 0x100002, 0x20000, CRC(01fd52b8) SHA1(b4ab13c7b2b2ffcfdab37d8e4855d5ef8823f1cc) )
	ROM_LOAD32_BYTE( "opr13541.104", 0x100003, 0x20000, CRC(a45c547b) SHA1(d93aaa850d14a7699a1b0411e823088a9bce7553) )
	ROM_LOAD32_BYTE( "opr13540.93",  0x180000, 0x20000, CRC(84b42ab0) SHA1(d24ba7fe23463fc5813ef26e0395951559d6d162) )
	ROM_LOAD32_BYTE( "opr13539.97",  0x180001, 0x20000, CRC(cd6e524f) SHA1(e6df2552a84b2da95301486379c78679b0297634) )
	ROM_LOAD32_BYTE( "opr13538.101", 0x180002, 0x20000, CRC(bf9a4586) SHA1(6013dee83375d72d262c8c04c2e668afea2e216c) )
	ROM_LOAD32_BYTE( "opr13537.105", 0x180003, 0x20000, CRC(fa14ed3e) SHA1(d684496ade2517696a56c1423dd4686d283c133f) )

	ROM_REGION( 0x10000, REGION_GFX3, 0 ) /* ground data */
	ROM_LOAD( "opr13564.40",	 0x00000, 0x10000, CRC(e70ba138) SHA1(85eb6618f408642227056d278f10dec8dcc5a80d) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 ) /* sound CPU */
	ROM_LOAD( "epr13560.17",    0x00000, 0x10000, CRC(83050925) SHA1(118710e5789c7999bb7326df4d7bd207cbffdfd4) )

	ROM_REGION( 0x60000, REGION_SOUND1, 0 ) /* Sega PCM sound data */
	ROM_LOAD( "opr13563.11",    0x00000, 0x20000, CRC(4083e74f) SHA1(e48c7ce0aa3406af0bbf79c169a8157693c97041) )
	ROM_LOAD( "opr13562.12",    0x20000, 0x20000, CRC(3cc3968f) SHA1(d25647f6a3fa939ba30e03e7334362ef0749b23a) )
	ROM_LOAD( "opr13561.13",    0x40000, 0x20000, CRC(80a7c02a) SHA1(7e8c1b9ba270d8657dbe90ed8be2e4b6463e5928) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Racing Hero, Sega X-board
	CPU: FD1094 (317-0144)
*/
ROM_START( rachero )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr13129.58", 0x00000, 0x20000,CRC(ad9f32e7) SHA1(dbcb3436782bee88dcac05d4f59c97f170a7387d) )
	ROM_LOAD16_BYTE( "epr13130.63", 0x00001, 0x20000,CRC(6022777b) SHA1(965c76565d740be3355c4b403a1629cffb9fcd78) )
	ROM_LOAD16_BYTE( "epr12855.57", 0x40000, 0x20000,CRC(cecf1e73) SHA1(3f8631379f32dbfda7720ef345276f9be23ada06) )
	ROM_LOAD16_BYTE( "epr12856.62", 0x40001, 0x20000,CRC(da900ebb) SHA1(595ed65248185ddf8666b3f30ad6329162116448) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0144.key", 0x0000, 0x2000, CRC(8740bbff) SHA1(de96e606c04a09258b966532fb01a6b4d4db86a6) )

	ROM_REGION( 0x80000, REGION_CPU2, 0 ) /* 2nd 68000 code */
	ROM_LOAD16_BYTE( "epr12857.20", 0x00000, 0x20000, CRC(8a2328cc) SHA1(c34498428ddfb3eeb986f4153a6165a685d8fc8a) )
	ROM_LOAD16_BYTE( "epr12858.29", 0x00001, 0x20000, CRC(38a248b7) SHA1(a17672123665403c1c56fedab6c8abf44b1131f9) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "epr12879.154", 0x00000, 0x10000, CRC(c1a9de7a) SHA1(2425456a9d4ba92e1f2da6c2f164a6d5a5dee7c7) )
	ROM_LOAD( "epr12880.153", 0x10000, 0x10000, CRC(27ff04a5) SHA1(b554a6e060f4803100be8efa52977b503eb0f31d) )
	ROM_LOAD( "epr12881.152", 0x20000, 0x10000, CRC(72f14491) SHA1(b7a6cbd08470a5edda77cdd0337abd502c4905fd) )

	ROM_REGION32_LE( 0x200000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD32_BYTE( "epr12872.90",  0x000000, 0x20000, CRC(68d56139) SHA1(b5f32edbda10c31d52f90defea2bae226676069f) )
	ROM_LOAD32_BYTE( "epr12873.94",  0x000001, 0x20000, CRC(3d3ec450) SHA1(ac96ad8c7b365478bd1e5826a073e242f1208247) )
	ROM_LOAD32_BYTE( "epr12874.98",  0x000002, 0x20000, CRC(7d6bde23) SHA1(88b12ec6386cdad60b0028b72033a0037a0cdbdb) )
	ROM_LOAD32_BYTE( "epr12875.102", 0x000003, 0x20000, CRC(e33092bf) SHA1(31e211e25adac0a98befb459093f23c905fbc1e6) )
	ROM_LOAD32_BYTE( "epr12868.91",  0x080000, 0x20000, CRC(96289583) SHA1(4d37e67860bc0e6ef69f0a0775c28f6f2fd6875e) )
	ROM_LOAD32_BYTE( "epr12869.95",  0x080001, 0x20000, CRC(2ef0de02) SHA1(11ee3d77df2cddd3156da52e50565505f95f4cd4) )
	ROM_LOAD32_BYTE( "epr12870.99",  0x080002, 0x20000, CRC(c76630e1) SHA1(7b76e4819990e147639d6b930b17b6fa10df191c) )
	ROM_LOAD32_BYTE( "epr12871.103", 0x080003, 0x20000, CRC(23401b1a) SHA1(eaf465ffda84bdb83cc85daf781275bada396aab) )
	ROM_LOAD32_BYTE( "epr12864.92",  0x100000, 0x20000, CRC(77d6cff4) SHA1(1e625204801d03369311844efb26d22216253ac4) )
	ROM_LOAD32_BYTE( "epr12865.96",  0x100001, 0x20000, CRC(1e7e685b) SHA1(532fe361357383aa9dada833cbe31716c58001e5) )
	ROM_LOAD32_BYTE( "epr12866.100", 0x100002, 0x20000, CRC(fdf31329) SHA1(9c229a0f9d8b8114acfe4f17b45a9b8640560b3e) )
	ROM_LOAD32_BYTE( "epr12867.104", 0x100003, 0x20000, CRC(b25e37fd) SHA1(fef5bfe4690b3203b83fd565d883b2c63f439633) )
	ROM_LOAD32_BYTE( "epr12860.93",  0x180000, 0x20000, CRC(86b64119) SHA1(d39aedad0f05e500e33af888126bd2fc22539141) )
	ROM_LOAD32_BYTE( "epr12861.97",  0x180001, 0x20000, CRC(bccff19b) SHA1(32c3f7802a12be02a114b78cd898c46fcb1c0a61) )
	ROM_LOAD32_BYTE( "epr12862.101", 0x180002, 0x20000, CRC(7d4c3b05) SHA1(4e25a077b403549c681c5047912d0e28f4c07720) )
	ROM_LOAD32_BYTE( "epr12863.105", 0x180003, 0x20000, CRC(85095053) SHA1(f93194ecc0300956280cc0515b3e3ba2c9f71364) )

	ROM_REGION( 0x10000, REGION_GFX3, 0 ) /* ground data */
	/* none */

	ROM_REGION( 0x10000, REGION_CPU3, 0 ) /* sound CPU */
	ROM_LOAD( "epr12859.17",    0x00000, 0x10000, CRC(d57881da) SHA1(75b7f331ea8c2e33d6236e0c8fc8dabe5eef8160) )

	ROM_REGION( 0x60000, REGION_SOUND1, 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr12876.11",    0x00000, 0x20000, CRC(f72a34a0) SHA1(28f7d077c24352557da3a91a7e49b0c5b79f2a2e) )
	ROM_LOAD( "epr12877.12",    0x20000, 0x20000, CRC(18c1b6d2) SHA1(860cbb96999ab76c40ce96996bba70c42d845abc) )
	ROM_LOAD( "epr12878.13",    0x40000, 0x20000, CRC(7c212c15) SHA1(360b332d2fb32d88949ff8b357a863ffaaca39c2) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Super Monaco GP, Sega X-board
	CPU: FD1094 (317-0125a)
*/
ROM_START( smgp )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ep12563b.58", 0x00000, 0x20000, CRC(baf1f333) SHA1(f91a7a311237b9940a44b815716d4226a7ae1e8b) )
	ROM_LOAD16_BYTE( "ep12564b.63", 0x00001, 0x20000, CRC(b5191af0) SHA1(d6fb19552e4816eefe751907ec55a2e07ad24879) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0126a.key", 0x0000, 0x2000, CRC(2abc1982) SHA1(cc4c36e6ba52431df17c6e36ba08d3a89be7b7e7) )

	ROM_REGION( 0x80000, REGION_CPU2, 0 ) /* 2nd 68000 code */
	ROM_LOAD16_BYTE( "epr12576a.20", 0x00000, 0x20000, CRC(2c9599c1) SHA1(79206f38c2976bd9299ed37bf62ac26dd3fba801) )
	ROM_LOAD16_BYTE( "epr12577a.29", 0x00001, 0x20000, CRC(abf9a50b) SHA1(e367b305cd45900aae4849af4904543f05456dc6) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "epr12429.154", 0x00000, 0x10000, CRC(5851e614) SHA1(3dc97237ede2c6125e92ea6efc68a748d0ec69be) )
	ROM_LOAD( "epr12430.153", 0x10000, 0x10000, CRC(05e00134) SHA1(8baaa80815d5dabd38dc8600e357975b96d23b95) )
	ROM_LOAD( "epr12431.152", 0x20000, 0x10000, CRC(35572f4a) SHA1(d66456ecf7b59f81736fb873c553926b56bb3977))

	ROM_REGION32_LE( 0x200000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD32_BYTE( "mpr12425.90",  0x000000, 0x20000, CRC(14bf2a15) SHA1(84db3ac09e4a8fe470ac051d8d5de1814b48bc72) )
	ROM_LOAD32_BYTE( "mpr12426.94",  0x000001, 0x20000, CRC(28b60dc0) SHA1(ad69d449434853445a076319a55a29014217a100) )
	ROM_LOAD32_BYTE( "mpr12427.98",  0x000002, 0x20000, CRC(0a367928) SHA1(bcb558b7c23906397e66a7f046b09eb5036c0888) )
	ROM_LOAD32_BYTE( "mpr12428.102", 0x000003, 0x20000, CRC(efa80ad5) SHA1(9bc7c3fb60cc076f29a0af487d58e5b48f1c4b06) )
	ROM_LOAD32_BYTE( "mpr12421.91",  0x080000, 0x20000, CRC(25f46140) SHA1(ea75e364cf52636d100158f79be627e36da8c327) )
	ROM_LOAD32_BYTE( "mpr12422.95",  0x080001, 0x20000, CRC(cb51c8f6) SHA1(5af56ae1916c3212b8d5b9e4bccbbe1916694f89) )
	ROM_LOAD32_BYTE( "mpr12423.99",  0x080002, 0x20000, CRC(0be9818e) SHA1(637a8201416e73d53f7e2502ea0a5277e43c167d) )
	ROM_LOAD32_BYTE( "mpr12424.103", 0x080003, 0x20000, CRC(0ce00dfc) SHA1(3b1990977ec7ad4c3bea66527707cff2cd8d5a98) )
	ROM_LOAD32_BYTE( "mpr12417.92",  0x100000, 0x20000, CRC(a806eabf) SHA1(1a61a2135d92b42ee131fd3240bc8a17a96696ab) )
	ROM_LOAD32_BYTE( "mpr12418.96",  0x100001, 0x20000, CRC(ed1a0f2b) SHA1(1aa87292ca0465fa129d6be81d95dbb77332ecab) )
	ROM_LOAD32_BYTE( "mpr12419.100", 0x100002, 0x20000, CRC(ce4568cb) SHA1(1ed66e74ce94d41593b498827d9cc243f775d4ba) )
	ROM_LOAD32_BYTE( "mpr12420.104", 0x100003, 0x20000, CRC(679442eb) SHA1(f88ef0219497f955d8db6783f3636dad52928f46) )
	ROM_LOAD32_BYTE( "epr12609.93",  0x180000, 0x20000, CRC(a867812f) SHA1(f8950bf794b6c2ec767ffff837d28917b636dbe7) ) //
	ROM_LOAD32_BYTE( "epr12610.97",  0x180001, 0x20000, CRC(53b99417) SHA1(ab72d35c88695c777d24c5557e5d3ea2d446e51b) ) //
	ROM_LOAD32_BYTE( "epr12611.101", 0x180002, 0x20000, CRC(bd5c6ab0) SHA1(7632dc4daa8eabe74769369856a8ba451e5bd420) ) // these differ from japan set
	ROM_LOAD32_BYTE( "epr12612.105", 0x180003, 0x20000, CRC(ac86e890) SHA1(7720c1c8df6de5de50254e97772c15161b796031) ) //

	ROM_REGION( 0x10000, REGION_GFX3, 0 ) /* road gfx */
	/* none?? */

	ROM_REGION( 0x10000, REGION_CPU3, 0 ) /* sound CPU */
	ROM_LOAD( "epr12436.17",    0x00000, 0x10000, CRC(16ec5f0a) SHA1(307b7388b5c36fd4bc2a61f7941db44858e03c5c) )

	ROM_REGION( 0x60000, REGION_SOUND1, 0 ) /* Sega PCM sound data */
	ROM_LOAD( "mpr12437.11",    0x00000, 0x20000, CRC(a1c7e712) SHA1(fa7fa8c39690ae5dab8b28af5aeed5ffae2cd6de) )
	ROM_LOAD( "mpr12438.12",    0x20000, 0x20000, CRC(6573d46b) SHA1(c4a4a0ea35250eff28a5bfd5e9cd372f52fd1308) )
	ROM_LOAD( "mpr12439.13",    0x40000, 0x20000, CRC(13bf6de5) SHA1(92228a05ec33d606491a1da98c4989f69cddbb49) )

	ROM_REGION( 0x10000, REGION_CPU4, 0 ) /* comms */
	ROM_LOAD( "epr12587.14",    0x00000, 0x8000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Thunder Blade, Sega X-board
	CPU: FD1094 (317-0056)
*/
ROM_START( thndrbld )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr11405.58", 0x000000, 0x20000, CRC(e057dd5a) SHA1(4c032db4752dfb44dba3def5ee5377fffd94b79c) )
	ROM_LOAD16_BYTE( "epr11406.63", 0x000001, 0x20000, CRC(c6b994b8) SHA1(098b2ae30c4aafea35222369d60f8e89f87639eb) )
	ROM_LOAD16_BYTE( "11306.epr",   0x040000, 0x20000, CRC(4b95f2b4) SHA1(9e0ff898a2af05c35db3551e52c7485748698c28) )
	ROM_LOAD16_BYTE( "11307.epr",   0x040001, 0x20000, CRC(2d6833e4) SHA1(b39a744370014237121f0010d18897e63f7058cf) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0056.key", 0x0000, 0x2000, CRC(b40cd2c5) SHA1(865e70bce4f55f6702960d6eaa780b7b1f880e41) )

	ROM_REGION( 0x100000, REGION_CPU2, 0 ) /* 2nd 68000 code */
	ROM_LOAD16_BYTE( "epr11390.20", 0x000000, 0x20000, CRC(ed988fdb) SHA1(b809b0b7dabd5cb29f5387522c6dfb993d1d0271) )
	ROM_LOAD16_BYTE( "epr11391.29", 0x000001, 0x20000, CRC(12523bc1) SHA1(54635d6c4cc97cf4148dcac3bb2056fc414252f7) )
	ROM_LOAD16_BYTE( "11310.epr",   0x040000, 0x20000, CRC(5d9fa02c) SHA1(0ca71e35cf9740e38a52960f7d1ef96e7e1dda94) )
	ROM_LOAD16_BYTE( "11311.epr",   0x040001, 0x20000, CRC(483de21b) SHA1(871f0e856dcc81dcef1d9846261b3c011fa26dde) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "11314.epr", 0x00000, 0x10000, CRC(d4f954a9) SHA1(93ee8cf8fcf4e1d0dd58329bba9b594431193449) )
	ROM_LOAD( "11315.epr", 0x10000, 0x10000, CRC(35813088) SHA1(ea1ec982d1509efb26e7b6a150825a6a905efed9) )
	ROM_LOAD( "11316.epr", 0x20000, 0x10000, CRC(84290dff) SHA1(c13fb6ef12a991f79a95072f953a02b5c992aa2d) )

	ROM_REGION32_LE( 0x200000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD32_BYTE( "11323.epr",    0x000000, 0x20000, CRC(27e40735) SHA1(284ddb88efe741fb78199ea619c9b230ee689803) )
	ROM_LOAD32_BYTE( "11322.epr",    0x000001, 0x20000, CRC(10364d74) SHA1(393b19a972b5d8817ffd438f13ded73cd58ebe56) )
	ROM_LOAD32_BYTE( "11321.epr",    0x000002, 0x20000, CRC(8e738f58) SHA1(9f2dceebf01e582cf60f072ae411000d8503894b) )
	ROM_LOAD32_BYTE( "11320.epr",    0x000003, 0x20000, CRC(a95c76b8) SHA1(cda62f3c25b9414a523c2fc5d109031ed560069e) )
	ROM_LOAD32_BYTE( "11327.epr",    0x080000, 0x20000, CRC(deae90f1) SHA1(c73c23bab949041242302cec13d653dcc71bb944) )
	ROM_LOAD32_BYTE( "11326.epr",    0x080001, 0x20000, CRC(29198403) SHA1(3ecf315a0e6b3ed5005f8bdcb2e2a884c8b176c7) )
	ROM_LOAD32_BYTE( "11325.epr",    0x080002, 0x20000, CRC(b9e98ae9) SHA1(c4932e2590b10d54fa8ded94593dc4203fccc60d) )
	ROM_LOAD32_BYTE( "11324.epr",    0x080003, 0x20000, CRC(9742b552) SHA1(922032264d469e943dfbcaaf57464efc638fcf73) )
	ROM_LOAD32_BYTE( "11331.epr",    0x100000, 0x20000, CRC(3a2c042e) SHA1(c296ff222d156d3bdcb42bef321831f502830fd6) )
	ROM_LOAD32_BYTE( "11330.epr",    0x100001, 0x20000, CRC(aa7c70c5) SHA1(b6fea17392b7821b8b3bba78002f9c1604f09edc) )
	ROM_LOAD32_BYTE( "11329.epr",    0x100002, 0x20000, CRC(31b20257) SHA1(7ce10a94bce67b2d15d7b576b0f7d47389dc8948) )
	ROM_LOAD32_BYTE( "11328.epr",    0x100003, 0x20000, CRC(da39e89c) SHA1(526549ce9112754c82743552eeebec63fe7ad968) )
	ROM_LOAD32_BYTE( "epr11395.93",  0x180000, 0x20000, CRC(90775579) SHA1(15a86071a105da40ec9c0c0074e342231fc030d0) ) //
	ROM_LOAD32_BYTE( "epr11394.97",  0x180001, 0x20000, CRC(5f2783be) SHA1(424510153a91902901f321f39738a862d6fba8e7) ) // different numbers?
	ROM_LOAD32_BYTE( "epr11393.101", 0x180002, 0x20000, CRC(525e2e1d) SHA1(6fd09f775e7e6cad8078513d1af0a8ff40fb1360) ) // replaced from original rev?
	ROM_LOAD32_BYTE( "epr11392.105", 0x180003, 0x20000, CRC(b4a382f7) SHA1(c03a05ba521f654db1a9c5f5717b7a15e5a29d4e) ) //

	ROM_REGION( 0x10000, REGION_GFX3, 0 ) /* ???? */
	ROM_LOAD( "11313.epr",	 0x00000, 0x10000, CRC(6a56c4c3) SHA1(c1b8023cb2ba4e96be052031c24b6ae424225c71) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 ) /* sound CPU */
	ROM_LOAD( "epr11396.17",	 0x00000, 0x10000, CRC(d37b54a4) SHA1(c230fe7241a1f13ca13506d1492f348f506c40a7) )

	ROM_REGION( 0x60000, REGION_SOUND1, 0 ) /* Sega PCM sound data */
	ROM_LOAD( "11317.epr",   0x00000, 0x20000, CRC(d4e7ac1f) SHA1(ec5d6e4949938adf56e5613801ae56ff2c3dede5) )
	ROM_LOAD( "11318.epr",   0x20000, 0x20000, CRC(70d3f02c) SHA1(391aac2bc5673e06150de27e19c7c6359da8ca82) )
	ROM_LOAD( "11319.epr",   0x40000, 0x20000, CRC(50d9242e) SHA1(a106371bf680c3088ec61f07fc5c4ce467973c15) )
ROM_END

/**************************************************************************************************************************
	Thunder Blade (Japan), Sega X-board
	CPU: MC68000
*/
ROM_START( thndrbdj )
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "11304.epr", 0x00000, 0x20000, CRC(a90630ef) SHA1(8f29e020119b2243b1c95e15546af1773327ae85) ) // patched
	ROM_LOAD16_BYTE( "11306.epr", 0x40000, 0x20000, CRC(4b95f2b4) SHA1(9e0ff898a2af05c35db3551e52c7485748698c28) ) // patched
	ROM_LOAD16_BYTE( "11305.epr", 0x00001, 0x20000, CRC(9ba3ef61) SHA1(f75748b37ce35b0ef881804f73417643068dfbb2) )
	ROM_LOAD16_BYTE( "11307.epr", 0x40001, 0x20000, CRC(2d6833e4) SHA1(b39a744370014237121f0010d18897e63f7058cf) )

	ROM_REGION( 0x80000, REGION_CPU2, 0 ) /* 2nd 68000 code */
	ROM_LOAD16_BYTE( "11308.epr", 0x00000, 0x20000, CRC(7956c238) SHA1(4608225cfd6ea3d38317cbe970f26a5fc2f8e320) )
	ROM_LOAD16_BYTE( "11310.epr", 0x40000, 0x20000, CRC(5d9fa02c) SHA1(0ca71e35cf9740e38a52960f7d1ef96e7e1dda94) )
	ROM_LOAD16_BYTE( "11309.epr", 0x00001, 0x20000, CRC(c887f620) SHA1(644c47cc2cf75cbe489ea084c13c59d94631e83f) )
	ROM_LOAD16_BYTE( "11311.epr", 0x40001, 0x20000, CRC(483de21b) SHA1(871f0e856dcc81dcef1d9846261b3c011fa26dde) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "11314.epr", 0x00000, 0x10000, CRC(d4f954a9) SHA1(93ee8cf8fcf4e1d0dd58329bba9b594431193449) )
	ROM_LOAD( "11315.epr", 0x10000, 0x10000, CRC(35813088) SHA1(ea1ec982d1509efb26e7b6a150825a6a905efed9) )
	ROM_LOAD( "11316.epr", 0x20000, 0x10000, CRC(84290dff) SHA1(c13fb6ef12a991f79a95072f953a02b5c992aa2d) )

	ROM_REGION32_LE( 0x200000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD32_BYTE( "11323.epr", 0x000000, 0x20000, CRC(27e40735) SHA1(284ddb88efe741fb78199ea619c9b230ee689803) )
	ROM_LOAD32_BYTE( "11322.epr", 0x000001, 0x20000, CRC(10364d74) SHA1(393b19a972b5d8817ffd438f13ded73cd58ebe56) )
	ROM_LOAD32_BYTE( "11321.epr", 0x000002, 0x20000, CRC(8e738f58) SHA1(9f2dceebf01e582cf60f072ae411000d8503894b) )
	ROM_LOAD32_BYTE( "11320.epr", 0x000003, 0x20000, CRC(a95c76b8) SHA1(cda62f3c25b9414a523c2fc5d109031ed560069e) )
	ROM_LOAD32_BYTE( "11327.epr", 0x080000, 0x20000, CRC(deae90f1) SHA1(c73c23bab949041242302cec13d653dcc71bb944) )
	ROM_LOAD32_BYTE( "11326.epr", 0x080001, 0x20000, CRC(29198403) SHA1(3ecf315a0e6b3ed5005f8bdcb2e2a884c8b176c7) )
	ROM_LOAD32_BYTE( "11325.epr", 0x080002, 0x20000, CRC(b9e98ae9) SHA1(c4932e2590b10d54fa8ded94593dc4203fccc60d) )
	ROM_LOAD32_BYTE( "11324.epr", 0x080003, 0x20000, CRC(9742b552) SHA1(922032264d469e943dfbcaaf57464efc638fcf73) )
	ROM_LOAD32_BYTE( "11331.epr", 0x100000, 0x20000, CRC(3a2c042e) SHA1(c296ff222d156d3bdcb42bef321831f502830fd6) )
	ROM_LOAD32_BYTE( "11330.epr", 0x100001, 0x20000, CRC(aa7c70c5) SHA1(b6fea17392b7821b8b3bba78002f9c1604f09edc) )
	ROM_LOAD32_BYTE( "11329.epr", 0x100002, 0x20000, CRC(31b20257) SHA1(7ce10a94bce67b2d15d7b576b0f7d47389dc8948) )
	ROM_LOAD32_BYTE( "11328.epr", 0x100003, 0x20000, CRC(da39e89c) SHA1(526549ce9112754c82743552eeebec63fe7ad968) )
	ROM_LOAD32_BYTE( "11335.epr", 0x180000, 0x20000, CRC(f19b3e86) SHA1(40e8ba10cd5020782b82279974d13330a9c015e5) )
	ROM_LOAD32_BYTE( "11334.epr", 0x180001, 0x20000, CRC(348f91c7) SHA1(03da6a4fee1fdea76058be4bc5ffcde7a79e5948) )
	ROM_LOAD32_BYTE( "11333.epr", 0x180002, 0x20000, CRC(05a2333f) SHA1(70f213945fa7fe056fe17a02558638e87f2c001e) )
	ROM_LOAD32_BYTE( "11332.epr", 0x180003, 0x20000, CRC(dc089ec6) SHA1(d72390c45138a507e79af112addbc015560fc248) )

	ROM_REGION( 0x10000, REGION_GFX3, 0 ) /* ground data */
	ROM_LOAD( "11313.epr",	 0x00000, 0x10000, CRC(6a56c4c3) SHA1(c1b8023cb2ba4e96be052031c24b6ae424225c71) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 ) /* sound CPU */
	ROM_LOAD( "11312.epr",   0x00000, 0x10000, CRC(3b974ed2) SHA1(cf18a2d0f01643c747a884bf00e5b7037ba2e64a) )

	ROM_REGION( 0x60000, REGION_SOUND1, 0 ) /* Sega PCM sound data */
	ROM_LOAD( "11317.epr",   0x00000, 0x20000, CRC(d4e7ac1f) SHA1(ec5d6e4949938adf56e5613801ae56ff2c3dede5) )
	ROM_LOAD( "11318.epr",   0x20000, 0x20000, CRC(70d3f02c) SHA1(391aac2bc5673e06150de27e19c7c6359da8ca82) )
	ROM_LOAD( "11319.epr",   0x40000, 0x20000, CRC(50d9242e) SHA1(a106371bf680c3088ec61f07fc5c4ce467973c15) )
ROM_END


/*************************************
 *
 *	Generic driver initialization
 *
 *************************************/

static DRIVER_INIT( generic_xboard )
{
	xboard_generic_init();
}

/*************************************
 *
 *	Game driver(s)
 *
 *************************************/

GAME( 1990, abcop,     0,        xboard,  abcop,    generic_xboard, ROT0, "Sega", "A.B. Cop" ) // decrypted
GAME( 1989, rachero,   0,        xboard,  rachero,  generic_xboard, ROT0, "Sega", "Racing Hero" ) // decrypted
GAME( 1989, smgp,      0,        xboard,  smgp,     generic_xboard, ROT0, "Sega", "Super Monaco GP (set 7, World, Rev B, 'Twin')" ) // decrypted
GAME( 1987, thndrbld,  0,        xboard,  thndrbld, generic_xboard, ROT0, "Sega", "Thunder Blade" ) // decrypted
GAME( 1987, thndrbdj,  thndrbld, xboard,  thndrbld, generic_xboard, ROT0, "Sega", "Thunder Blade (Japan)" ) // not encrypted
/*gamezfan backports segaxbd driver and segaxbd video from MAME90 all games in this driver are now playable
smgp has some GFX glitches which would be fixed if using the MAME91 Sega Gfx but that makes the games take a
major speed hit on the Xbox*/
#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()