/* Sega System 32 Protection related functions */

#include "driver.h"
#include "segas32.h"


/******************************************************************************
 ******************************************************************************
  Golden Axe 2 (Revenge of Death Adder)
 ******************************************************************************
 ******************************************************************************/

#define xxxx 0x00

const unsigned char ga2b_v25_opcode_table[256] = {
     xxxx,xxxx,0xEA,xxxx,xxxx,0x8B,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xFA,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x49,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,0xE8,xxxx,xxxx,0x75,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,0x8D,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBF,xxxx,0x88,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBC,
     xxxx,xxxx,xxxx,0x8A,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x83,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB8,0x26,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xEB,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB2,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,0xC3,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xB9,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,
     xxxx,xxxx,0x8E,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0xBE,xxxx,xxxx,xxxx,xxxx
};

#undef xxxx

void nec_v25_cpu_decrypt(void)
{
	int i;
	unsigned char *rom = memory_region(REGION_CPU3);
	UINT8* decrypted = auto_malloc(0x100000);
	UINT8* temp = malloc(0x100000);

	// set CPU3 opcode base
	memory_set_opcode_base(2,decrypted);

	// make copy of ROM so original can be overwritten
	memcpy(temp, rom, 0x10000);

	for(i = 0; i < 0x10000; i++)
	{
	        int j = BITSWAP16(i, 14, 11, 15, 12, 13, 4, 3, 7, 5, 10, 2, 8, 9, 6, 1, 0);

		// normal ROM data with address swap undone
		rom[i] = temp[j];

		// decryped opcodes with address swap undone
		decrypted[i] = ga2b_v25_opcode_table[ temp[j] ];
	}

	memcpy(rom+0xf0000, rom, 0x10000);
	memcpy(decrypted+0xf0000, decrypted, 0x10000);

	free(temp);
}

void decrypt_ga2b_protrom(void)
{
	nec_v25_cpu_decrypt();
}


WRITE16_HANDLER( ga2b_dpram_w )
{
	/* does it ever actually write.. */
}

READ16_HANDLER( ga2b_dpram_r )
{
	return (ga2b_dpram[offset])|(ga2b_dpram[offset+1]<<8);
}


#if 0 // simulation
READ16_HANDLER(ga2b_sprite_protection_r)
{
	static unsigned int prot[16] =
	{
		0x0a, 0,
		0xc5, 0,
		0x11, 0,
		0x11, 0,
		0x18, 0,
		0x18, 0,
		0x1f, 0,
		0xc6, 0,
	};

	return prot[offset];
}

READ16_HANDLER(ga2b_wakeup_protection_r)
{
	static const char *prot =
		"wake up! GOLDEN AXE The Revenge of Death-Adder! ";
	return prot[offset];
}
#endif

/******************************************************************************
 ******************************************************************************
  Dark Edge
 ******************************************************************************
 ******************************************************************************/

void darkedge_fd1149_vblank(void)
{
	program_write_word(0x20f072, 0);
	program_write_word(0x20f082, 0);

	if( program_read_byte(0x20a12c) != 0 )
	{
		program_write_byte(0x20a12c, program_read_byte(0x20a12c)-1 );

		if( program_read_byte(0x20a12c) == 0 )
			program_write_byte(0x20a12e, 1);
	}
}


WRITE16_HANDLER( darkedge_protection_w )
{
	logerror("%06x:darkedge_prot_w(%06X) = %04X & %04X\n",
		activecpu_get_pc(), 0xa00000 + 2*offset, data, mem_mask ^ 0xffff);
}


READ16_HANDLER( darkedge_protection_r )
{
	logerror("%06x:darkedge_prot_r(%06X) & %04X\n",
		activecpu_get_pc(), 0xa00000 + 2*offset, mem_mask ^ 0xffff);
	return 0xffff;
}