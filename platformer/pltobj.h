#ifndef __PLTOBJ_H__
#define __PLTOBJ_H__

#include "../jg/jg.h"

#undef INFINITE
#define INFINITE (-1)

#define HBSIDE_LEFT 0
#define HBSIDE_MIDDLE 1
#define HBSIDE_RIGHT 2

typedef struct {
    JGRECT2D rect;
    double damage;
    time_t duration;
    int side;
} HURTBOX;

HURTBOX *Object_CreateHurtBox(JGRECT2D*, double, time_t, int);
void Object_RemoveHurtBox(HURTBOX*);

typedef struct {
    JGRECT framePos;
    int frameTime;
} FRAME;

typedef struct {
    FRAME *frames;
    int frameCount;
    int frameLoopIndex;
    int propertyMask;
} ANIMATION;

typedef struct {
    ANIMATION *animations;
    int animationCount;
    int animationIndex;
    int frameIndex;
    int currFrameTime;
} OBJANIMATION;

typedef struct {
    double damage;
    double health;
    double maxHealth;
    int currentCooldown;
    int cooldown;
} OBJCOMBAT;

struct object;

#define OBJPROP_BOUNCE 1
#define OBJPROP_GROUNDED 2
#define OBJPROP_XMOVING 4
#define OBJPROP_FLIPPED 8
#define OBJPROP_JUMP 16
#define OBJPROP_DEACTIVATED 32
#define OBJPROP_INVISIBLE 64
#define OBJPROP_DEFANIMATION 128

enum {
    OBJH_KEYPRESSED,
    OBJH_KEYRELEASED,
    OBJH_KEYTYPED,
    OBJH_MOUSEPRESSED,
    OBJH_MOUSERELEASED,
    OBJH_MOUSECLICKED,
    OBJH_MOUSEMOVED,
    OBJH_MOUSEWHEEL,

    OBJH_CREATE = 100,
    OBJH_UPDATE,
    OBJH_DRAW,
    OBJH_HURT,
    OBJH_MOVED,
    OBJH_TOUCH,
    OBJH_HITFLOOR,
    OBJH_HITWALL,
    OBJH_HITCEILING,
    OBJH_ANIMATIONENDED,
    OBJH_PICKITEM,
};

#define OBJH_HARM 1

#define OBJH_ITEMCOIN 100
#define OBJH_ITEMBOW 101

typedef struct object {
    JGRECT2D rect;
    JGVEC2D velocity;
    int properties;
    double weight;
    double friction;
    double rotation;
    double rotationAlignmentX;
    double rotationAlignmentY;
    int side;
    OBJANIMATION animation;
    OBJCOMBAT combat;
    HURTBOX *activeHurtBox;
    uintptr_t param;
    int (*ObjectHandle)(struct object*, struct object*, int, uintptr_t);
} OBJECT;

void Objects_Init(void);

void Object_AddClass(const char*, int, OBJECT*);

OBJECT **Objects_Grow(void);
_Bool Objects_Remove(OBJECT*);
int Objects_Count(void);
OBJECT **Objects_Base(void);
void Objects_Update(time_t);
void Objects_Draw(JGCAMERA*);
void Object_Draw(OBJECT*, JGCAMERA*);

OBJECT *Object_CreateFromID(int);
OBJECT *Object_Create(const char*);

int DefObjectHandle(OBJECT*, OBJECT*, int, uintptr_t);

_Bool Object_Update(OBJECT*, time_t);

void Object_DrawHurtBoxes(void);

#endif // __PLTOBJ_H__

