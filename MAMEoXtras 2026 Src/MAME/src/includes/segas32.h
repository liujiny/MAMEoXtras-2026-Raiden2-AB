/***************************************************************************

    Sega System 32/Multi 32 hardware

***************************************************************************/


/*----------- defined in drivers/segas32.c -----------*/

extern data8_t *ga2b_dpram;
extern data16_t *system32b_workram;
extern data16_t *system32b_protram;


/*----------- defined in machine/segas32.c -----------*/

void decrypt_ga2b_protrom(void);
READ16_HANDLER( ga2b_dpram_r );
WRITE16_HANDLER( ga2b_dpram_w );

READ16_HANDLER( darkedge_protection_r );
WRITE16_HANDLER( darkedge_protection_w );
void darkedge_fd1149_vblank(void);


/*----------- defined in vidhrdw/segas32.c -----------*/

extern data16_t *system32b_videoram;
extern data16_t *system32b_spriteram;
extern data16_t *system32b_paletteram[2];
extern data16_t system32b_displayenable[2];
extern data16_t system32b_tilebank_external;

VIDEO_START(system32b);
VIDEO_START(multi32b);
VIDEO_UPDATE(system32b);
VIDEO_UPDATE(multi32b);
void system32b_set_vblank(int state);

READ16_HANDLER( system32b_videoram_r );
WRITE16_HANDLER( system32b_videoram_w );
READ32_HANDLER( multi32b_videoram_r );
WRITE32_HANDLER( multi32b_videoram_w );

READ16_HANDLER( system32b_spriteram_r );
WRITE16_HANDLER( system32b_spriteram_w );
READ32_HANDLER( multi32b_spriteram_r );
WRITE32_HANDLER( multi32b_spriteram_w );

READ16_HANDLER( system32b_paletteram_r );
WRITE16_HANDLER( system32b_paletteram_w );
READ32_HANDLER( multi32b_paletteram_0_r );
WRITE32_HANDLER( multi32b_paletteram_0_w );
READ32_HANDLER( multi32b_paletteram_1_r );
WRITE32_HANDLER( multi32b_paletteram_1_w );

READ16_HANDLER( system32b_sprite_control_r );
WRITE16_HANDLER( system32b_sprite_control_w );
READ32_HANDLER( multi32b_sprite_control_r );
WRITE32_HANDLER( multi32b_sprite_control_w );

READ16_HANDLER( system32b_mixer_r );
WRITE16_HANDLER( system32b_mixer_w );
WRITE32_HANDLER( multi32b_mixer_0_w );
WRITE32_HANDLER( multi32b_mixer_1_w );
