//----------------------------------------------------------------------------------------------------------------------------------

#ifndef BL_VARIABLES_H
#define BL_VARIABLES_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "bl_font_structs.h"

//----------------------------------------------------------------------------------------------------------------------------------
//Screen information
//----------------------------------------------------------------------------------------------------------------------------------

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   480

#define SCREEN_SIZE     (SCREEN_WIDTH * SCREEN_HEIGHT)

//----------------------------------------------------------------------------------------------------------------------------------
//Display data
//----------------------------------------------------------------------------------------------------------------------------------

//This first buffer is defined as 32 bits to be able to write it to file
extern uint32 maindisplaybuffer[SCREEN_SIZE / 2];

//----------------------------------------------------------------------------------------------------------------------------------

extern FONTDATA font_0;

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* BL_VARIABLES_H */

//----------------------------------------------------------------------------------------------------------------------------------