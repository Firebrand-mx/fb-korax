#ifndef __DOOMSDAY_DYNLIGHTS_H__
#define __DOOMSDAY_DYNLIGHTS_H__

#include "gl_def.h"
#include "gl_rl.h"

extern lumobj_t	*luminousList;
extern int		numLuminous;

// Setup.
void DL_InitBlockMap();
void DL_Clear();	// 'Physically' destroy the tables.

// Action.
void DL_InitForNewFrame();
void DL_ProcessSubsector(rendpoly_t *poly, subsector_t *ssec);
void DL_ProcessWallSeg(lumobj_t *lum, seg_t *seg, sector_t *frontsector);

// Helpers.
boolean DL_RadiusIterator(fixed_t x, fixed_t y, fixed_t radius, boolean (*func)(lumobj_t*,fixed_t));
boolean DL_BoxIterator(fixed_t box[4], void *ptr, boolean (*func)(lumobj_t*,void*));

#endif