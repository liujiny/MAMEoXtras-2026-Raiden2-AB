#pragma code_seg("C906")
#pragma data_seg("D906")
#pragma bss_seg("B906")
#pragma const_seg("K906")
#pragma comment(linker, "/merge:D906=906")
#pragma comment(linker, "/merge:C906=906")
#pragma comment(linker, "/merge:B906=906")
#pragma comment(linker, "/merge:K906=906")

#include "driver.h"
#include "system16.h"
#include "machine/segaic16.h"

extern void mc8123_decrypt_0066(void);


/*************************************
 *
 *	Debugging
 *
 *************************************/

#define PRINT_MAPPINGS			(0)



/*************************************
 *
 *	Constants
 *
 *************************************/
#define ROM_BOARD_171_5358_SMALL (0)	/* 171-5358 with smaller ROMs */
#define ROM_BOARD_171_5358		(1)		/* 171-5358 */
#define ROM_BOARD_171_5521		(2)		/* 171-5521 */
#define ROM_BOARD_171_5704		(2)		/* 171-5704 - don't know any diff between this and 171-5521 */
#define ROM_BOARD_171_5797		(3)		/* 171-5797 */
#define ROM_BOARD_171_5704_EXTRA_RAM	(4)	/* System 16C Fantasy Zone II*/


/*************************************
 *
 *	Types
 *
 *************************************/

struct region_info
{
	UINT8			regbase;
	offs_t			regoffs;
	offs_t			length;
	offs_t			mirror;
	offs_t			romoffset;
	read16_handler	read;
	write16_handler	write;
	data16_t **		base;
	const char *	name;
};



/*************************************
 *
 *	Statics
 *
 *************************************/

static data16_t *workram;

static UINT8 rom_board;
static UINT8 memory_control[0x20];
static UINT8 mcu_data;

static UINT8 has_sound_cpu;

static read16_handler custom_io_r;
static write16_handler custom_io_w;

static UINT8 disable_screen_blanking;

static void (*i8751_vblank_hook)(void);



/*************************************
 *
 *	Prototypes
 *
 *************************************/

extern void fd1094_machine_init(void);
extern void fd1094_driver_init(void);

static void update_memory_mapping(void);

static READ16_HANDLER( unmapped_r );
static WRITE16_HANDLER( unmapped_w );
static READ16_HANDLER( misc_io_r );
static WRITE16_HANDLER( misc_io_w );
static WRITE16_HANDLER( rom_5704_bank_w );
static READ16_HANDLER( rom_5797_bank_math_r );
static WRITE16_HANDLER( rom_5797_bank_math_w );
static READ16_HANDLER( unknown_rgn2_r );
static WRITE16_HANDLER( unknown_rgn2_w );



/*************************************
 *
 *	Memory mapping tables
 *
 *************************************/
 
static struct region_info rom_171_5358_info_small[] =
{
	{ 0x3d/2, 0x00000, 0x04000, 0xffc000,      ~0, misc_io_r,             misc_io_w,             NULL,                  "I/O space" },
	{ 0x39/2, 0x00000, 0x01000, 0xfff000,      ~0, paletteram16_word_r,   segaic16_paletteram_w,&paletteram16,         "color RAM" },
	{ 0x35/2, 0x00000, 0x10000, 0xfe0000,      ~0, MRA16_BANK11,          segaic16_tileram_w,   &segaic16_tileram,     "tile RAM" },
	{ 0x35/2, 0x10000, 0x01000, 0xfef000,      ~0, MRA16_BANK12,          system16b_textram_w,  &segaic16_textram,     "text RAM" },
	{ 0x31/2, 0x00000, 0x00800, 0xfff800,      ~0, MRA16_BANK13,          MWA16_BANK13,         &segaic16_spriteram,   "object RAM" },
	{ 0x2d/2, 0x00000, 0x04000, 0xffc000,      ~0, MRA16_BANK14,          MWA16_BANK14,         &workram,              "work RAM" },
	{ 0x29/2, 0x00000, 0x20000, 0xfe0000, 0x20000, MRA16_BANK15,          MWA16_ROM,             NULL,                  "ROM 2" },
	{ 0x25/2, 0x00000, 0x20000, 0xfe0000, 0x10000, MRA16_BANK16,          MWA16_ROM,             NULL,                  "ROM 1" },
	{ 0x21/2, 0x00000, 0x20000, 0xfe0000, 0x00000, MRA16_BANK17,          MWA16_ROM,             NULL,                  "ROM 0" },
	{ 0 }
};


static struct region_info rom_171_5358_info[] =
{
	{ 0x3d/2, 0x00000, 0x04000, 0xffc000,      ~0, misc_io_r,             misc_io_w,             NULL,                "I/O space" },
	{ 0x39/2, 0x00000, 0x01000, 0xfff000,      ~0, paletteram16_word_r,   segaic16_paletteram_w,&paletteram16,        "color RAM" },
	{ 0x35/2, 0x00000, 0x10000, 0xfe0000,      ~0, MRA16_BANK10,          segaic16_tileram_w,    &segaic16_tileram,   "tile RAM" },
	{ 0x35/2, 0x10000, 0x01000, 0xfef000,      ~0, MRA16_BANK11,          system16b_textram_w,   &segaic16_textram,   "text RAM" },
	{ 0x31/2, 0x00000, 0x00800, 0xfff800,      ~0, MRA16_BANK12,          MWA16_BANK12,          &segaic16_spriteram, "object RAM" },
	{ 0x2d/2, 0x00000, 0x04000, 0xffc000,      ~0, MRA16_BANK13,          MWA16_BANK13,          &workram,            "work RAM" },
	{ 0x29/2, 0x00000, 0x20000, 0xfe0000, 0x40000, MRA16_BANK14,          MWA16_ROM,             NULL,                "ROM 2" },
	{ 0x25/2, 0x00000, 0x20000, 0xfe0000, 0x20000, MRA16_BANK15,          MWA16_ROM,             NULL,                "ROM 1" },
	{ 0x21/2, 0x00000, 0x20000, 0xfe0000, 0x00000, MRA16_BANK16,          MWA16_ROM,             NULL,                "ROM 0" },
	{ 0 }
};

static struct region_info rom_171_5704_info[] =
{
	{ 0x3d/2, 0x00000, 0x04000, 0xffc000,      ~0, misc_io_r,             misc_io_w,             NULL,                "I/O space" },
	{ 0x39/2, 0x00000, 0x01000, 0xfff000,      ~0, paletteram16_word_r,   segaic16_paletteram_w, &paletteram16,       "color RAM" },
	{ 0x35/2, 0x00000, 0x10000, 0xfe0000,      ~0, MRA16_BANK10,          segaic16_tileram_w,    &segaic16_tileram,   "tile RAM" },
	{ 0x35/2, 0x10000, 0x01000, 0xfef000,      ~0, MRA16_BANK11,          system16b_textram_w,   &segaic16_textram,   "text RAM" },
	{ 0x31/2, 0x00000, 0x00800, 0xfff800,      ~0, MRA16_BANK12,          MWA16_BANK12,          &segaic16_spriteram, "object RAM" },
	{ 0x2d/2, 0x00000, 0x04000, 0xffc000,      ~0, MRA16_BANK13,          MWA16_BANK13,          &workram,            "work RAM" },
	{ 0x29/2, 0x00000, 0x10000, 0xff0000,      ~0, NULL,                  rom_5704_bank_w,       NULL,                "tile bank" },
	{ 0x25/2, 0x00000, 0x80000, 0xfc0000, 0x80000, MRA16_BANK15,          MWA16_ROM,             NULL,                "ROM 1" },
	{ 0x21/2, 0x00000, 0x80000, 0xfc0000, 0x00000, MRA16_BANK16,          MWA16_ROM,             NULL,                "ROM 0" },
	{ 0 }
};

static struct region_info rom_171_5797_info[] =
{
	{ 0x3d/2, 0x00000, 0x04000, 0xffc000,      ~0, misc_io_r,             misc_io_w,             NULL,                "I/O space" },
	{ 0x39/2, 0x00000, 0x01000, 0xfff000,      ~0, paletteram16_word_r,   segaic16_paletteram_w, &paletteram16,       "color RAM" },
	{ 0x35/2, 0x00000, 0x10000, 0xfe0000,      ~0, MRA16_BANK10,          segaic16_tileram_w,    &segaic16_tileram,   "tile RAM" },
	{ 0x35/2, 0x10000, 0x01000, 0xfef000,      ~0, MRA16_BANK11,          system16b_textram_w,   &segaic16_textram,   "text RAM" },
	{ 0x31/2, 0x00000, 0x00800, 0xfff800,      ~0, MRA16_BANK12,          MWA16_BANK12,          &segaic16_spriteram, "object RAM" },
	{ 0x2d/2, 0x00000, 0x04000, 0xffc000,      ~0, MRA16_BANK13,          MWA16_BANK13,          &workram,            "work RAM" },
	{ 0x29/2, 0x00000, 0x10000, 0xff0000,      ~0, unknown_rgn2_r,        unknown_rgn2_w,        NULL,                "???" },
	{ 0x25/2, 0x00000, 0x04000, 0xffc000,      ~0, rom_5797_bank_math_r,  rom_5797_bank_math_w,  NULL,                "tile bank/math" },
	{ 0x21/2, 0x00000, 0x80000, 0xf80000, 0x00000, MRA16_BANK16,          MWA16_ROM,             NULL,                "ROM 0" },
	{ 0 }
};

static struct region_info  rom_171_5704_extra_ram_info[] =
{
	{ 0x3d/2, 0x00000, 0x04000, 0xffc000,      ~0, misc_io_r,             misc_io_w,             NULL,                "I/O space" },
	{ 0x39/2, 0x00000, 0x01000, 0xfff000,      ~0, paletteram16_word_r,   segaic16_paletteram_w, &paletteram16,       "color RAM" },
	{ 0x35/2, 0x00000, 0x10000, 0xfe0000,      ~0, MRA16_BANK11,          segaic16_tileram_w,    &segaic16_tileram,   "tile RAM" },
	{ 0x35/2, 0x10000, 0x01000, 0xfef000,      ~0, MRA16_BANK12,          system16b_textram_w,   &segaic16_textram,   "text RAM" },
	{ 0x31/2, 0x00000, 0x00800, 0xfff800,      ~0, MRA16_BANK13,          MWA16_BANK13,          &segaic16_spriteram, "object RAM" },
	{ 0x2d/2, 0x00000, 0x40000, 0xfc0000,      ~0, MRA16_BANK14,          MWA16_BANK14,          &workram,            "work RAM" },
	{ 0x29/2, 0x00000, 0x10000, 0xff0000,      ~0, NULL,                  rom_5704_bank_w,       NULL,                "tile bank" },
	{ 0x25/2, 0x00000, 0x80000, 0xfc0000, 0x80000, MRA16_BANK16,          MWA16_ROM,             NULL,                "ROM 1" },
	{ 0x21/2, 0x00000, 0x80000, 0xfc0000, 0x00000, MRA16_BANK17,          MWA16_ROM,             NULL,                "ROM 0" },
	{ 0 }
};


static struct region_info *region_info_list[] =
{
    &rom_171_5358_info_small[0],
	&rom_171_5358_info[0],
	&rom_171_5704_info[0],
	&rom_171_5797_info[0],
	&rom_171_5704_extra_ram_info[0]
};



/*************************************
 *
 *	Configuration
 *
 *************************************/



static void system16b_generic_init(int _rom_board)
{
	int rgnum;

	
	/* set the ROM board */
	rom_board = _rom_board;

	/* loop over the regions and allocate memory */
	for (rgnum = 0; region_info_list[rom_board][rgnum].regbase != 0; rgnum++)
	{
		const struct region_info *rgn = &region_info_list[rom_board][rgnum];
		if (rgn->base)
			*rgn->base = auto_malloc(rgn->length);
	}

	/* init the FD1094 */
	fd1094_driver_init();

	/* reset the custom handlers and other pointers */
	custom_io_r = NULL;
	custom_io_w = NULL;
	i8751_vblank_hook = NULL;
	disable_screen_blanking = 0;

	/* see if we have a sound CPU and a UPD7759 chip */
	has_sound_cpu = (mame_find_cpu_index("sound") != -1);
}



/*************************************
 *
 *	Initialization & interrupts
 *
 *************************************/

static MACHINE_INIT( system16b )
{
	fd1094_machine_init();
	system16b_reset_video();
	
	/* if we have a fake i8751 handler, disable the actual 8751 */
	if (i8751_vblank_hook != NULL)
		cpunum_suspend(2, SUSPEND_REASON_DISABLE, 1);
}


/*************************************
 *
 *	Dynamic memory maps
 *
 *************************************/

static void memory_control_w(offs_t offset, data8_t data)
{
	UINT8 oldval;

	/* wraps every 32 words */
	offset &= 0x1f;

	/* remember the previous value and swap in the new one */
	oldval = memory_control[offset];
	memory_control[offset] = data;

	/* switch off the offset */
	switch (offset)
	{
		case 0x02:
			/* misc commands */
			/*   00 - resume execution after 03 */
			/*   03 - maybe controls halt and reset lines together? */
			if ((oldval ^ memory_control[offset]) & 3)
			{
				cpunum_set_input_line(0, INPUT_LINE_RESET, (memory_control[offset] & 3) == 3 ? ASSERT_LINE : CLEAR_LINE);
				if ((memory_control[offset] & 3) == 3)
					fd1094_machine_init();
			}
			break;

		case 0x03:
			if (has_sound_cpu)
			{
				soundlatch_w(0, data & 0xff);
				cpunum_set_input_line(1, 0, HOLD_LINE);
			}
			break;

		case 0x04:
			/* controls IRQ lines to 68000, negative logic -- write $B to signal IRQ4 */
			if ((memory_control[offset] & 7) != 7)
				cpunum_set_input_line(0, (~memory_control[offset] & 7), HOLD_LINE);
			break;

		case 0x05:
			/* read/write control */
			/*   01 - write data latched in 00,01 to 2 * (address in 0A,0B,0C) */
			/*   02 - read data into latches 00,01 from 2 * (address in 07,08,09) */
			if (data == 0x01)
			{
				offs_t addr = (memory_control[0x0a] << 17) | (memory_control[0x0b] << 9) | (memory_control[0x0c] << 1);
				cpuintrf_push_context(0);
				program_write_word_16be(addr, (memory_control[0x00] << 8) | memory_control[0x01]);
				cpuintrf_pop_context();
			}
			else if (data == 0x02)
			{
				offs_t addr = (memory_control[0x07] << 17) | (memory_control[0x08] << 9) | (memory_control[0x09] << 1);
				data16_t result;
				cpuintrf_push_context(0);
				result = program_read_word_16be(addr);
				cpuintrf_pop_context();
				memory_control[0x00] = result >> 8;
				memory_control[0x01] = result;
			}
			break;
		
		case 0x07:	case 0x08:	case 0x09:
			/* writes here latch a 68000 address for writing */
			break;

		case 0x0a:	case 0x0b:	case 0x0c:
			/* writes here latch a 68000 address for reading */
			break;

		case 0x10:	case 0x11:
		case 0x12:	case 0x13:
		case 0x14:	case 0x15:
		case 0x16:	case 0x17:
		case 0x18:	case 0x19:
		case 0x1a:	case 0x1b:
		case 0x1c:	case 0x1d:
		case 0x1e:	case 0x1f:
			if (oldval != data)
				update_memory_mapping();
			break;

		default:
			logerror("Unknown memory_control_w to address %02X = %02X\n", offset, data);
			break;
	}
}


static data8_t memory_control_r(offs_t offset, data8_t unmap_value)
{
	/* wraps every 32 words */
	offset &= 0x1f;

	/* switch off the offset */
	switch (offset)
	{
		case 0x00:
		case 0x01:
			/* data latches - return the values latched */
			return memory_control[offset];

		case 0x02:
			/* various input bits from the 68000 */
			/*   01 - ???? */
			/*   02 - ???? */
			/*   04 - ???? */
			/*   08 - ???? */
			/*   40 - set if busy processing a read/write request */
			/* Together, 01+02 == 00 if the 68000 is halted */
			/* Together, 01+02+04+08 == 0F if the 68000 is executing */
			return (memory_control[0x02] & 3) == 3 ? 0x00 : 0x0f;

		case 0x03:
			/* this returns data that the sound CPU writes */
			return mcu_data;

		default:
			logerror("Unknown memory_control_r from address %02X\n", offset);
			break;
	}
	return unmap_value;
}


static void update_memory_mapping(void)
{
	static const offs_t region_size_map[4] = { 0x00ffff, 0x01ffff, 0x07ffff, 0x1fffff };
	int rgnum;

	if (PRINT_MAPPINGS) printf("----\nRemapping:\n");

	/* first reset everything back to the beginning */
	memory_install_read16_handler (0, ADDRESS_SPACE_PROGRAM, 0x000000, 0x00ffff, 0, 0, MRA16_ROM);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x000000, 0x00ffff, 0, 0, (write16_handler)STATIC_UNMAP);
	memory_install_read16_handler (0, ADDRESS_SPACE_PROGRAM, 0x010000, 0xffffff, 0, 0, unmapped_r);
	memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, 0x010000, 0xffffff, 0, 0, unmapped_w);

	/* loop over the regions */
	for (rgnum = 0; region_info_list[rom_board][rgnum].regbase != 0; rgnum++)
	{
		const struct region_info *rgn = &region_info_list[rom_board][rgnum];
		offs_t region_size = region_size_map[memory_control[rgn->regbase] & 3];
		offs_t region_base = (memory_control[rgn->regbase + 1] << 16) & ~region_size;

		/* only map if the base is non-zero, or if it's the base of the first ROM */
		if (region_base != 0 || rgn->romoffset == 0)
		{
			offs_t region_mirror = rgn->mirror & region_size;
			offs_t region_start = region_base + (rgn->regoffs & region_size);
			offs_t region_end = region_start + ((rgn->length - 1 < region_size) ? rgn->length - 1 : region_size);
			write16_handler write = rgn->write;
			read16_handler read = rgn->read;
			int banknum = 0;

			/* check for mapping to banks */
			if ((FPTR)read >= STATIC_BANK1 && (FPTR)read <= STATIC_BANKMAX)
				banknum = ((FPTR)read - STATIC_BANK1) + 1;
			if ((FPTR)write >= STATIC_BANK1 && (FPTR)write <= STATIC_BANKMAX)
				banknum = ((FPTR)write - STATIC_BANK1) + 1;

			/* ROM areas need extra clamping */
			if (rgn->romoffset != ~0)
			{
				offs_t romsize = memory_region_length(REGION_CPU1);
				if (region_start >= romsize)
					read = NULL;
				else if (region_start + rgn->length > romsize)
					region_end = romsize - 1;
			}

			/* map it */
			if (read)
				memory_install_read16_handler(0, ADDRESS_SPACE_PROGRAM, region_start, region_end, 0, region_mirror, read);
			if (write)
				memory_install_write16_handler(0, ADDRESS_SPACE_PROGRAM, region_start, region_end, 0, region_mirror, write);

			/* set the bank pointer */
			if (banknum && read)
			{
				if (rgn->base)
					cpu_setbank(banknum, *rgn->base);
				else if (rgn->romoffset != ~0)
					cpu_setbank(banknum, memory_region(REGION_CPU1) + region_start);
			}

			if (PRINT_MAPPINGS) printf("  %06X-%06X (%06X) = %s\n", region_start, region_end, region_mirror, rgn->name);
		}
	}
}



/*************************************
 *
 *	Dynamic memory map accesors
 *
 *************************************/

static data16_t unmapped_memory_r(void)
{
	static int recurse = 0;
	data16_t result;

	/* Unmapped memory returns the last word on the data bus, which is almost always the opcode */
	/* of the next instruction due to prefetch; however, since we may be encrypted, we actually */
	/* need to return the encrypted opcode, not the last decrypted data. */

	/* Believe it or not, this is actually important for Cotton, which has the following evil */
	/* code: btst #0,$7038f7, which tests the low bit of an unmapped address, which thus should */
	/* return the prefetched value. */

	/* prevent recursion */
	if (recurse)
		return 0xffff;

	/* read original encrypted memory at that address */
	recurse = 1;
	result = program_read_word_16be(activecpu_get_pc());
	recurse = 0;
	return result;
}


static READ16_HANDLER( unmapped_r )
{
	return memory_control_r(offset, unmapped_memory_r());
}


static WRITE16_HANDLER( unmapped_w )
{
	if (ACCESSING_LSB)
		memory_control_w(offset, data & 0xff);
}


static READ8_HANDLER( mcu_memory_map_r )
{
	return memory_control_r(offset, 0xff);
}


static WRITE8_HANDLER( mcu_memory_map_w )
{
	memory_control_w(offset, data);
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
		case 0x1000/2:
			return readinputport(offset & 3);

		case 0x2000/2:
			return readinputport(4 + (offset & 1));
	}
	if (custom_io_r)
		return custom_io_r(offset, mem_mask);
	logerror("%06X:misc_io_r - unknown read access to address %04X\n", activecpu_get_pc(), offset * 2);
	return unmapped_memory_r();
}


static WRITE16_HANDLER( misc_io_w )
{
	offset &= 0x1fff;
	switch (offset & (0x3000/2))
	{
		case 0x0000/2:
			/*
				D7 : ?
				D6 : 1= Screen flip, 0= Normal screen display
				D5 : 1= Display on, 0= Display off
				D4 : ?
				D3 : Output to lamp 2 (1= On, 0= Off)
				D2 : Output to lamp 1 (1= On, 0= Off)
				D1 : (Output to coin counter 2?)
				D0 : Output to coin counter 1
			*/
/*
	all sets D7, clears D4, except:
	ddux clears D7
	eswat sets D4
	sdi clears D7
	wb3b clears D7
*/
			system16b_set_screen_flip(data & 0x40);
			if (!disable_screen_blanking)
				system16b_set_draw_enable(data & 0x20);
			set_led_status(1, data & 0x08);
			set_led_status(0, data & 0x04);
			coin_counter_w(1, data & 0x02);
			coin_counter_w(0, data & 0x01);
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
 *	Tile banking/math chips
 *
 *************************************/

static WRITE16_HANDLER( rom_5704_bank_w )
{
	if (ACCESSING_LSB)
		system16b_set_tile_bank(offset & 1, data & 7);
}


static READ16_HANDLER( rom_5797_bank_math_r )
{
	offset &= 0x1fff;
	switch (offset & (0x3000/2))
	{
		case 0x0000/2:
			/* multiply registers */
			return segaic16_multiply_0_r(offset & 3, mem_mask);

		case 0x1000/2:
			/* compare registers */
			return segaic16_compare_timer_0_r(offset & 7, mem_mask);
	}
	return unmapped_memory_r();
}


static WRITE16_HANDLER( rom_5797_bank_math_w )
{
	offset &= 0x1fff;
	switch (offset & (0x3000/2))
	{
		case 0x0000/2:
			/* multiply registers */
			segaic16_multiply_0_w(offset & 3, data, mem_mask);
			break;

		case 0x1000/2:
			/* compare registers */
			segaic16_compare_timer_0_w(offset & 7, data, mem_mask);
			break;

		case 0x2000/2:
			if (ACCESSING_LSB)
				system16b_set_tile_bank(offset & 1, data & 7);
			break;
	}
}


static READ16_HANDLER( unknown_rgn2_r )
{
	if (PRINT_MAPPINGS) printf("Region 2: read from %04X\n", offset * 2);
	return segaic16_compare_timer_1_r(offset & 7, mem_mask);
}


static WRITE16_HANDLER( unknown_rgn2_w )
{
	if (PRINT_MAPPINGS) printf("Region 2: write to %04X = %04X & %04X\n", offset * 2, data, mem_mask ^ 0xffff);
	segaic16_compare_timer_1_w(offset & 7, data, mem_mask);
}



/*************************************
 *
 *	Sound interaction
 *
 *************************************/

static WRITE8_HANDLER( upd7759b_control_w )
{
	int size = memory_region_length(REGION_CPU2) - 0x10000;
	if (size > 0)
	{
		int bankoffs = 0;
		
		/* it is important to write in this order: if the /START line goes low
		   at the same time /RESET goes low, no sample should be started */
		upd7759b_start_w(0, data & 0x80);
		upd7759b_reset_w(0, data & 0x40);
		
		/* banking depends on the ROM board */
		switch (rom_board)
		{
			case ROM_BOARD_171_5358:
			case ROM_BOARD_171_5358_SMALL:
				/*
					D5 : /CS for ROM at A11
					D4 : /CS for ROM at A10
					D3 : /CS for ROM at A9
					D2 : /CS for ROM at A8
					D1 : A15 for all ROMs (Or ignored for 27256's)
					D0 : A14 for all ROMs
				*/
				if (!(data & 0x04)) bankoffs = 0x00000;
				if (!(data & 0x08)) bankoffs = 0x10000;
				if (!(data & 0x10)) bankoffs = 0x20000;
				if (!(data & 0x20)) bankoffs = 0x30000;
				bankoffs += (data & 0x03) * 0x4000;
				break;
			
			case ROM_BOARD_171_5521:
			case ROM_BOARD_171_5704_EXTRA_RAM:
				/*
					D5 : Unused
					D4 : Unused
					D3 : ROM select 0=A11, 1=A12
					D2 : A16 for all ROMs
					D1 : A15 for all ROMs
					D0 : A14 for all ROMs
				*/
				bankoffs = ((data & 0x08) >> 3) * 0x20000;
				bankoffs += (data & 0x07) * 0x4000;
				break;
			
			case ROM_BOARD_171_5797:
				/*
					D5 : Unused
					D4 : A17 for all ROMs
					D3 : ROM select 0=A11, 1=A12
					D2 : A16 for all ROMs
					D1 : A15 for all ROMs
					D0 : A14 for all ROMs
				*/
				bankoffs = ((data & 0x08) >> 3) * 0x40000;
				bankoffs += ((data & 0x10) >> 4) * 0x20000;
				bankoffs += (data & 0x07) * 0x04000;
				break;
		}
		cpu_setbank(1, memory_region(REGION_CPU2) + 0x10000 + (bankoffs % size));
	}
}


static READ8_HANDLER( upd7759b_status_r )
{
	return upd7759b_busy_r(0) << 7;
}


static void upd7759b_generate_nmi(int state)
{
	if (state)
		cpunum_set_input_line(1, INPUT_LINE_NMI, PULSE_LINE);
}


static WRITE8_HANDLER( mcu_data_w )
{
	mcu_data = data;
	cpunum_set_input_line(2, 1, PULSE_LINE);
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
 *	Per-game I8751 workarounds
 *
 *************************************/

static void tturf_i8751_sim(void)
{
	data16_t temp;

	/* signal a VBLANK to the main CPU */
	cpunum_set_input_line(0, 4, HOLD_LINE);
	
	/* process any new sound data */
	temp = workram[0x01d0/2];
	if ((temp & 0xff00) != 0x0000)
	{
		memory_control_w(0x03, temp);
		workram[0x01d0/2] = temp & 0x00ff;
	}

	/* read inputs */
	workram[0x01e6/2] = readinputport(0) << 8;
	workram[0x01e8/2] = readinputport(1) << 8;
	workram[0x01ea/2] = readinputport(2) << 8;
}

static void wrestwar_i8751_sim(void)
{
	data16_t temp;

	/* signal a VBLANK to the main CPU */
	cpunum_set_input_line(0, 4, HOLD_LINE);
	
	/* process any new sound data */
	temp = workram[0x208e/2];
	if ((temp & 0xff00) != 0x0000)
	{
		memory_control_w(0x03, temp);
		workram[0x208e/2] = temp & 0x00ff;
	}

	/* read inputs */
	workram[0x2082/2] = readinputport(0);
}


/*************************************
 *
 *	Capacitor-backed RAM
 *
 *************************************/

static NVRAM_HANDLER( system16b )
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

static ADDRESS_MAP_START( system16b_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x010000, 0xffffff) AM_WRITE(unmapped_w)
ADDRESS_MAP_END



/*************************************
 *
 *	Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xdfff) AM_ROMBANK(1)
	AM_RANGE(0xe800, 0xe800) AM_READ(soundlatch_r)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x3e) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x3e) AM_READWRITE(YM2151_status_port_0_r, YM2151_data_port_0_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_WRITE(upd7759b_control_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3f) AM_READWRITE(upd7759b_status_r, upd7759b_0_port_w)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x3f) AM_READ(soundlatch_r)
ADDRESS_MAP_END



/*************************************
 *
 *	i8751 MCU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_data_map, ADDRESS_SPACE_DATA, 8 )
	ADDRESS_MAP_FLAGS( AMEF_UNMAP(1) )
	AM_RANGE(0x0000, 0x001f) AM_READWRITE(mcu_memory_map_r, mcu_memory_map_w)
ADDRESS_MAP_END



/*************************************
 *
 *	port definitions
 *
 *************************************/

static INPUT_PORTS_START( bullet )
	PORT_START_TAG("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START_TAG("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT | IPF_8WAY | IPF_PLAYER1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT | IPF_8WAY | IPF_PLAYER1 )

	PORT_START_TAG("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT | IPF_8WAY | IPF_PLAYER3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT | IPF_8WAY | IPF_PLAYER3 )

	PORT_START_TAG("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT | IPF_8WAY | IPF_PLAYER2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT | IPF_8WAY | IPF_PLAYER2 )

	PORT_START_TAG("DSW")
    PORT_DIPNAME( 0x01, 0x01, "Players" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
    PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
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
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


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
INPUT_PORTS_END


static INPUT_PORTS_START( cotton )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, "Easy" )
	PORT_DIPSETTING(    0x18, "Normal" )
	PORT_DIPSETTING(    0x08, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
INPUT_PORTS_END


static INPUT_PORTS_START( ddux )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x04, "Easy" )
	PORT_DIPSETTING(    0x06, "Normal" )
	PORT_DIPSETTING(    0x02, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x40, "150000" )
	PORT_DIPSETTING(    0x60, "200000" )
	PORT_DIPSETTING(    0x20, "300000" )
	PORT_DIPSETTING(    0x00, "400000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
INPUT_PORTS_END

static INPUT_PORTS_START( eswat )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x01, "2 Credits to Start" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Time" )
	PORT_DIPSETTING(    0x08, "Normal" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, "Easy" )
	PORT_DIPSETTING(    0x30, "Normal" )
	PORT_DIPSETTING(    0x10, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
INPUT_PORTS_END


static INPUT_PORTS_START( fz2 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
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
    PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) 
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "240 (Cheat)")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, "Easy" )
	PORT_DIPSETTING(    0xc0, "Normal" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
INPUT_PORTS_END

static INPUT_PORTS_START( fantzoneta )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

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
	PORT_DIPSETTING(    0x00, "240 (Cheat)")
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
	PORT_DIPNAME( 0x30, 0x30, "Extra Ship Cost" )
	PORT_DIPSETTING(    0x30, "5000" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, "Easy" )
	PORT_DIPSETTING(    0xc0, "Normal" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
INPUT_PORTS_END

static INPUT_PORTS_START( mvp )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Credits needed" )
	PORT_DIPSETTING(    0x01, "1 to start, 1 to continue" )
	PORT_DIPSETTING(    0x00, "2 to start, 1 to continue" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Innings" )
	PORT_DIPSETTING(    0x04, "1 Credit 1 Inning Only" )
	PORT_DIPSETTING(    0x00, "+2 Credits 3 Innings" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, "Time Limits" )
	PORT_DIPSETTING(    0x18, "Easy" )
	PORT_DIPSETTING(    0x28, "Easy 2" )
	PORT_DIPSETTING(    0x08, "Easy 3" )
	PORT_DIPSETTING(    0x38, "Normal" )
	PORT_DIPSETTING(    0x30, "Hard" )
	PORT_DIPSETTING(    0x10, "Hard 2" )
	PORT_DIPSETTING(    0x20, "Hard 3" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, "Easy" )
	PORT_DIPSETTING(    0x40, "Easy 2" )
	PORT_DIPSETTING(    0xc0, "Normal" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
INPUT_PORTS_END

static INPUT_PORTS_START( passshta )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )

	PORT_START_TAG("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x00, "Initial Point" )
	PORT_DIPSETTING(    0x06, "2000" )
	PORT_DIPSETTING(    0x0a, "3000" )
	PORT_DIPSETTING(    0x0c, "4000" )
	PORT_DIPSETTING(    0x0e, "5000" )
	PORT_DIPSETTING(    0x08, "6000" )
	PORT_DIPSETTING(    0x04, "7000" )
	PORT_DIPSETTING(    0x02, "8000" )
	PORT_DIPSETTING(    0x00, "9000" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x30, 0x30, "Point Table" )
	PORT_DIPSETTING(    0x20, "Easy" )
	PORT_DIPSETTING(    0x30, "Normal" )
	PORT_DIPSETTING(    0x10, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, "Easy" )
	PORT_DIPSETTING(    0xc0, "Normal" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
INPUT_PORTS_END


static INPUT_PORTS_START( sonicbom )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x06, "Easy" )
	PORT_DIPSETTING(    0x07, "Normal" )
	PORT_DIPSETTING(    0x05, "Hard 1" )
	PORT_DIPSETTING(    0x04, "Hard 2" )
	PORT_DIPSETTING(    0x03, "Hard 3" )
	PORT_DIPSETTING(    0x02, "Hard 4" )
	PORT_DIPSETTING(    0x01, "Hard 5" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x60, "40000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x00, "60000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

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
INPUT_PORTS_END

static INPUT_PORTS_START( tturf )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x03, 0x00, "Continues" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "Unlimited" )
	PORT_DIPSETTING(    0x03, "Unlimited" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, "Easy" )
	PORT_DIPSETTING(    0x0c, "Normal" )
	PORT_DIPSETTING(    0x04, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
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
	PORT_DIPNAME( 0x30, 0x20, "Starting Energy" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x30, "8" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Bonus Energy" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

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
INPUT_PORTS_END

static INPUT_PORTS_START( wrestwar )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Round Time" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x0c, "110" )
	PORT_DIPSETTING(    0x08, "120" )
	PORT_DIPSETTING(    0x04, "130" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Allow_Continue" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, "Easy" )
	PORT_DIPSETTING(    0xc0, "Normal" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
	{ YM3012_VOL(45,MIXER_PAN_LEFT,45,MIXER_PAN_RIGHT) },
	{ 0 }
};


static struct upd7759b_interface upd7759b_interface =
{
	1,
	{ UPD7759b_STANDARD_CLOCK },
	{ 48 },
	{ 0 },
	{ upd7759b_generate_nmi },
};


static struct YM2413interface ym2413_interface =
{
	1,
	4000000,
	{ YM2413_VOL(100,MIXER_PAN_CENTER,100,MIXER_PAN_CENTER) }
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

static MACHINE_DRIVER_START( system16b )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(system16b_map,0)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1)

	MDRV_CPU_ADD_TAG("sound", Z80, 5000000)
	MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
	MDRV_CPU_PROGRAM_MAP(sound_map,0)
	MDRV_CPU_IO_MAP(sound_portmap,0)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(1000000 * (262 - 224) / (262 * 60))

	MDRV_MACHINE_INIT(system16b)
	MDRV_NVRAM_HANDLER(system16b)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(40*8, 28*8)
	MDRV_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MDRV_GFXDECODE(gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(2048*3)

	MDRV_VIDEO_START(system16b)
	MDRV_VIDEO_UPDATE(system16b)

	/* sound hardware */
	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD_TAG("2151", YM2151, ym2151_interface)
	MDRV_SOUND_ADD_TAG("7759", UPD7759b, upd7759b_interface)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( system16b_8751 )
	MDRV_IMPORT_FROM(system16b)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_VBLANK_INT(i8751_main_cpu_vblank,1)

	MDRV_CPU_ADD(I8751, 8000000)
	MDRV_CPU_PROGRAM_MAP(mcu_map,0)
	MDRV_CPU_DATA_MAP(mcu_data_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_pulse,1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( timescn )
	MDRV_IMPORT_FROM(system16b)
	MDRV_VIDEO_START(timscanr)
MACHINE_DRIVER_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Bullet (JPN Ver.)
    (c)1987 Sega
    System 16B
    CPU Custom No. 317-0041
    Rom Board No. 171-5358

    J1 - -
    J2 ---
    J3 512
    J4 256
    J5 512
    J6 256
*/
ROM_START( bullet )
	ROM_REGION( 0x040000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr11010.a4",  0x000000, 0x08000, CRC(dd9001de) SHA1(6f6775104b7a9779c32da1dc72b92e03373af704) )
	ROM_LOAD16_BYTE( "epr11007.a1",  0x000001, 0x08000, CRC(d9e08110) SHA1(288b50fab079b33a17076b46ac8388c4ce8e68e9) )
	ROM_LOAD16_BYTE( "epr11011.a5",  0x010000, 0x08000, CRC(7f446b9f) SHA1(0b92ab100c13bdcdd0f770da5da5e19cb79afde1) )
	ROM_LOAD16_BYTE( "epr11008.a2",  0x010001, 0x08000, CRC(34824d3b) SHA1(7a3134a71ad176b8a08a919c0acb75ae1e05743b) )
	ROM_LOAD16_BYTE( "epr11012.a6",  0x020000, 0x08000, CRC(3992f159) SHA1(50686b394693ab01cbd159ae661f326c8eee50b8) )
	ROM_LOAD16_BYTE( "epr11009.a3",  0x020001, 0x08000, CRC(df199999) SHA1(2669e923aa4f1bedc788401f44ad19c318658f00) )

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* decryption key */
	ROM_LOAD( "317-0041.key", 0x0000, 0x2000, CRC(4cd4861a) SHA1(7578cfbd3efa28fa5eda0c007750b23060a305eb) )

	ROM_REGION16_BE( 0x040000, REGION_USER2, ROMREGION_ERASE00 ) /* working overlay */

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "epr10994.b9",  0x00000, 0x10000, CRC(3035468a) SHA1(778366815a2a74188d72d64c5e1e95215bc4ca81) )
	ROM_LOAD( "epr10995.b10", 0x10000, 0x10000, CRC(6b97aff1) SHA1(323bafe43a703476f6f4e68b46ec86bb9152f88e) )
	ROM_LOAD( "epr10996.b11", 0x20000, 0x10000, CRC(501bddd6) SHA1(545273b1b874b3e68d23b0dcae81c8531bd98756) )

	ROM_REGION16_BE( 0x80000, REGION_GFX2, ROMREGION_ERASE00 ) /* sprites */
	ROM_LOAD16_BYTE( "epr10999.b1", 0x00001, 0x010000, CRC(119f0008) SHA1(6a39b537bb58ea19ed3b0322ebca37e6574289fd) )
	ROM_LOAD16_BYTE( "epr11003.b5", 0x00000, 0x010000, CRC(2f429089) SHA1(08bf9d9c15fafbcb26604ff30be367ecf25404b2) )
	ROM_LOAD16_BYTE( "epr11000.b2", 0x20001, 0x010000, CRC(f5482bbe) SHA1(d8482ba73622798b15e78ab2c123d0fd4c33480a) )
	ROM_LOAD16_BYTE( "epr11004.b6", 0x20000, 0x010000, CRC(8c886df0) SHA1(348f9111fe45fc94cb32b101d0a1a6a39ef1ec50) )
	ROM_LOAD16_BYTE( "epr11001.b3", 0x40001, 0x010000, CRC(65ea71e0) SHA1(79224c445ceaa1d13a3616e58e9d4eb595e920cb) )
	ROM_LOAD16_BYTE( "epr11005.b7", 0x40000, 0x010000, CRC(ea2f9d50) SHA1(db62584591d62780f81de651869bc74a61363793) )
	ROM_LOAD16_BYTE( "epr11002.b4", 0x60001, 0x010000, CRC(9e25042b) SHA1(cb0e20ca8ca1c42ad2a95b83ea8711b7ad8e42f5) )
	ROM_LOAD16_BYTE( "epr11006.b8", 0x60000, 0x010000, CRC(6b7384f2) SHA1(5201e3b5e4aeb4bc8f5b3ba3d8a9ffb3705eccf4) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr10997.a7", 0x00000, 0x8000, CRC(5dd9cab5) SHA1(b9b27bbdc19feacb83cc5b33a74d910d86ac8f33) )
	ROM_LOAD( "epr10998.a8", 0x10000, 0x8000, CRC(f971a817) SHA1(502c95638e4fd5f87e5fc837cb44b39a5d62f4e4) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Cotton, Sega System 16B
	CPU: FD1094 (317-0181A)
	ROM Board: 171-5704
*/
ROM_START( cotton )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr13921a.a7", 0x000000, 0x20000, CRC(f047a037) SHA1(e4a9eeb1f2cf6b5ee3b2fcf74be917ffd3e6ca0e) )
	ROM_LOAD16_BYTE( "epr13919a.a5", 0x000001, 0x20000, CRC(651108b1) SHA1(1ab32085ca1a8cb3d464059b0abb10253decd423) )
	/* empty 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "epr13922a.a8", 0x080000, 0x20000, CRC(1ca248c5) SHA1(f7df2ccf54a3f1f09334c3b3df3dcd2ec33f99d1) )
	ROM_LOAD16_BYTE( "epr13920a.a6", 0x080001, 0x20000, CRC(fa3610f9) SHA1(6127496bf7cd47d4343291fc2e11673d77ccc550) )

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* decryption key */
	ROM_LOAD( "317-0181a.key", 0x0000, 0x2000, CRC(5c419b36) SHA1(df43153fc0420c42ca1559fb318bb21101e10f17) )

	ROM_REGION( 0xc0000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "opr13862.a14", 0x00000, 0x20000, CRC(a47354b6) SHA1(ce52813b245f1d491a134d4bd5ab074e71d20129) )
	ROM_LOAD( "opr13877.b14", 0x20000, 0x20000, CRC(d38424b5) SHA1(884ca190936aee2d2cac86491d4d0cdf4a45efe5) )
	ROM_LOAD( "opr13863.a15", 0x40000, 0x20000, CRC(8c990026) SHA1(07b4510936376c171f3b31d87ac6154361eb0cbc) )
	ROM_LOAD( "opr13878.b15", 0x60000, 0x20000, CRC(21c15b8a) SHA1(690d92420ec5465885e0f4870419992961420e33) )
	ROM_LOAD( "opr13864.a16", 0x80000, 0x20000, CRC(d2b175bf) SHA1(897b7c794d0e7229ea5e9a682f64266a947a818f) )
	ROM_LOAD( "opr13879.b16", 0xa0000, 0x20000, CRC(b9d62531) SHA1(e8c5e7b93339c00f75a3b66ce18f7838255577be) )

	ROM_REGION16_BE( 0x200000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "opr13869.b5", 0x000000, 0x20000, CRC(ab4b3468) SHA1(3071654a295152d609d2c2c1d4153b5ba3f174d5) )
	ROM_LOAD16_BYTE( "opr13865.b1", 0x000001, 0x20000, CRC(7024f404) SHA1(4b2f9cdfdd97218797a3e386106e53f713b8650d) )
	ROM_LOAD16_BYTE( "opr13870.b6", 0x040000, 0x20000, CRC(69b41ac3) SHA1(4c5a85e5a5ca9f8260557d4e97eb091dd857d63a) )
	ROM_LOAD16_BYTE( "opr13866.b2", 0x040001, 0x20000, CRC(6169bba4) SHA1(a24a418ee7cd0c1109870a2e7a91e430671897ed) )
	ROM_LOAD16_BYTE( "opr13871.b7", 0x080000, 0x20000, CRC(0801cf02) SHA1(3007bbbce2f327f4700e78e2b8672f4482189cd7) )
	ROM_LOAD16_BYTE( "opr13867.b3", 0x080001, 0x20000, CRC(b014f02d) SHA1(46f5ed0b44cee03a6aec9ec57b506bb15bf35e47) )
	ROM_LOAD16_BYTE( "opr13872.b8", 0x0c0000, 0x20000, CRC(f066f315) SHA1(bbeb24daaded994240d0cdb5cec2e662b677cb75) )
	ROM_LOAD16_BYTE( "opr13868.b4", 0x0c0001, 0x20000, CRC(e62a7cd6) SHA1(1e6d06345f7b6cef2e887d9b9cd45e0155140c5e) )
	ROM_LOAD16_BYTE( "opr13873.b10",0x100000, 0x20000, CRC(1bd145f3) SHA1(4744ffe9fbda453785345b46eb61b56730048f42) )
	ROM_LOAD16_BYTE( "opr13852.a1", 0x100001, 0x20000, CRC(943aba8b) SHA1(d0dd1665a8d9495a92ae4e35d6b15b966e8d43cd) )
	ROM_LOAD16_BYTE( "opr13874.b11",0x140000, 0x20000, CRC(4fd59bff) SHA1(2b4630e49b60593d668fe34d8faf712ac6928c14) )
	ROM_LOAD16_BYTE( "opr13853.a2", 0x140001, 0x20000, CRC(7ea93200) SHA1(8e2d8cd48a12306772653f25bddc99ad0597a698) )
	ROM_LOAD16_BYTE( "opr13894.b12",0x180000, 0x20000, CRC(e3d0bee2) SHA1(503a78123ca9d6f3405972bca281dcdaba929c99) )
	ROM_LOAD16_BYTE( "opr13891.a3", 0x180001, 0x20000, CRC(c6b3c414) SHA1(0f0d936e77eb483be8865e8d968d78260e88ca99) )
	ROM_LOAD16_BYTE( "opr13876.b13",0x1c0000, 0x20000, CRC(1c5ffad8) SHA1(13e5886ceece564cc71ba7f43a26d2b1782ccfc8) )
	ROM_LOAD16_BYTE( "opr13855.a4", 0x1c0001, 0x20000, CRC(856f3ee2) SHA1(72346d887ff9738ebe93acb2e3f8cd80d494621e) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr13892.a10", 0x00000, 0x08000, CRC(fdfbe6ad) SHA1(9ebb94889c0e96e6af9cdced084804ca98612d61) )
	ROM_LOAD( "opr13893.a11", 0x10000, 0x20000, CRC(384233df) SHA1(dfdf94697587a5ee45e97700f3741be54b90742b) )
ROM_END

/**************************************************************************************************************************
	Cotton, Sega System 16B
	CPU: FD1094 (317-0180T) // T could be wrong, it was handwritten
	ROM Board: 171-5704
*/
ROM_START( cottonu )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "cotton.a7", 0x000000, 0x20000, CRC(e7ef7d10) SHA1(de70ebf04d78e938bf568df158a47b377ddb3dc5) )
	ROM_LOAD16_BYTE( "cotton.a5", 0x000001, 0x20000, CRC(abe4f83e) SHA1(128f1815ecb27ee2a93e9d0708fdf6733dd1ab46) )
	/* empty 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "cotton.a8", 0x080000, 0x20000, CRC(fc0f4401) SHA1(e20fd6a3906b484c48abd9139744f7f960d751e1) )
	ROM_LOAD16_BYTE( "cotton.a6", 0x080001, 0x20000, CRC(f50f1ea2) SHA1(b53a18a71de4d0c7cba8bfcc156818712af9e2d5) )

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* decryption key */
	ROM_LOAD( "317-0180.key", 0x0000, 0x2000, CRC(a236b915) SHA1(374f2b23f0822891aadb86b779434ae2a1194932) )

	ROM_REGION( 0xc0000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "opr13862.a14", 0x00000, 0x20000, CRC(a47354b6) SHA1(ce52813b245f1d491a134d4bd5ab074e71d20129) )
	ROM_LOAD( "opr13877.b14", 0x20000, 0x20000, CRC(d38424b5) SHA1(884ca190936aee2d2cac86491d4d0cdf4a45efe5) )
	ROM_LOAD( "opr13863.a15", 0x40000, 0x20000, CRC(8c990026) SHA1(07b4510936376c171f3b31d87ac6154361eb0cbc) )
	ROM_LOAD( "opr13878.b15", 0x60000, 0x20000, CRC(21c15b8a) SHA1(690d92420ec5465885e0f4870419992961420e33) )
	ROM_LOAD( "opr13864.a16", 0x80000, 0x20000, CRC(d2b175bf) SHA1(897b7c794d0e7229ea5e9a682f64266a947a818f) )
	ROM_LOAD( "opr13879.b16", 0xa0000, 0x20000, CRC(b9d62531) SHA1(e8c5e7b93339c00f75a3b66ce18f7838255577be) )

	ROM_REGION16_BE( 0x200000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "opr13869.b5", 0x000000, 0x20000, CRC(ab4b3468) SHA1(3071654a295152d609d2c2c1d4153b5ba3f174d5) )
	ROM_LOAD16_BYTE( "opr13865.b1", 0x000001, 0x20000, CRC(7024f404) SHA1(4b2f9cdfdd97218797a3e386106e53f713b8650d) )
	ROM_LOAD16_BYTE( "opr13870.b6", 0x040000, 0x20000, CRC(69b41ac3) SHA1(4c5a85e5a5ca9f8260557d4e97eb091dd857d63a) )
	ROM_LOAD16_BYTE( "opr13866.b2", 0x040001, 0x20000, CRC(6169bba4) SHA1(a24a418ee7cd0c1109870a2e7a91e430671897ed) )
	ROM_LOAD16_BYTE( "opr13871.b7", 0x080000, 0x20000, CRC(0801cf02) SHA1(3007bbbce2f327f4700e78e2b8672f4482189cd7) )
	ROM_LOAD16_BYTE( "opr13867.b3", 0x080001, 0x20000, CRC(b014f02d) SHA1(46f5ed0b44cee03a6aec9ec57b506bb15bf35e47) )
	ROM_LOAD16_BYTE( "opr13872.b8", 0x0c0000, 0x20000, CRC(f066f315) SHA1(bbeb24daaded994240d0cdb5cec2e662b677cb75) )
	ROM_LOAD16_BYTE( "opr13868.b4", 0x0c0001, 0x20000, CRC(e62a7cd6) SHA1(1e6d06345f7b6cef2e887d9b9cd45e0155140c5e) )
	ROM_LOAD16_BYTE( "opr13873.b10",0x100000, 0x20000, CRC(1bd145f3) SHA1(4744ffe9fbda453785345b46eb61b56730048f42) )
	ROM_LOAD16_BYTE( "opr13852.a1", 0x100001, 0x20000, CRC(943aba8b) SHA1(d0dd1665a8d9495a92ae4e35d6b15b966e8d43cd) )
	ROM_LOAD16_BYTE( "opr13874.b11",0x140000, 0x20000, CRC(4fd59bff) SHA1(2b4630e49b60593d668fe34d8faf712ac6928c14) )
	ROM_LOAD16_BYTE( "opr13853.a2", 0x140001, 0x20000, CRC(7ea93200) SHA1(8e2d8cd48a12306772653f25bddc99ad0597a698) )
	ROM_LOAD16_BYTE( "opr13894.b12",0x180000, 0x20000, CRC(e3d0bee2) SHA1(503a78123ca9d6f3405972bca281dcdaba929c99) )
	ROM_LOAD16_BYTE( "opr13891.a3", 0x180001, 0x20000, CRC(c6b3c414) SHA1(0f0d936e77eb483be8865e8d968d78260e88ca99) )
	ROM_LOAD16_BYTE( "opr13876.b13",0x1c0000, 0x20000, CRC(1c5ffad8) SHA1(13e5886ceece564cc71ba7f43a26d2b1782ccfc8) )
	ROM_LOAD16_BYTE( "opr13855.a4", 0x1c0001, 0x20000, CRC(856f3ee2) SHA1(72346d887ff9738ebe93acb2e3f8cd80d494621e) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr13892.a10", 0x00000, 0x08000, CRC(fdfbe6ad) SHA1(9ebb94889c0e96e6af9cdced084804ca98612d61) )
	ROM_LOAD( "opr13893.a11", 0x10000, 0x20000, CRC(384233df) SHA1(dfdf94697587a5ee45e97700f3741be54b90742b) )
ROM_END

/**************************************************************************************************************************
	Cotton (Japan), Sega System 16B
	CPU: FD1094 (317-0179A)
	ROM Board: 171-5704
*/
ROM_START( cottonj )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr13858.a7", 0x000000, 0x20000, CRC(276f42fe) SHA1(47d2f910f6f101628cb0c660e5fd12c0e331bd99) )
	ROM_LOAD16_BYTE( "epr13856.a5", 0x000001, 0x20000, CRC(14e6b5e7) SHA1(708c69cddd1c60e729a74f539d40e67b2a6d9d6f) )
	/* empty 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "epr13859.a8", 0x080000, 0x20000, CRC(4703ef9d) SHA1(8b03a71736a599c337ad5d95cbc812ea38b0cc43) )
	ROM_LOAD16_BYTE( "epr13857.a6", 0x080001, 0x20000, CRC(de37e527) SHA1(124ce7c7eef1199c89735556cebf71255573a155) )

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* decryption key */
	ROM_LOAD( "317-0179a.key", 0x0000, 0x2000, CRC(488096d3) SHA1(5ffe81c141e1016f6a46f8fcee3e6d79c60151c0) )

	ROM_REGION( 0xc0000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "opr13862.a14", 0x00000, 0x20000, CRC(a47354b6) SHA1(ce52813b245f1d491a134d4bd5ab074e71d20129) )
	ROM_LOAD( "opr13877.b14", 0x20000, 0x20000, CRC(d38424b5) SHA1(884ca190936aee2d2cac86491d4d0cdf4a45efe5) )
	ROM_LOAD( "opr13863.a15", 0x40000, 0x20000, CRC(8c990026) SHA1(07b4510936376c171f3b31d87ac6154361eb0cbc) )
	ROM_LOAD( "opr13878.b15", 0x60000, 0x20000, CRC(21c15b8a) SHA1(690d92420ec5465885e0f4870419992961420e33) )
	ROM_LOAD( "opr13864.a16", 0x80000, 0x20000, CRC(d2b175bf) SHA1(897b7c794d0e7229ea5e9a682f64266a947a818f) )
	ROM_LOAD( "opr13879.b16", 0xa0000, 0x20000, CRC(b9d62531) SHA1(e8c5e7b93339c00f75a3b66ce18f7838255577be) )

	ROM_REGION16_BE( 0x200000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "opr13869.b5", 0x000000, 0x20000, CRC(ab4b3468) SHA1(3071654a295152d609d2c2c1d4153b5ba3f174d5) )
	ROM_LOAD16_BYTE( "opr13865.b1", 0x000001, 0x20000, CRC(7024f404) SHA1(4b2f9cdfdd97218797a3e386106e53f713b8650d) )
	ROM_LOAD16_BYTE( "opr13870.b6", 0x040000, 0x20000, CRC(69b41ac3) SHA1(4c5a85e5a5ca9f8260557d4e97eb091dd857d63a) )
	ROM_LOAD16_BYTE( "opr13866.b2", 0x040001, 0x20000, CRC(6169bba4) SHA1(a24a418ee7cd0c1109870a2e7a91e430671897ed) )
	ROM_LOAD16_BYTE( "opr13871.b7", 0x080000, 0x20000, CRC(0801cf02) SHA1(3007bbbce2f327f4700e78e2b8672f4482189cd7) )
	ROM_LOAD16_BYTE( "opr13867.b3", 0x080001, 0x20000, CRC(b014f02d) SHA1(46f5ed0b44cee03a6aec9ec57b506bb15bf35e47) )
	ROM_LOAD16_BYTE( "opr13872.b8", 0x0c0000, 0x20000, CRC(f066f315) SHA1(bbeb24daaded994240d0cdb5cec2e662b677cb75) )
	ROM_LOAD16_BYTE( "opr13868.b4", 0x0c0001, 0x20000, CRC(e62a7cd6) SHA1(1e6d06345f7b6cef2e887d9b9cd45e0155140c5e) )
	ROM_LOAD16_BYTE( "opr13873.b10",0x100000, 0x20000, CRC(1bd145f3) SHA1(4744ffe9fbda453785345b46eb61b56730048f42) )
	ROM_LOAD16_BYTE( "opr13852.a1", 0x100001, 0x20000, CRC(943aba8b) SHA1(d0dd1665a8d9495a92ae4e35d6b15b966e8d43cd) )
	ROM_LOAD16_BYTE( "opr13874.b11",0x140000, 0x20000, CRC(4fd59bff) SHA1(2b4630e49b60593d668fe34d8faf712ac6928c14) )
	ROM_LOAD16_BYTE( "opr13853.a2", 0x140001, 0x20000, CRC(7ea93200) SHA1(8e2d8cd48a12306772653f25bddc99ad0597a698) )
	ROM_LOAD16_BYTE( "mpr13875.b12",0x180000, 0x20000, CRC(6a66868d) SHA1(60961a8b1f193d0b08c1906f4a79123fa0db443a) )
	ROM_LOAD16_BYTE( "mpr13854.a3", 0x180001, 0x20000, CRC(1c942190) SHA1(514fac5cc7362e9e3168c84975c8fe0e34bb4471) )
	ROM_LOAD16_BYTE( "opr13876.b13",0x1c0000, 0x20000, CRC(1c5ffad8) SHA1(13e5886ceece564cc71ba7f43a26d2b1782ccfc8) )
	ROM_LOAD16_BYTE( "opr13855.a4", 0x1c0001, 0x20000, CRC(856f3ee2) SHA1(72346d887ff9738ebe93acb2e3f8cd80d494621e) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr13860.a10", 0x00000, 0x08000, CRC(6a57b027) SHA1(8f9de548df203605bb4ab9eececf09739b55adf1) )
	ROM_LOAD( "mpr13061.a11", 0x10000, 0x20000, CRC(4d21153f) SHA1(173ddd9633f255c39ca508c37d0562e374704e7b) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Dynamite Dux, Sega System 16B
	CPU: FD1094 (317-0096)
	ROM Board: 171-5521

	Pos.   Silk        Type        Part         Pos.   Silk        Type        Part

	A1     OBJ4-O      Unused                   B1     OBJ0-O      MPR-11920   27C512
	A2     OBJ5-O      Unused                   B2     OBJ1-O      MPR-11921   27C512
	A3     OBJ6-O      Unused                   B3     OBJ2-O      Unused
	A4     OBJ7-O      Unused                   B4     OBJ3-O      Unused
	A5     ROM0-O      190         27C1000      B5     OBJ0-E      MPR-11922   27C512
	A6     ROM1-O      913         27C1000      B6     OBJ1-E      MPR-11923   27C512
	A7     ROM0-E      191         27C1000      B7     OBJ2-E      Unused
	A8     ROM1-E      915         27C1000      B8     OBJ3-E      Unused
	A10    S-PROG      916         27C256       B10    OBJ4-E      Unused
	A11    SPEECH0     Unused                   B11    OBJ5-E      Unused
	A12    SPEECH1     Unused                   B12    OBJ6-E      Unused
	A13    No text     Unused                   B13    OBJ7-E      Unused
	A14    SCR01       MPR-11917   27C512       B14    SCR11       Unused
	A15    SCR02       MPR-11918   27C512       B15    SCR12       Unused
	A16    SCR03       MPR-11919   27C512       B16    SCR13       Unused
*/
ROM_START( ddux )
	ROM_REGION( 0x0c0000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "191.a7", 0x000000, 0x20000, CRC(500e400a) SHA1(c4d23cd09baf3049a8014190d82598d1e3f8ce89) )
	ROM_LOAD16_BYTE( "190.a5", 0x000001, 0x20000, CRC(2a698308) SHA1(bf6ad7a2bac77d088537d65041a69e6ea2788b14) )
	/* empty 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "915.a8", 0x080000, 0x20000, CRC(d8ed3132) SHA1(a9d5ad8f79fb635cc234a99fad398688a5f15926) )
	ROM_LOAD16_BYTE( "913.a6", 0x080001, 0x20000, CRC(30c6cb92) SHA1(2e17c74eeb37c9731fc2e365cc0114f7383c0106) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0096.key", 0x0000, 0x2000, CRC(6fd7d26e) SHA1(6e8feaf14d0981e8b0fa8dcf4cc45aabb0a09f83) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "mpr11917.a14", 0x00000, 0x10000, CRC(6f772190) SHA1(e68dc78785a1cb0da362efc8c4a088ccc580bd6e) )
	ROM_LOAD( "mpr11918.a15", 0x10000, 0x10000, CRC(c731db95) SHA1(b3b9cbd772f7bfd35355bcb2a7c0801b61eaf19f) )
	ROM_LOAD( "mpr11919.a16", 0x20000, 0x10000, CRC(64d5a491) SHA1(686151c9a58f524f786f52c03f086cdaa5728233) )

	ROM_REGION16_BE( 0x100000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "mpr11920.b1", 0x00001, 0x020000, CRC(e5d1e3cd) SHA1(d8c0f40dab00f1b09f6d018597fd45147f9ca3f6) )
	ROM_LOAD16_BYTE( "mpr11922.b5", 0x00000, 0x020000, CRC(70b0c4dd) SHA1(b67acab0c6a0f5051fc3fcda2476b8834f65b376) )
	ROM_LOAD16_BYTE( "mpr11921.b2", 0x40001, 0x020000, CRC(61d2358c) SHA1(216fd295ff9d56976f9b1c465a48806be843dd04) )
	ROM_LOAD16_BYTE( "mpr11923.b6", 0x40000, 0x020000, CRC(c9ffe47d) SHA1(fd6dc3781c6e7d1734a9f4a8e4a9c44cfc091e0a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "916.a10", 0x0000, 0x8000, CRC(7ab541cf) SHA1(feb88022ca1796d020e53e95ad345159bd415530) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	E-Swat (World), Sega System 16B
	CPU: FD1094 (317-0130)
	ROM Board: 171-5797
*/
ROM_START( eswat )
	ROM_REGION( 0x080000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr12659.bin", 0x000000, 0x40000, CRC(c5ab2db9) SHA1(5c7ded9a39e03d1f438cff1574b614295c8ebeef) )
	ROM_LOAD16_BYTE( "epr12658.bin", 0x000001, 0x40000, CRC(af40bd71) SHA1(3d7422d5c95fd2cbf1ac4916cc8c625a53391eea) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0130.key", 0x0000, 0x2000, CRC(ba7b717b) SHA1(7a5cef9f525d8b5e4199a94f6ba5e960ab44eb0c) )

	ROM_REGION( 0xc0000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "mpr12624.b11", 0x00000, 0x40000, CRC(375a5ec4) SHA1(42b9116bdc0e0a5b1dd667ac1856b4c2252829ba) )
	ROM_LOAD( "mpr12625.b12", 0x40000, 0x40000, CRC(3b8c757e) SHA1(0b66e8446d059a12e47e2a6fe8f0a333245bb95c) )
	ROM_LOAD( "mpr12626.b13", 0x80000, 0x40000, CRC(3efca25c) SHA1(0d866bf53a16b52719f73081e933f4db27d72ece) )

	ROM_REGION16_BE( 0x1c0000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "mpr12618.b1", 0x000001, 0x20000, CRC(0d1530bf) SHA1(bb8626cd98761c1c20cee117d00315c85621ba6a) )
	ROM_CONTINUE(                   0x100001, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12621.b4", 0x000000, 0x20000, CRC(18ff0799) SHA1(5417223378aef16ee2b4f438d1f8f11a23fe7265) )
	ROM_CONTINUE(                   0x100000, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12619.b2", 0x040001, 0x20000, CRC(32069246) SHA1(4913009bc72bf4f8b171b14fe06457f5784cab15) )
	ROM_CONTINUE(                   0x140001, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12622.b5", 0x040000, 0x20000, CRC(a3dfe436) SHA1(640ccc552114d403f35d441574d2f3e4f1d4a8f9) )
	ROM_CONTINUE(                   0x140000, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12620.b3", 0x080001, 0x20000, CRC(f6b096e0) SHA1(695ad1adbdc29f4d614645867e16de038cf92709) )
	ROM_CONTINUE(                   0x180001, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12623.b6", 0x080000, 0x20000, CRC(6773fef6) SHA1(91e646ea447be02254d060daf255d26afe0cc79e) )
	ROM_CONTINUE(                   0x180000, 0x20000 )

	ROM_REGION( 0x90000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12617.a13", 0x00000, 0x08000, CRC(7efecf23) SHA1(2b87af7cfaab5942a3f7b38c987fcba01d3475ab) )
	ROM_LOAD( "mpr12616.a11", 0x10000, 0x40000, CRC(254347c2) SHA1(bf2d83a69a5be375c7e42e9f7d6e65c1095a354c) )
ROM_END

/**************************************************************************************************************************
	E-Swat (Japan), Sega System 16B
	Game board : 837-7166/833-7164/834-7165
	CPU: FD1094 (317-0128)
	ROM Board: 171-5797
*/
ROM_START( eswatj )
	ROM_REGION( 0x080000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr12615.a2", 0x000000, 0x40000, CRC(388c2ea7) SHA1(a1a8ef973dfe3055981c998d780e36a1488e4052) )
	ROM_LOAD16_BYTE( "epr12614.a1", 0x000001, 0x40000, CRC(d5f0fb47) SHA1(d1363eaf74d40a76764fbff217a38e560d8c4c45) )

	ROM_REGION( 0x2000, REGION_USER1, 0 ) /* decryption key */
	ROM_LOAD( "317-0128.key", 0x0000, 0x2000, CRC(95f96277) SHA1(8e97de40872e113653463ff3787a5b1aa640e630) )

	ROM_REGION( 0xc0000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "mpr12624.b11", 0x00000, 0x40000, CRC(375a5ec4) SHA1(42b9116bdc0e0a5b1dd667ac1856b4c2252829ba) )
	ROM_LOAD( "mpr12625.b12", 0x40000, 0x40000, CRC(3b8c757e) SHA1(0b66e8446d059a12e47e2a6fe8f0a333245bb95c) )
	ROM_LOAD( "mpr12626.b13", 0x80000, 0x40000, CRC(3efca25c) SHA1(0d866bf53a16b52719f73081e933f4db27d72ece) )

	ROM_REGION16_BE( 0x1c0000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "mpr12618.b1", 0x000001, 0x20000, CRC(0d1530bf) SHA1(bb8626cd98761c1c20cee117d00315c85621ba6a) )
	ROM_CONTINUE(                   0x100001, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12621.b4", 0x000000, 0x20000, CRC(18ff0799) SHA1(5417223378aef16ee2b4f438d1f8f11a23fe7265) )
	ROM_CONTINUE(                   0x100000, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12619.b2", 0x040001, 0x20000, CRC(32069246) SHA1(4913009bc72bf4f8b171b14fe06457f5784cab15) )
	ROM_CONTINUE(                   0x140001, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12622.b5", 0x040000, 0x20000, CRC(a3dfe436) SHA1(640ccc552114d403f35d441574d2f3e4f1d4a8f9) )
	ROM_CONTINUE(                   0x140000, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12620.b3", 0x080001, 0x20000, CRC(f6b096e0) SHA1(695ad1adbdc29f4d614645867e16de038cf92709) )
	ROM_CONTINUE(                   0x180001, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12623.b6", 0x080000, 0x20000, CRC(6773fef6) SHA1(91e646ea447be02254d060daf255d26afe0cc79e) )
	ROM_CONTINUE(                   0x180000, 0x20000 )

	ROM_REGION( 0x90000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12617.a13", 0x00000, 0x08000, CRC(7efecf23) SHA1(2b87af7cfaab5942a3f7b38c987fcba01d3475ab) )
	ROM_LOAD( "mpr12616.a11", 0x10000, 0x40000, CRC(254347c2) SHA1(bf2d83a69a5be375c7e42e9f7d6e65c1095a354c) )
ROM_END

/**************************************************************************************************************************
	E-Swat (US), Sega System 16B
	CPU: FD1094 (317-0129)
	ROM Board: 171-5797
*/
ROM_START( eswatu )
	ROM_REGION( 0x080000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr12657.a2", 0x000000, 0x40000, CRC(43ca72aa) SHA1(4c6c536f0ef50570992116b50ca816bbc7d42801) )
	ROM_LOAD16_BYTE( "epr12656.a1", 0x000001, 0x40000, CRC(5f018967) SHA1(753cd39bdb51126591b5814d54bb57ed1f77cf22) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0129.key", 0x0000, 0x2000, CRC(128302c7) SHA1(6abeab57e3c55ab5e7f57add68a6e6e918d88c1c) )

	ROM_REGION( 0xc0000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "mpr12624.b11", 0x00000, 0x40000, CRC(375a5ec4) SHA1(42b9116bdc0e0a5b1dd667ac1856b4c2252829ba) )
	ROM_LOAD( "mpr12625.b12", 0x40000, 0x40000, CRC(3b8c757e) SHA1(0b66e8446d059a12e47e2a6fe8f0a333245bb95c) )
	ROM_LOAD( "mpr12626.b13", 0x80000, 0x40000, CRC(3efca25c) SHA1(0d866bf53a16b52719f73081e933f4db27d72ece) )

	ROM_REGION16_BE( 0x1c0000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "mpr12618.b1", 0x000001, 0x20000, CRC(0d1530bf) SHA1(bb8626cd98761c1c20cee117d00315c85621ba6a) )
	ROM_CONTINUE(                   0x100001, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12621.b4", 0x000000, 0x20000, CRC(18ff0799) SHA1(5417223378aef16ee2b4f438d1f8f11a23fe7265) )
	ROM_CONTINUE(                   0x100000, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12619.b2", 0x040001, 0x20000, CRC(32069246) SHA1(4913009bc72bf4f8b171b14fe06457f5784cab15) )
	ROM_CONTINUE(                   0x140001, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12622.b5", 0x040000, 0x20000, CRC(a3dfe436) SHA1(640ccc552114d403f35d441574d2f3e4f1d4a8f9) )
	ROM_CONTINUE(                   0x140000, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12620.b3", 0x080001, 0x20000, CRC(f6b096e0) SHA1(695ad1adbdc29f4d614645867e16de038cf92709) )
	ROM_CONTINUE(                   0x180001, 0x20000 )
	ROM_LOAD16_BYTE( "mpr12623.b6", 0x080000, 0x20000, CRC(6773fef6) SHA1(91e646ea447be02254d060daf255d26afe0cc79e) )
	ROM_CONTINUE(                   0x180000, 0x20000 )

	ROM_REGION( 0x90000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12617.a13", 0x00000, 0x08000, CRC(7efecf23) SHA1(2b87af7cfaab5942a3f7b38c987fcba01d3475ab) )
	ROM_LOAD( "mpr12616.a11", 0x10000, 0x40000, CRC(254347c2) SHA1(bf2d83a69a5be375c7e42e9f7d6e65c1095a354c) )
ROM_END

/**************************************************************************************************************************
+    Fantasy Zone II, Sega System 16C
+    CPU: 68000 (has Sega part number?)
+    ROM Board type: 171-5704 + Extra RAM
***************************************************************************************************************************/

ROM_START( fantzn2x )
	ROM_REGION( 0xc0000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "fz2.a7", 0x00000, 0x20000, CRC(94c05f0b) SHA1(53da68a919776a46ae96dbc094ff941308d13613) )
	ROM_LOAD16_BYTE( "fz2.a5", 0x00001, 0x20000, CRC(f3526895) SHA1(3197956608138601192f111d3bcc26662a7d6ec1) )
	/* empty 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "fz2.a8", 0x80000, 0x20000, CRC(b2ebb209) SHA1(bd40c90a372ab92a869bdd28d12cf52b45ecc80e) )
	ROM_LOAD16_BYTE( "fz2.a6", 0x80001, 0x20000, CRC(6833f546) SHA1(b4503cdb5bdb1322c34b9da3ff4227c740dad707) )

	ROM_REGION( 0x60000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "fz2.a14", 0x00000, 0x20000, CRC(1c0a4537) SHA1(3abdf51ea81780309bcfaf12c04efdf7cb15a649) )
	ROM_LOAD( "fz2.a15", 0x20000, 0x20000, CRC(2b933344) SHA1(5b53ea8d58cc3d157aec6926db048359984e4276) )
	ROM_LOAD( "fz2.a16", 0x40000, 0x20000, CRC(e63281a1) SHA1(72379c579484c1ef7784a9598d373446ef0a472b) )

	ROM_REGION16_BE( 0x180000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "fz2.b1",  0x000001, 0x20000, CRC(46bba615) SHA1(b291df4a83d7155eb7606f86ed733c24362a4db3) )
	ROM_LOAD16_BYTE( "fz2.b5",  0x000000, 0x20000, CRC(bebeee5d) SHA1(9e57e62c6b9136667aa90d7d423fc33ac6df4352) )
	ROM_LOAD16_BYTE( "fz2.b2",  0x040001, 0x20000, CRC(6681a7b6) SHA1(228df38601ba3895e9449921a64850941715b421) )
	ROM_LOAD16_BYTE( "fz2.b6",  0x040000, 0x20000, CRC(42d3241f) SHA1(c3240e3e1d7d398e74e76ba65adca6b06f0f67a9) )
	ROM_LOAD16_BYTE( "fz2.b3",  0x080001, 0x20000, CRC(5863926f) SHA1(0e591c4b85e5d572b3311bec2c1f6d2484204db6) )
	ROM_LOAD16_BYTE( "fz2.b7",  0x080000, 0x20000, CRC(cd830510) SHA1(8a32a1aa43f8af5e86f552f05da40b6e4ba12495) )
	ROM_LOAD16_BYTE( "fz2.b4",  0x0c0001, 0x20000, CRC(b98fa5b6) SHA1(c3f8891f81e80321e2ee5cc1f4d93b1867ed1868) )
	ROM_LOAD16_BYTE( "fz2.b8",  0x0c0000, 0x20000, CRC(e8248f68) SHA1(7876945d2baf1d7bdb9cc3a23be9f1a1681cede9) )
	ROM_LOAD16_BYTE( "fz2.a1",  0x100001, 0x20000, CRC(9d2f41f3) SHA1(54f5dc47d854cd26b108695f55263d8b8c29ce0e) )
	ROM_LOAD16_BYTE( "fz2.b10", 0x100000, 0x20000, CRC(7686ea33) SHA1(812a638f42500b30f80f9a3956c5eb4553cc35d0) )
	ROM_LOAD16_BYTE( "fz2.a2",  0x140001, 0x20000, CRC(3b4050b7) SHA1(8c7c8051c577a4b2ca54d7e60c100fbd5391551f) )
	ROM_LOAD16_BYTE( "fz2.b11", 0x140000, 0x20000, CRC(da8a95dc) SHA1(d44e1515008d4ee302f940ce7799fa9a790799e9) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "fz2.a10", 0x00000, 0x08000, CRC(92c92924) SHA1(3c98cea8f42c316405b28ae03469c6876de5e806) )
	ROM_LOAD( "fz2.a11", 0x10000, 0x20000, CRC(8c641bb9) SHA1(920da63961d2f3457c80d4c5f6d4f405374bb23a) )
ROM_END


ROM_START( fantzn2xp ) // based on trial version
	ROM_REGION( 0x0c0000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "cpu1b.bin", 0x000000, 0x20000, CRC(d23ef944) SHA1(e147a436a661442dda0e7643e1adc169cc12a9e8) )
	ROM_LOAD16_BYTE( "cpu1a.bin", 0x000001, 0x20000, CRC(407490e4) SHA1(19e0db715f8ca647454f9da03a3a944ecc0b956c) )
	/* empty 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "cpu1d.bin", 0x080000, 0x20000, CRC(c8c7716b) SHA1(9f8f83307dc47fc7456f292f17d51595fa60a77f) )
	ROM_LOAD16_BYTE( "cpu1c.bin", 0x080001, 0x20000, CRC(242e7b6e) SHA1(25d311d463ba95089be9b0b44c482a634ecb70a0) )

	ROM_REGION( 0x60000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "tilea.bin", 0x00000, 0x20000, CRC(47e0e5ce) SHA1(604ed062d7e7d640c49732d2c3202c991410381a) )
	ROM_LOAD( "tileb.bin", 0x20000, 0x20000, CRC(59e181b7) SHA1(0d57d7b49b85baafa3038dc5b7551fea85f4f5c8) )
	ROM_LOAD( "tilec.bin", 0x40000, 0x20000, CRC(375d354c) SHA1(2af5374777b13cb5aefb7fe90adea0b4d79e9202) )

	ROM_REGION16_BE( 0x140000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "obja.bin", 0x000001, 0x20000, CRC(9af87a4d) SHA1(b4f0f40f96c4081803742bc6576c37edff6eab16) )
	ROM_LOAD16_BYTE( "objb.bin", 0x000000, 0x20000, CRC(2fdbca68) SHA1(bc0c96bba9e89f2711267b737ecd4bf2a177edda) )
	ROM_LOAD16_BYTE( "objc.bin", 0x040001, 0x20000, CRC(2587487a) SHA1(eb04c03e918f1d98c36360f2b089018fe25f8dad) )
	ROM_LOAD16_BYTE( "objd.bin", 0x040000, 0x20000, CRC(8de4e7aa) SHA1(0b1b7b54306d680fb6c9142947a292847789b887) )
	ROM_LOAD16_BYTE( "obje.bin", 0x080001, 0x20000, CRC(dfada4ff) SHA1(1a8f2b706d629042f64b8158f906e351d77898f2) )
	ROM_LOAD16_BYTE( "objf.bin", 0x080000, 0x20000, CRC(65e5d23d) SHA1(97479e84c74ec65176365f5abd5f43927ec13056) )
	ROM_LOAD16_BYTE( "objg.bin", 0x0c0001, 0x20000, CRC(dc9fbb75) SHA1(b774d7b1debe65241358d3ef327773c3dce6ad1e) )
	ROM_LOAD16_BYTE( "objh.bin", 0x0c0000, 0x20000, CRC(97bb7c19) SHA1(07e2cc9a76ca5e01a4b66e55d2824caead2f033d) )
	ROM_LOAD16_BYTE( "obji.bin", 0x100001, 0x20000, CRC(c7790fee) SHA1(bd4ce6ad439042f3d2da9f756f7ba82630102386) )
	ROM_LOAD16_BYTE( "objj.bin", 0x100000, 0x20000, CRC(4535eb0e) SHA1(aaa067c509bc669a80a42df2f827c2bc0179b721) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "cpu2a.bin", 0x00000, 0x08000, CRC(92c92924) SHA1(3c98cea8f42c316405b28ae03469c6876de5e806) )
	ROM_LOAD( "cpu2b.bin", 0x10000, 0x20000, CRC(2c8ad475) SHA1(9ef1ed5aab81a82844ccb0949cb393a8d1abac92) )
ROM_END

ROM_START( fantzoneta ) // based on PS2 version, from bootleg conversion board
	ROM_REGION( 0x80000, REGION_CPU1, 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fzta__a07.bin", 0x00000, 0x20000, CRC(ad07d1fd) SHA1(63fbaa135a3860bd956d5147a5f64d951d2ebdba) )
	ROM_LOAD16_BYTE( "fzta__a05.bin", 0x00001, 0x20000, CRC(47dbe11b) SHA1(edc6960506745235bb8668eaf71139cef4a2cd16) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "fzta__a14.bin", 0x00000, 0x10000, CRC(9468ab33) SHA1(714660b9eafb78ef5d3aed218367b4e5708376f6) )
	ROM_LOAD( "fzta__a15.bin", 0x10000, 0x10000, CRC(22a3cf75) SHA1(6f21bc2a565738b997f898ed6ee631e9452662ca) )
	ROM_LOAD( "fzta__a16.bin", 0x20000, 0x10000, CRC(25cba87f) SHA1(abece2c27cd9d299319fa3ea87e016606426abc2) )

	ROM_REGION16_BE( 0x180000, REGION_GFX2, ROMREGION_ERASEFF ) // sprites
	ROM_LOAD16_BYTE( "fzta__b01.bin",  0x000001, 0x20000, CRC(0beb4a22) SHA1(e35f6d92b88f0730ccf03a090900ca2ed9824e74) )
	ROM_LOAD16_BYTE( "fzta__b05.bin",  0x000000, 0x20000, CRC(7f676c69) SHA1(231e7829b4ddb039f9075aebd1f2a123c79d396d) )
	ROM_LOAD16_BYTE( "fzta__a01.bin",  0x100001, 0x20000, CRC(40e1db9a) SHA1(305cd5e2d8512774638dfa724df6696ffa81ebb2) )
	ROM_LOAD16_BYTE( "fzta__b10.bin",  0x100000, 0x20000, CRC(acbb5cff) SHA1(bd356f664ec1c0e955161aa3afd06f2aeda80357) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) // sound CPU
	ROM_LOAD( "fzta__a10.bin", 0x00000, 0x08000, CRC(dab6fcd0) SHA1(151b62c5353533ae8660cbeebd8fe3219adbf4b5) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	MVP, Sega System 16B
	CPU: FD1094 (317-0143)
	ROM Board: 171-5797
*/
ROM_START( mvp )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "13000.rom", 0x000000, 0x40000, CRC(2e0e21ec) SHA1(3b72da0746fb0ece4311d6e935bc6b9ece3549ec) )
	ROM_LOAD16_BYTE( "12999.rom", 0x000001, 0x40000, CRC(fd213d28) SHA1(5324ee402a2f28a6c152905493da0052d4976b29) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0143.key", 0x0000, 0x2000, CRC(fba2e8da) SHA1(fb891af6a99958ee5b687012309a8107a7c433dd) )

	ROM_REGION( 0xc0000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "13011.rom", 0x00000, 0x40000, CRC(1cb871fc) SHA1(d20a46e538d57b25d78faa7deb9c11519b4111d3) )
	ROM_LOAD( "13012.rom", 0x40000, 0x40000, CRC(b75e6821) SHA1(a2b049995755d79a136a4b4b0dc78d902c5b9eed) )
	ROM_LOAD( "13013.rom", 0x80000, 0x40000, CRC(f1944a3c) SHA1(db59cadb435c26f3a957bd4996a083fa30c8bbd0) )

	ROM_REGION16_BE( 0x200000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "13003.rom", 0x000001, 0x20000, CRC(21424151) SHA1(156e15eee9ff7122c30a42bfec0b307073b7a375) )
	ROM_CONTINUE(                 0x100001, 0x20000 )
	ROM_LOAD16_BYTE( "13006.rom", 0x000000, 0x20000, CRC(2e9afd2f) SHA1(7fe0929e70e061878065fab2d26309066d14c038) )
	ROM_CONTINUE(                 0x100000, 0x20000 )
	ROM_LOAD16_BYTE( "13004.rom", 0x040001, 0x20000, CRC(0aa09dd3) SHA1(0cd58d29efa714e52c494ee1ec8a0e90c0a03e3c) )
	ROM_CONTINUE(                 0x140001, 0x20000 )
	ROM_LOAD16_BYTE( "13007.rom", 0x040000, 0x20000, CRC(55c8605b) SHA1(6c81e5f9bcd61f6e67c87ea5b25a8fe1ee50f14d) )
	ROM_CONTINUE(                 0x140000, 0x20000 )
	ROM_LOAD16_BYTE( "13005.rom", 0x080001, 0x20000, CRC(c899c810) SHA1(a251cfd8f99f2c2f98585cc9ba1d86f08b2eca51) )
	ROM_CONTINUE(                 0x180001, 0x20000 )
	ROM_LOAD16_BYTE( "13008.rom", 0x080000, 0x20000, CRC(b3d46dfc) SHA1(27a5c58d5fd974fbbb12b535a49aa7fb3f7d3f6a) )
	ROM_CONTINUE(                 0x180000, 0x20000 )
	ROM_LOAD16_BYTE( "13009.rom", 0x0c0001, 0x20000, CRC(126d2e37) SHA1(91317d3bcc4f918a8607cc0c383181c3641ebccf) )
	ROM_CONTINUE(                 0x1c0001, 0x20000 )
	ROM_LOAD16_BYTE( "13010.rom", 0x0c0000, 0x20000, CRC(df37c567) SHA1(05c959e379a3e20fe5e70912410e3bae4db13905) )
	ROM_CONTINUE(                 0x1c0000, 0x20000 )

	ROM_REGION( 0x90000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "13002.rom",	 0x00000, 0x08000, CRC(1b6e1515) SHA1(1816d48dcb1bfd819a2cfa55fb51e1ca04ad4feb) )
	ROM_LOAD( "13001.rom",   0x10000, 0x40000, CRC(e8cace8c) SHA1(5f47b935d927f2aa5f7a5f6dc52f5380baebe1bb) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Passing Shot, Sega System 16B
	CPU: FD1094 No. 317-0080
	ROM Board No. 171-5358

	J1 - -
	J2 ---
	J3 512
	J4 256
	J5 512
	J6 512
*/
ROM_START( passsht )
	ROM_REGION( 0x20000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr11871.a4", 0x000000, 0x10000, CRC(0f9ccea5) SHA1(515a2721a35332df0303f4b9616122194b5c7170) )
	ROM_LOAD16_BYTE( "epr11870.a1", 0x000001, 0x10000, CRC(df43ebcf) SHA1(3ca11a25819e1e8d5162f7b36cccc928d8efe150) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0080.key", 0x0000, 0x2000,  CRC(222d016f) SHA1(e426e5ea231e5b2ec1f40b0b2599269c14fef21a) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "opr11854.b9",  0x00000, 0x10000, CRC(d31c0b6c) SHA1(610d04988da70c30300cc5614817eda9d2204f39) )
	ROM_LOAD( "opr11855.b10", 0x10000, 0x10000, CRC(b78762b4) SHA1(d594ef846bd7fed8da91a89906b39c1a2867a1fe) )
	ROM_LOAD( "opr11856.b11", 0x20000, 0x10000, CRC(ea49f666) SHA1(36ccd32cdcbb7fcc300628bb59c220ec3c324d82) )

	ROM_REGION16_BE( 0x60000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "opr11862.b1",  0x00001, 0x10000, CRC(b6e94727) SHA1(0838e034f1f10d9cd1312c8c94b5c57387c0c271) )
	ROM_LOAD16_BYTE( "opr11865.b5",  0x00000, 0x10000, CRC(17e8d5d5) SHA1(ac1074b0a705be13c6e3391441e6cfec1d2b3f8a) )
	ROM_LOAD16_BYTE( "opr11863.b2",  0x20001, 0x10000, CRC(3e670098) SHA1(2cfc83f4294be30cd868738886ac546bd8489962) )
	ROM_LOAD16_BYTE( "opr11866.b6",  0x20000, 0x10000, CRC(50eb71cc) SHA1(463b4917ca19c7f4ad2c2845caa104d5e4a2dda3) )
	ROM_LOAD16_BYTE( "opr11864.b3",  0x40001, 0x10000, CRC(05733ca8) SHA1(1dbc7c99450ebe6a9fd8c0244fd3cb38b74984ef) )
	ROM_LOAD16_BYTE( "opr11867.b7",  0x40000, 0x10000, CRC(81e49697) SHA1(a70fa409e3555ad6c8f28930a7026fdf2deb8c65) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr11857.a7",  0x00000, 0x08000, CRC(789edc06) SHA1(8c89c94e503513c287807d187de78a7fbd75a7cf) )
	ROM_LOAD( "epr11858.a8",  0x10000, 0x08000, CRC(08ab0018) SHA1(0685f80a7d403208c9cfffea3f2035324f3924fe) )
	ROM_LOAD( "epr11859.a9",  0x20000, 0x08000, CRC(8673e01b) SHA1(e79183ab30e683fdf61ced2e9dbe010567c324cb) )
	ROM_LOAD( "epr11860.a10", 0x30000, 0x08000, CRC(10263746) SHA1(1f981fb185c6a9795208ecdcfba36cf892a99ed5) )
	ROM_LOAD( "epr11861.a11", 0x40000, 0x08000, CRC(38b54a71) SHA1(68ec4ef5b115844214ff2213be1ce6678904fbd2) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Sonic Boom, Sega System 16B
	CPU: FD1094 (317-0053)
	ROM Board: 171-5358

	Pos.   Silk        Type        Part                Pos.   Silk        Type        Part

	A1     EPR-11340   27C512      68000 program       B1     OPR-11350   27C512      Sprite data
	A2     EPR-11341   27C512      68000 program       B2     OPR-11351   27C512      Sprite data
	A3     Unused                  68000 program       B3     OPR-11352   27C512      Sprite data
	A4     EPR-11342   27C512      68000 program       B4     OPR-11353   27C512      Sprite data
	A5     EPR-11343   27C512      68000 program       B5     OPR-11354   27C512      Sprite data
	A6     Unused                  68000 program       B6     OPR-11355   27C512      Sprite data
	A7     EPR-11347   27C256      Z80 program         B7     OPR-11356   27C512      Sprite data
	A8     EPR-11348   27C256      Z80 sample data     B8     OPR-11357   27C512      Sprite data
	A9     EPR-11349   27C256      Z80 sample data     B9     OPR-11344   27C512      Tile data
	A10    Unused                  Z80 sample data     B10    OPR-11345   27C512      Tile data
	A11    Unused                  Z80 sample data     B11    OPR-11346   27C512      Tile data
*/
ROM_START( sonicbom )
	ROM_REGION( 0x040000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr11342.a4",  0x000000, 0x10000, CRC(454693f1) SHA1(3a9542443b4b403ccae968f2b5e2fc9cf810f85a) )
	ROM_LOAD16_BYTE( "epr11340.a1",  0x000001, 0x10000, CRC(03ba3fed) SHA1(2186639e34023690e5b9da942a015e11a6cdcc65) )
	ROM_LOAD16_BYTE( "epr11343.a5",  0x020000, 0x10000, CRC(edfeb7d4) SHA1(0f703e028f9ca9f3c4f5563f3c65ec9b938074a5) )
	ROM_LOAD16_BYTE( "epr11341.a2",  0x020001, 0x10000, CRC(0338f771) SHA1(a1a2928eb3f9826733bad54bbf17f622d9307285) )

	ROM_REGION( 0x2000, REGION_USER1, 0 )	/* decryption key */
	ROM_LOAD( "317-0053.key", 0x0000, 0x2000, CRC(91c80c88) SHA1(db2345257474c7e74a12ef8d125b7d0ea2ecd4c8) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "opr11344.b9",  0x00000, 0x10000, CRC(59a9f940) SHA1(b1c13cfa9609a22cbe047ee39681ccf8d0b3cf9c) )
	ROM_LOAD( "opr11345.b10", 0x10000, 0x10000, CRC(b44c068b) SHA1(05e875dc418aef12fc48d1df44f680249f6952de) )
	ROM_LOAD( "opr11346.b11", 0x20000, 0x10000, CRC(e5ada66c) SHA1(7e8e34ea909848d0d1b1fcccf628bf9ec169ae9b) )

	ROM_REGION16_BE( 0x80000, REGION_GFX2, ROMREGION_ERASE00 ) /* sprites */
	ROM_LOAD16_BYTE( "opr11350.b1", 0x00001, 0x010000, CRC(525ba1df) SHA1(e35487c8bf4009a767e54258d9a55056d13ba02a) )
	ROM_LOAD16_BYTE( "opr11354.b5", 0x00000, 0x010000, CRC(793fa3ac) SHA1(14d5a71667b4745d5b556cc15334dd9bff8de93f) )
	ROM_LOAD16_BYTE( "opr11351.b2", 0x20001, 0x010000, CRC(63b1f1ca) SHA1(1f19a3af099d4a6ad196968b0a3c17a11384e474) )
	ROM_LOAD16_BYTE( "opr11355.b6", 0x20000, 0x010000, CRC(fe0fa332) SHA1(15ea820f87399e35b798969d3800614701a61161) )
	ROM_LOAD16_BYTE( "opr11352.b3", 0x40001, 0x010000, CRC(047fa4b0) SHA1(d6c6f920a71db7734000cb64f3001145c4e05c6e) )
	ROM_LOAD16_BYTE( "opr11356.b7", 0x40000, 0x010000, CRC(aea3c39d) SHA1(2a4f01463b3f29c8d6858c3e99ef70ac548598b4) )
	ROM_LOAD16_BYTE( "opr11353.b4", 0x60001, 0x010000, CRC(4e0791f8) SHA1(3278bfd478a2fdbcf2d641268c9ca4ccd5a5bd2f) )
	ROM_LOAD16_BYTE( "opr11357.b8", 0x60000, 0x010000, CRC(a7c5ea41) SHA1(405e6ebd2cff22179de70fd31fae5ff967fbaf63) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr11347.a7", 0x00000, 0x8000, CRC(b41f0ced) SHA1(946b58c0f5b4541fac93add065916319302fe5e0) )
	ROM_LOAD( "epr11348.a8", 0x10000, 0x8000, CRC(89924588) SHA1(9b97da0f562c5faaf12e0f3f8943cdb4fe7a9d24) )
	ROM_LOAD( "epr11349.a9", 0x20000, 0x8000, CRC(8e4b6204) SHA1(ec0a2812b4726b5ff236f2fbb63fd2dd13cf4935) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Tough Turf, Sega System 16B
	CPU: 68000 + i8751 (317-0104)
	ROM Board: 171-5704
 */
ROM_START( tturf )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "12327.7a",  0x00000, 0x20000, CRC(0376c593) SHA1(3057828b16ee3b7cbb57a76047eecf74d1d8b465) )
	ROM_LOAD16_BYTE( "12326.5a",  0x00001, 0x20000, CRC(f998862b) SHA1(69902ab0162eb42e1d6a9792651a5d41cb77477d) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "12268.14a", 0x00000, 0x10000, CRC(e0dac07f) SHA1(c7f6de42eb93a8f34afdc300628735b5f40a34c2) )
	ROM_LOAD( "12269.15a", 0x10000, 0x10000, CRC(457a8790) SHA1(b701e1a1745cefb31083c8a3daa3b23181f89576) )
	ROM_LOAD( "12270.16a", 0x20000, 0x10000, CRC(69fc025b) SHA1(20be1242de27f1b997d43890051cc5d5ac8a127a) )

	ROM_REGION16_BE( 0x100000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "12279.1b", 0x00001, 0x10000, CRC(7a169fb1) SHA1(1ec6da0d2cfcf727e61f61c847fd8b975b64f944) )
	ROM_LOAD16_BYTE( "12283.5b", 0x00000, 0x10000, CRC(ae0fa085) SHA1(ae9af92d4dd0c8a0f064d24e647522b588fbd7f7) )
	ROM_LOAD16_BYTE( "12278.2b", 0x40001, 0x10000, CRC(961d06b7) SHA1(b1a9dea63785bfa2c0e7b931387b91dfcd27d79b) )
	ROM_LOAD16_BYTE( "12282.6b", 0x40000, 0x10000, CRC(e8671ee1) SHA1(a3732938c370f1936d867aae9c3d1e9bbfb57ede) )
	ROM_LOAD16_BYTE( "12277.3b", 0x80001, 0x10000, CRC(f16b6ba2) SHA1(00cc04c7b5aad82d51d2d252e1e57bcdc5e2c9e3) )
	ROM_LOAD16_BYTE( "12281.7b", 0x80000, 0x10000, CRC(1ef1077f) SHA1(8ce6fd7d32a20b93b3f91aaa43fe22720da7236f) )
	ROM_LOAD16_BYTE( "12276.4b", 0xc0001, 0x10000, CRC(838bd71f) SHA1(82d9d127438f5e1906b1cf40bf3b4727f2ee5685) )
	ROM_LOAD16_BYTE( "12280.8b", 0xc0000, 0x10000, CRC(639a57cb) SHA1(84fd8b96758d38f9e1ba1a3c2cf8099ec0452784) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
    ROM_LOAD( "epr-12328.a10", 0x00000, 0x08000, CRC(13a346de) SHA1(4e8cb12b7936c4c5d8ebc9ba563099ac2056ee60) )
	ROM_LOAD( "12329.11a",     0x10000, 0x10000, CRC(ed9a686d) SHA1(da433033d501ee871429ee676b3972b14179df9f) )		// speech
	// note: it needs this hole otherwise voice samples won't playback
	ROM_LOAD( "12330.12a",     0x30000, 0x10000, CRC(fb762bca) SHA1(ff9191c5ec38c711ebb7c2ad043f62b6d7e2203c) )
	
	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* protection MCU */
	//ROM_LOAD( "317-0104.bin", 0x00000, 0x1000, NO_DUMP )
ROM_END

/**************************************************************************************************************************
	Tough Turf, Sega System 16B
	CPU: 68000 + i8751 (317-0099)
	ROM Board: 171-5358
 */
ROM_START( tturfu )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr12266.bin",  0x00000, 0x10000, CRC(f549def8) SHA1(9e913509d46274bd192455321904ec9884b5f629) )
	ROM_LOAD16_BYTE( "epr12264.bin",  0x00001, 0x10000, CRC(f7cdb289) SHA1(9c386cf33a96a977c623c2f243db38fda2c41ba4) )
	ROM_LOAD16_BYTE( "epr12267.bin",  0x20000, 0x10000, CRC(3c3ce191) SHA1(036ece2be585a85e05c047fe4a1ab3b6814c3490) )
	ROM_LOAD16_BYTE( "epr12265.bin",  0x20001, 0x10000, CRC(8cdadd9a) SHA1(a17852f0eb8f63a82ff44cbeb100da72fe5890e0) )

	ROM_REGION( 0x30000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "12268.14a", 0x00000, 0x10000, CRC(e0dac07f) SHA1(c7f6de42eb93a8f34afdc300628735b5f40a34c2) )
	ROM_LOAD( "12269.15a", 0x10000, 0x10000, CRC(457a8790) SHA1(b701e1a1745cefb31083c8a3daa3b23181f89576) )
	ROM_LOAD( "12270.16a", 0x20000, 0x10000, CRC(69fc025b) SHA1(20be1242de27f1b997d43890051cc5d5ac8a127a) )

	ROM_REGION16_BE( 0x80000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "12276.4b", 0x00001, 0x10000, CRC(838bd71f) SHA1(82d9d127438f5e1906b1cf40bf3b4727f2ee5685) )
	ROM_LOAD16_BYTE( "12280.8b", 0x00000, 0x10000, CRC(639a57cb) SHA1(84fd8b96758d38f9e1ba1a3c2cf8099ec0452784) )
	ROM_LOAD16_BYTE( "12277.3b", 0x20001, 0x10000, CRC(f16b6ba2) SHA1(00cc04c7b5aad82d51d2d252e1e57bcdc5e2c9e3) )
	ROM_LOAD16_BYTE( "12281.7b", 0x20000, 0x10000, CRC(1ef1077f) SHA1(8ce6fd7d32a20b93b3f91aaa43fe22720da7236f) )
	ROM_LOAD16_BYTE( "12278.2b", 0x40001, 0x10000, CRC(961d06b7) SHA1(b1a9dea63785bfa2c0e7b931387b91dfcd27d79b) )
	ROM_LOAD16_BYTE( "12282.6b", 0x40000, 0x10000, CRC(e8671ee1) SHA1(a3732938c370f1936d867aae9c3d1e9bbfb57ede) )
	ROM_LOAD16_BYTE( "12279.1b", 0x60001, 0x10000, CRC(7a169fb1) SHA1(1ec6da0d2cfcf727e61f61c847fd8b975b64f944) )
	ROM_LOAD16_BYTE( "12283.5b", 0x60000, 0x10000, CRC(ae0fa085) SHA1(ae9af92d4dd0c8a0f064d24e647522b588fbd7f7) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12271.bin", 0x00000, 0x8000, CRC(99671e52) SHA1(ab21b7e7a449c8081b5db1fdb579071e31b6852c) )
	ROM_LOAD( "epr12272.bin", 0x10000, 0x8000, CRC(7cf7e69f) SHA1(62c9f75db8e2b4ece517167ba5c0712bac5f1d95) )
	ROM_LOAD( "epr12273.bin", 0x18000, 0x8000, CRC(28f0bb8b) SHA1(d99aff57d213b10ac10c37ceff2f125875816488) )
	ROM_LOAD( "epr12274.bin", 0x20000, 0x8000, CRC(8207f0c4) SHA1(169914861a52fa731a305e1ee2d230aa0d0d97fe) )
	ROM_LOAD( "epr12275.bin", 0x28000, 0x8000, CRC(182f3c3d) SHA1(1482fe08a05a721e315b1a3aa5bef4dddc72e26e) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* protection MCU */
	//ROM_LOAD( "317-0099.bin", 0x00000, 0x1000, NO_DUMP )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
	Wrestle War, Sega System 16B
	CPU: 68000 + i8751 (317-0103)
	ROM Board: 171-5704
	GAME NUMBER: 837-6889-02 (MB:833-6887-07, ROM:834-6888-07)

	LOC   NAME         LOC   NAME
	A1    OPR-12141    B1    MPR-12153
	A2    OPR-12142    B2    MPR-12154
	A3    -            B3    MPR-12155
	A4    -            B4    OPR-12156
	A5    EPR-12371    B5    MPR-12157
	A6    EPR-12144    B6    MPR-12158
	A7    EPR-12372    B7    MPR-12159
	A8    EPR-12146    B8    OPR-12160
	-                  B9    PAL 315-5298
	A10   EPR-12147    B10   OPR-12161
	A11   MPR-12148    B11   OPR-12162
	A12   MPR-12149    B12   -
	-                  B13   -
	A14   MPR-12150    B14   -
	A15   MPR-12151    B15   -
	A16   MPR-12152    B6    -
 */
ROM_START( wrestwar )
	ROM_REGION( 0xc0000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr12372.a7", 0x00000, 0x20000, CRC(eeaba126) SHA1(ca1f630ff0cfc301205d2b8226d7614eadc117b7) )
	ROM_LOAD16_BYTE( "epr12371.a5", 0x00001, 0x20000, CRC(6714600a) SHA1(8b04347f39bb8a8bc52b1bbfa367d42ea7c61bc9) )
	/* empty 0x40000 - 0x80000 */
	ROM_LOAD16_BYTE( "epr12146.a8", 0x80000, 0x20000, CRC(b77ba665) SHA1(b6a01ca857b5127ebb763f18cd4123185a7765a6) )
	ROM_LOAD16_BYTE( "epr12144.a6", 0x80001, 0x20000, CRC(ddf075cb) SHA1(5d887f0d5786fa62757c593d937bba6f150c1b12) )

	ROM_REGION( 0x60000, REGION_GFX1, ROMREGION_DISPOSE ) /* tiles */
	ROM_LOAD( "mpr12150.a14", 0x00000, 0x20000, CRC(6a821ab9) SHA1(e69f7e534835d4c820746ffc3ad76c3b7bb9b02e) )
	ROM_LOAD( "mpr12151.a15", 0x20000, 0x20000, CRC(2b1a0751) SHA1(8cb1027ef3728f5bdfdb5e2df0f0421f743cdc0a) )
	ROM_LOAD( "mpr12152.a16", 0x40000, 0x20000, CRC(f6e190fe) SHA1(4c8b334fb22c449d8d00c8f49f5eccbe008e244f) )

	ROM_REGION16_BE( 0x180000, REGION_GFX2, 0 ) /* sprites */
	ROM_LOAD16_BYTE( "mpr12153.b1",  0x000001, 0x20000, CRC(ffa7d368) SHA1(e5663ef1cbe8ab27be0919a3cd78d9a7747bbac6) )
	ROM_LOAD16_BYTE( "mpr12157.b5",  0x000000, 0x20000, CRC(8d7794c1) SHA1(ace87970cfa02ab8200173622633d0d70ef7aa9e) )
	ROM_LOAD16_BYTE( "mpr12154.b2",  0x040001, 0x20000, CRC(0ed343f2) SHA1(951bd616e63c5fe0aa3f387c9c12153b4f29675f) )
	ROM_LOAD16_BYTE( "mpr12158.b6",  0x040000, 0x20000, CRC(99458d58) SHA1(87020267610c5784e066bb4e3551852d27b4cf61) )
	ROM_LOAD16_BYTE( "mpr12155.b3",  0x080001, 0x20000, CRC(3087104d) SHA1(9ad0ea2b580820443c576cbb95d57467e32ea7b5) )
	ROM_LOAD16_BYTE( "mpr12159.b7",  0x080000, 0x20000, CRC(abcf9bed) SHA1(4f755cfd0304e877b798c31de50e15995f8c4edf) )
	ROM_LOAD16_BYTE( "opr12156.b4",  0x0c0001, 0x20000, CRC(41b6068b) SHA1(c1f1f51c4e0f4320cef7821bccac5b0c9e915d9b) )
	ROM_LOAD16_BYTE( "opr12160.b8",  0x0c0000, 0x20000, CRC(97eac164) SHA1(2bb62e6d8b2d662e9b31aa8d238a51af7c7905e8) )
	ROM_LOAD16_BYTE( "opr12141.a1",  0x100001, 0x20000, CRC(260311c5) SHA1(6b52b671252aef992c0546468c44b722bdb6a649) )
	ROM_LOAD16_BYTE( "opr12161.b10", 0x100000, 0x20000, CRC(35a4b1b1) SHA1(c3b8ba708f9f2822e48e52ea74d7e96f08182ac4) )
	ROM_LOAD16_BYTE( "opr12142.a2",  0x140001, 0x10000, CRC(12e38a5c) SHA1(05558a370b4e8100d2fa5e700a5ab76771ff7729) )
	ROM_LOAD16_BYTE( "opr12162.b11", 0x140000, 0x10000, CRC(fa06fd24) SHA1(20a578b82a75fe96a230c91645108fdc8b5bae21) )

	ROM_REGION( 0x50000, REGION_CPU2, 0 ) /* sound CPU */
	ROM_LOAD( "epr12147.a10", 0x00000, 0x08000, CRC(c3609607) SHA1(2e0acb775c60851bf0b2037b91b07ead061d5862) )
	ROM_LOAD( "mpr12148.a11", 0x10000, 0x20000, CRC(fb9a7f29) SHA1(7ba79c18ab4e586be2deccd78e4479d55eb75a7e) )
	ROM_LOAD( "mpr12149.a12", 0x30000, 0x20000, CRC(d6617b19) SHA1(aa36d257eaa52c8c871a39aaa2f29c203525dbaf) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )	/* protection MCU */
	//ROM_LOAD( "317-0103.bin", 0x00000, 0x1000, NO_DUMP )
ROM_END

/*************************************
 *
 *	Generic driver initialization
 *
 *************************************/

static DRIVER_INIT( generic_5358 )
{
	system16b_generic_init(ROM_BOARD_171_5358);
	system16b_configure_sprite_banks(0);
}

static DRIVER_INIT( generic_5358_small )
{
	system16b_generic_init(ROM_BOARD_171_5358_SMALL);
	system16b_configure_sprite_banks(0);
}


static DRIVER_INIT( generic_5521 )
{
	system16b_generic_init(ROM_BOARD_171_5521);
	system16b_configure_sprite_banks(1);
}


static DRIVER_INIT( generic_5704 )
{
	system16b_generic_init(ROM_BOARD_171_5704);
	system16b_configure_sprite_banks(1);
}


static DRIVER_INIT( generic_5797 )
{
	system16b_generic_init(ROM_BOARD_171_5797);
	system16b_configure_sprite_banks(1);
}

static DRIVER_INIT( generic_5704_extra_ram )
{
	system16b_generic_init(ROM_BOARD_171_5704_EXTRA_RAM);
	system16b_configure_sprite_banks(1);
}


static DRIVER_INIT( tturf_5704 )
{
	init_generic_5704();
	i8751_vblank_hook = tturf_i8751_sim;
}


static DRIVER_INIT( tturf_5358 )
{
	init_generic_5358();
	i8751_vblank_hook = tturf_i8751_sim;
}

static DRIVER_INIT( wrestwar_8751 )
{
	init_generic_5704();
	i8751_vblank_hook = wrestwar_i8751_sim;
}


/*************************************
 *
 *	Game driver(s)
 *
 *************************************/
GAME( 1987, bullet,   0,        system16b,      bullet,   generic_5358_small,  ROT0,     "Sega",   "Bullet" )
GAME( 1991, cotton,   0,        system16b,      cotton,   generic_5704,  ROT0,   "Sega / Success", "Cotton (World)" )
GAME( 1991, cottonu,  cotton,   system16b,      cotton,   generic_5704,  ROT0,   "Sega / Success", "Cotton (US)" )
GAME( 1991, cottonj,  cotton,   system16b,      cotton,   generic_5704,  ROT0,   "Sega / Success", "Cotton (Japan)" )
GAME( 1989, ddux,     0,        system16b,      ddux,     generic_5521,  ROT0,   "Sega",           "Dynamite Dux" )
GAME( 1989, eswat,    0,        system16b,      eswat,    generic_5797,  ROT0,   "Sega",           "E-Swat - Cyber Police (World)" )
GAME( 1989, eswatj,   eswat,    system16b,      eswat,    generic_5797,  ROT0,   "Sega",           "E-Swat - Cyber Police (Japan)" )
GAME( 1989, eswatu,   eswat,    system16b,      eswat,    generic_5797,  ROT0,   "Sega",           "E-Swat - Cyber Police (US)" )
GAME( 2008, fantzn2x, 0,        system16b,      fz2,      generic_5704_extra_ram, ROT0,   "Sega",  "Fantasy Zone II - The Tears of Opa-Opa (System 16C version)" )
GAME( 2008, fantzn2xp,fantzn2x, system16b,      fz2,      generic_5704_extra_ram, ROT0,   "Sega",  "Fantasy Zone II - The Tears of Opa-Opa (System 16C version, prototype)" )
GAME( 2008, fantzoneta,0,       system16b,      fantzoneta,generic_5704, ROT0,   "Sega",           "Fantasy Zone (Time Attack, bootleg)" )
GAME( 1989, mvp,      0,        system16b,      mvp,      generic_5797,  ROT0,   "Sega",           "MVP (US)" )
GAME( 1988, passsht,  0,        system16b,      passshta, generic_5358,  ROT270, "Sega",           "Passing Shot (2 Players)" )
GAME( 1987, sonicbom, 0,        system16b,      sonicbom, generic_5358,  ROT270, "Sega",           "Sonic Boom" )
GAME( 1989, tturf,    0,        system16b_8751, tturf,    tturf_5704,    ROT0,   "Sega / Sunsoft", "Tough Turf (Japan)")
GAME( 1989, tturfu,   tturf,    system16b_8751, tturf,    tturf_5358,    ROT0,   "Sega / Sunsoft", "Tough Turf (US)")
GAME( 1989, wrestwar, 0,        system16b_8751, wrestwar, wrestwar_8751, ROT270, "Sega",           "Wrestle War (8751, 317-0103)" )
/*gamezfan adds partial Segas16b driver plus updates associated video machine and prot code to MAME90 also added 
some extra Rom mem load tables for Bullet and Fantasy Zone II then moved across some extra games from system16.c
Tough Turf and Wrestle War sound or graphics problems are now fixed plus the non-bootleg versions of ESWAT DDUX 
and Passing Shot are now supported all games in this driver now playable also ported the updated UPD7759 from MAME92
but just reworked it to be UPD7759b fixing the sound and music in Cotton

Mahoneyt944 Fixed missing sound and music in the japan version of Tough Turf
and enabled the missing speech sounds.
*/
#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()