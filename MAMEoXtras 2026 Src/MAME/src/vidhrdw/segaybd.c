/***************************************************************************

	Sega Y-board hardware

***************************************************************************/

#include "driver.h"
#include "sorun.h"
#include "system16.h"



/*************************************
 *
 *	Statics
 *
 *************************************/

static struct mame_bitmap *yboard_bitmap;



/*************************************
 *
 *	Video startup
 *
 *************************************/

VIDEO_START( yboard )
{
	/* compute palette info */
	segaorun_palette_init(0x2000);
	
	/* allocate a bitmap for the yboard layer */
	yboard_bitmap = auto_bitmap_alloc_depth(512, 512, 16);

	/* initialize the sprites */
	if (segaorun_sprites_init(0, SEGAORUN_SPRITES_YBOARD_16B, 0x800, 0))
		return 1;
	if (segaorun_sprites_init(1, SEGAORUN_SPRITES_YBOARD, 0x1000, 0))
		return 1;
		
	/* initialize the rotation layer */
	if (segaorun_rotate_init(0, SEGAORUN_ROTATE_YBOARD, 0x000))
		return 1;

	return 0;
}



/*************************************
 *
 *	Video update
 *
 *************************************/

VIDEO_UPDATE( yboard )
{
	struct rectangle yboard_clip;

	/* if no drawing is happening, fill with black and get out */
	if (!segaorun_display_enable)
	{
		fillbitmap(bitmap, get_black_pen(), cliprect);
		return;
	}
	
	/* draw the yboard sprites */
	yboard_clip.min_x = yboard_clip.min_y = 0;
	yboard_clip.max_x = yboard_clip.max_y = 511;
	segaorun_sprites_draw(1, yboard_bitmap, &yboard_clip);

	/* apply rotation */
	segaorun_rotate_draw(0, bitmap, cliprect, yboard_bitmap);

	/* draw the 16B sprites */
	fillbitmap(priority_bitmap, 0, cliprect);
	segaorun_sprites_draw(0, bitmap, cliprect);
}
