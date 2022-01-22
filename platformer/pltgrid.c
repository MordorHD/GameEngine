#include "pltgrid.h"

short Grid[GRID_LENGTH];

void Grid_Init(void)
{
    for(int x = 0; x < GRID_WIDTH; x++)
        Grid[x + (GRID_HEIGHT - 1) * GRID_WIDTH] = GRID_SAND;
    for(int x = 0; x < GRID_WIDTH; x++)
        Grid[x + (GRID_HEIGHT - 2) * GRID_WIDTH] = GRID_DIRT;
    for(int x = 0; x < GRID_WIDTH; x++)
        Grid[x + (GRID_HEIGHT - 3) * GRID_WIDTH] = GRID_DIRTGRASS;

    Grid[40 + (GRID_HEIGHT - 4) * GRID_WIDTH] = GRID_DIRTGRASS;
    Grid[41 + (GRID_HEIGHT - 4) * GRID_WIDTH] = GRID_DIRTGRASS;

    Grid[43 + (GRID_HEIGHT - 6) * GRID_WIDTH] = GRID_DIRTGRASS;
    Grid[44 + (GRID_HEIGHT - 6) * GRID_WIDTH] = GRID_DIRTGRASS;

    Grid[48 + (GRID_HEIGHT - 8) * GRID_WIDTH] = GRID_DIRTGRASS;
    Grid[49 + (GRID_HEIGHT - 8) * GRID_WIDTH] = GRID_DIRTGRASS;

    Grid[650] = GRID_DIRT;
    Grid[651] = GRID_DIRT;

    for(int y = 0; y < GRID_HEIGHT; y++)
        Grid[y * GRID_WIDTH] = GRID_DIRT;
}

short *Grid_Get(void)
{
    return Grid;
}
