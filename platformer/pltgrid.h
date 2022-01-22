#ifndef __PLTGRID_H__
#define __PLTGRID_H__

enum {
    GRID_DIRTGRASS = 1,
    GRID_DIRT,
    GRID_SAND,
};

#define GRID_SPSIZE 32
#define GRID_WIDTH 100
#define GRID_HEIGHT 20
#define GRID_LENGTH (GRID_WIDTH*GRID_HEIGHT)

void Grid_Init(void);
short *Grid_Get(void);

#endif // __PLTGRID_H__

