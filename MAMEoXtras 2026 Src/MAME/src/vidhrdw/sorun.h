/***************************************************************************

	Sega 16-bit common hardware

***************************************************************************/

/* globals */
extern UINT8 segaorun_display_enable;
extern data16_t *segaorun_tileram_0;
extern data16_t *segaorun_textram_0;
extern data16_t *segaorun_spriteram_0;
extern data16_t *segaorun_spriteram_1;
extern data16_t *segaorun_roadram_0;
extern data16_t *segaorun_rotateram_0;


/* misc functions */
void segaorun_set_display_enable(int enable);

/* palette handling */
void segaorun_palette_init(int entries);
WRITE16_HANDLER( segaorun_paletteram_w );

/* tilemap systems */
#define SEGAORUN_MAX_TILEMAPS		1

#define SEGAORUN_TILEMAP_HANGON		0
#define SEGAORUN_TILEMAP_16A		1
#define SEGAORUN_TILEMAP_16B		2
#define SEGAORUN_TILEMAP_16B_ALT	3

#define SEGAORUN_TILEMAP_FOREGROUND	0
#define SEGAORUN_TILEMAP_BACKGROUND	1
#define SEGAORUN_TILEMAP_TEXT		2

int segaorun_tilemap_init(int which, int type, int colorbase, int xoffs, int numbanks);
void segaorun_tilemap_reset(int which);
void segaorun_tilemap_draw(int which, struct mame_bitmap *bitmap, const struct rectangle *cliprect, int map, int priority, int priority_mark);
void segaorun_tilemap_set_bank(int which, int banknum, int offset);
void segaorun_tilemap_set_flip(int which, int flip);
void segaorun_tilemap_set_rowscroll(int which, int enable);
void segaorun_tilemap_set_colscroll(int which, int enable);

WRITE16_HANDLER( segaorun_tileram_0_w );
WRITE16_HANDLER( segaorun_textram_0_w );

/* sprite systems */
#define SEGAORUN_MAX_SPRITES		2

#define SEGAORUN_SPRITES_HANGON		0
#define SEGAORUN_SPRITES_16A		1
#define SEGAORUN_SPRITES_16B		2
#define SEGAORUN_SPRITES_SHARRIER	3
#define SEGAORUN_SPRITES_OUTRUN		4
#define SEGAORUN_SPRITES_XBOARD		5
#define SEGAORUN_SPRITES_YBOARD		6
#define SEGAORUN_SPRITES_YBOARD_16B	7

int segaorun_sprites_init(int which, int type, int colorbase, int xoffs);
void segaorun_sprites_draw(int which, struct mame_bitmap *bitmap, const struct rectangle *cliprect);
void segaorun_sprites_set_bank(int which, int banknum, int offset);
void segaorun_sprites_set_flip(int which, int flip);
void segaorun_sprites_set_shadow(int which, int shadow);
WRITE16_HANDLER( segaorun_sprites_draw_0_w );
WRITE16_HANDLER( segaorun_sprites_draw_1_w );

/* road systems */
#define SEGAORUN_MAX_ROADS			1

#define SEGAORUN_ROAD_HANGON		0
#define SEGAORUN_ROAD_SHARRIER		1
#define SEGAORUN_ROAD_OUTRUN		2

#define SEGAORUN_ROAD_BACKGROUND	0
#define SEGAORUN_ROAD_FOREGROUND	1

int segaorun_road_init(int which, int type, int colorbase1, int colorbase2, int colorbase3, int xoffs);
void segaorun_road_draw(int which, struct mame_bitmap *bitmap, const struct rectangle *cliprect, int priority);
READ16_HANDLER( segaorun_road_control_0_r );
WRITE16_HANDLER( segaorun_road_control_0_w );

/* rotation systems */
#define SEGAORUN_MAX_ROTATE			1

#define SEGAORUN_ROTATE_YBOARD		0

int segaorun_rotate_init(int which, int type, int colorbase);
void segaorun_rotate_draw(int which, struct mame_bitmap *bitmap, const struct rectangle *cliprect, struct mame_bitmap *srcbitmap);
READ16_HANDLER( segaorun_rotate_control_0_r );
