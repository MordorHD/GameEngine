#ifndef __JGOBJ_H__
#define __JGOBJ_H__

#include "jg.h"

#define JGSTATE_FLIPPED JGSTATE_RESERVED12

typedef struct {
    JGCLASS *class_;
    uint32_t state;
    union {
        JGRECT2D rect;
        struct {
            double left;
            double top;
            double right;
            double bottom;
        };
    };
    JGTEXTURE texture;
    JGVEC2D velocity;
    float rotation;
    float rotationAlignmentX;
    float rotationAlignmentY;
    float weight;
    float friction;
} *JGOBJECT, __JGOBJECT;

uint64_t JGDefObjectProc(JGOBJECT, uint32_t, JGEVENT*);
JGOBJECT JGCreateObject(string_t, uint32_t, JGTEXTURE, float, float, float, float, float, float, float, float, float);
#define JGSetTexture(obj, texture_) ((JGOBJECT) (obj))->texture = (texture_)
#define JGSetWeight(obj, w) ((JGOBJECT) (obj))->weight = (w)
#define JGSetMass JGSetWeight
#define JGSetFriction(obj, f) ((JGOBJECT) (obj))->friction = (f)
#define JGRotate(obj, r) ((JGOBJECT) (obj))->rotation += r
#define JGSetRotation(obj, r) ((JGOBJECT) (obj))->rotation = r
#define JGSetRotationAlign(obj, x, y) ((JGOBJECT) ((JGOBJECT) (obj)))->rotationAlignmentX = (x), ((JGOBJECT) (obj))->rotationAlignmentY = (y)
#define JGAccelerate(obj, a) JGVecAdd(&(((JGOBJECT) (obj))->velocity), (a), &(((JGOBJECT) (obj))->velocity))
#define JGSetVelocity(obj, x, y) ((JGOBJECT) (obj))->velocity.x = (x), ((JGOBJECT) (obj))->velocity.y = y

#define JGMoveObject(obj, dx, dy) ((JGOBJECT) (obj))->left += (dx), ((JGOBJECT) (obj))->top += (dy), ((JGOBJECT) (obj))->right += (dx), ((JGOBJECT) (obj))->bottom += (dy)
#define JGResizeObject(obj, dx, dy) ((JGOBJECT) (obj))->right += (dx), ((JGOBJECT) (obj))->bottom += (dy)
void JGSetObjectPos(JGOBJECT, JGPOINT2D);

#define JGGetTexture(obj) (((JGOBJECT) (obj))->texture)
#define JGGetWeight(obj) (((JGOBJECT) (obj))->weight)
#define JGGetMass JGGetWeight
#define JGGetFriction(obj) (((JGOBJECT) (obj))->friction)
#define JGGetRotation(obj) (((JGOBJECT) (obj))->rotation)
#define JGGetRotationAlignX(obj) (((JGOBJECT) (obj))->rotationAlignmentX)
#define JGGetRotationAlignY(obj) (((JGOBJECT) (obj))->rotationAlignmentY)
// this is just here for consistency, don't use it
#define JGGetVelocity(obj, dest) *(dest) = ((JGOBJECT) (obj))->velocity

#endif // __JGOBJ_H__

