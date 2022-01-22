#ifndef __PLTCOMMON_H__
#define __PLTCOMMON_H__

#include "../jg/jg.h"

#define PLTDEBUG

void Sprites_Load(void);
JGIMAGE Sprites_Get(void);

typedef enum {
    GS_RUNNING,
    GS_GAMEOVER,
    GS_PAUSED
} gs_t;

void Game_SetState(gs_t);
gs_t Game_GetState(void);

char *Game_GetKeys(void);

time_t JGGetMouseDownTime(void);
_Bool IsMouseDown(void);

#endif // __PLTCOMMON_H__
