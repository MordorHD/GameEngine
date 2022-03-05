#ifndef __JGLAYOUT_H__
#define __JGLAYOUT_H__

#include "jgdefs.h"
#include "jggeom.h"

// forward declaration needed, because including "jgcomps.h" would lead to weird nesting
struct ControlTag;
struct LayoutTag;

typedef void (*JGLAYOUTCOMPONENTS)(struct LayoutTag*, struct ControlTag**, int, const JGRECT*);

// these layouts do not get an extra struct because they just have the layoutFunc
#define JGLAYOUT_FLOW 0x1
#define JGLAYOUT_STACK 0x2

#define JGLAYOUT_GRIDBAG 0x3

typedef struct {
    struct ControlTag *partner;
    int gridX, gridY;
    int gridWidth, gridHeight;
} JGGRIDBC;

typedef struct GridBagLayoutTag {
    JGLAYOUTCOMPONENTS layoutFunc;
    JGGRIDBC *gridBcs;
    int gridBcCount;
    int gridBcCapacity;
    JGGRIDBC defaultGridBc;
} JGGRIDBAGLAYOUT;

void JGGridBagLayout_AddLayoutControl(struct ControlTag*, JGGRIDBC*);
bool JGGridBagLayout_RemoveLayoutControl(struct LayoutTag*, struct ControlTag*);

// a layout positions components of a container in a specific way
// every layout struct must have a layout function pointer as first value
typedef struct LayoutTag {
    int type;
    union {
        JGLAYOUTCOMPONENTS layoutFunc;
        JGGRIDBAGLAYOUT gbg;
    };
} JGLAYOUT;

bool JGGetStockLayout(JGLAYOUT*, int);

#endif // __JGLAYOUT_H__
