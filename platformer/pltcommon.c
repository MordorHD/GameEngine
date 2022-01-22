#include "pltcommon.h"

JGIMAGE SpriteSheet;

void Sprites_Load(void)
{
    SpriteSheet = JGLoadImage("platformer/Sprites.bmp", 1);
}

JGIMAGE Sprites_Get(void)
{
    return SpriteSheet;
}

gs_t GameState;

void Game_SetState(gs_t state)
{
    GameState = state;
}

gs_t Game_GetState(void)
{
    return GameState;
}
