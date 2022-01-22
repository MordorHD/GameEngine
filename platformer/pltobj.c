#include "pltobj.h"
#include "pltgrid.h"
#include "pltcommon.h"

OBJECT **Objects;
int ObjectCount;
int ObjectCapacity;

int *RemoveQueue;
int RemoveQueueIndex;
int RemoveQueueCapacity;

int Objects_Count(void)
{
    return ObjectCount;
}

OBJECT **Objects_Base(void)
{
    return Objects;
}

OBJECT **Objects_Grow(void)
{
    int cnt;
    if(RemoveQueueIndex)
        return Objects + RemoveQueue[--RemoveQueueIndex];
    cnt = ObjectCount++;
    if(ObjectCount > ObjectCapacity)
    {
        ObjectCapacity = ObjectCapacity * 2 + 1;
        Objects = realloc(Objects, sizeof(*Objects) * ObjectCapacity);
    }
    return Objects + cnt;
}

// TODO: THRESHOLD
_Bool Objects_Remove(OBJECT *object)
{
    int index, oldRqIndex;
    int cnt;
    OBJECT **objects;
    index = -1;
    cnt = ObjectCount;
    objects = Objects;
    while(cnt--)
    {
        if(*objects == object)
        {
            index = ObjectCount - cnt - 1;
            break;
        }
        objects++;
    }
    if(cnt < 0)
        return 0;
    //printf("Removing %d, storing inside %d, %I64d\n", index, RemoveQueueIndex, object);
    oldRqIndex = RemoveQueueIndex++;
    if(RemoveQueueIndex >= RemoveQueueCapacity)
    {
        RemoveQueueCapacity *= 2;
        RemoveQueueCapacity++;
        RemoveQueue = realloc(RemoveQueue, sizeof(*RemoveQueue) * RemoveQueueCapacity);
    }
    if(object->activeHurtBox)
        Object_RemoveHurtBox(object->activeHurtBox);
    object->properties |= OBJPROP_DEACTIVATED | OBJPROP_INVISIBLE;
    RemoveQueue[oldRqIndex] = index;
    return 1;
}

HURTBOX **HurtBoxes;
int HurtBoxCount;
int HurtBoxCapacity;

PLTDEBUG void Object_DrawHurtBoxes(void)
{
    HURTBOX **boxes = HurtBoxes;
    int cnt = HurtBoxCount;
    while(cnt--)
    {
        JGRect2D(&((*boxes)->rect));
        boxes++;
    }
}

HURTBOX *Object_CreateHurtBox(JGRECT2D *rect, double damage, time_t duration, int side)
{
    HURTBOX *box;
    int cnt = HurtBoxCount++;
    if(HurtBoxCount > HurtBoxCapacity)
    {
        HurtBoxCapacity = HurtBoxCapacity * 2 + 1;
        HurtBoxes = realloc(HurtBoxes, sizeof(*HurtBoxes) * HurtBoxCapacity);
    }
    box = *(HurtBoxes + cnt) = malloc(sizeof(*box));
    box->rect = *rect;
    box->damage = damage;
    box->duration = duration;
    box->side = side;
    return box;
}

void Object_RemoveHurtBox(HURTBOX *box)
{
    int cnt = HurtBoxCount;
    HURTBOX **hurtBoxes = HurtBoxes;
    while(cnt--)
    {
        if(*hurtBoxes == box)
        {
            HurtBoxCount--;
            *hurtBoxes = *(HurtBoxes + HurtBoxCount);
            break;
        }
        hurtBoxes++;
    }
}

void Objects_Draw(JGCAMERA *cam)
{
    OBJECT **objects, *object;
    int cnt;
    objects = Objects;
    cnt = ObjectCount;
    while(cnt--)
    {
        object = *(objects++);
        if((object->properties & OBJPROP_INVISIBLE))
            continue;
        JGTranslate(object->rect.left - cam->x, object->rect.top - cam->y);
        JGTranslate(object->rotationAlignmentX * (object->rect.right - object->rect.left),
                    object->rotationAlignmentY * (object->rect.bottom - object->rect.top));
        JGRotate(object->rotation);
        object->ObjectHandle(object, NULL, OBJH_DRAW, 0);
        Object_Draw(object, cam);
        JGResetTransform();
    }
}

void Object_Draw(OBJECT *object, JGCAMERA *cam)
{
    OBJANIMATION *animation = &object->animation;
    FRAME *frame = (animation->animations + animation->animationIndex)->frames +

animation->frameIndex;
    if(object->properties & OBJPROP_FLIPPED)
        JGDrawImageSectionFlipped(Sprites_Get(), frame->framePos.left, frame->framePos.top,
                                  frame->framePos.right - frame->framePos.left, frame->framePos.bottom - frame->framePos.top,
                                  -(int) (object->rotationAlignmentX * (object->rect.right - object->rect.left)),
                                  -(int) (object->rotationAlignmentY * (object->rect.bottom - object->rect.top)),
                                   (int) (object->rect.right - object->rect.left),
                                   (int) (object->rect.bottom - object->rect.top));
    else
        JGDrawImageSection(Sprites_Get(), frame->framePos.left, frame->framePos.top,
                                  frame->framePos.right - frame->framePos.left, frame->framePos.bottom - frame->framePos.top,
                                  -(int) (object->rotationAlignmentX * (object->rect.right - object->rect.left)),
                                  -(int) (object->rotationAlignmentY * (object->rect.bottom - object->rect.top)),
                                   (int) (object->rect.right - object->rect.left),
                                   (int) (object->rect.bottom - object->rect.top));
}

void Objects_Update(time_t delta)
{
    OBJECT **objects, *object;
    HURTBOX **hurtBoxes;
    HURTBOX *hurtBox;
    int hbCnt;
    int cnt;
    objects = Objects;
    cnt = ObjectCount;
    while(cnt--)
    {
        object = *(objects++);
        if(object->combat.currentCooldown > 0)
            object->combat.currentCooldown -= delta;
        object->ObjectHandle(object, NULL, OBJH_UPDATE, delta);
        if(object->properties & OBJPROP_DEACTIVATED)
            continue;
        Object_Update(object, delta);
        hurtBoxes = HurtBoxes;
        hbCnt = HurtBoxCount;
        while(hbCnt--)
        {
            hurtBox = *hurtBoxes;
            if(JGRect2DIntersect(&hurtBox->rect, &object->rect))
            {
                if(hurtBox->side != object->side && object->combat.currentCooldown <= 0)
                {
                    object->combat.health -= hurtBox->damage;
                    object->ObjectHandle(object, NULL, OBJH_HURT, hurtBox->side);
                    object->combat.currentCooldown = object->combat.cooldown;
                }
            }
            hurtBoxes++;
        }
    }
    hurtBoxes = HurtBoxes;
    hbCnt = HurtBoxCount;
    while(hbCnt--)
    {
        hurtBox = *hurtBoxes;
        if(hurtBox->duration != INFINITE)
        {
            hurtBox->duration -= delta;
            if(hurtBox->duration <= 0)
            {
                HurtBoxCount--;
                *hurtBoxes = *(HurtBoxes + HurtBoxCount);
                continue;
            }
        }
        hurtBoxes++;
    }
}

int DefObjectHandle(OBJECT *object1, OBJECT *object2, int why, uintptr_t intention)
{ return 0; }

static _Bool OverflowX(int minY, int maxY, int xi)
{
    short *grid = Grid_Get() + xi + minY * GRID_WIDTH;
    //printf("X: %d, MINY: %d, MAXY: %d\n", xi, minY, maxY);
    minY = __max(minY, 0);
    maxY -= minY;
    maxY++;
    while(maxY--)
    {
        if(*grid)
            return 1;
        grid += GRID_WIDTH;
    }
    return 0;
}

static _Bool OverflowY(int minX, int maxX, int yi)
{
    short *grid = Grid_Get() + minX + yi * GRID_WIDTH;
    //printf("Y: %d, MINX: %d, MAXX: %d\n", yi, minX, maxX);
    maxX = __max(maxX, 0);
    maxX -= minX;
    maxX++;
    while(maxX--)
    {
        if(*grid)
            return 1;
        grid++;
    }
    return 0;
}

// true if object moved
static _Bool CheckCollision(OBJECT *obj)
{
    int minX, minY;
    int maxX, maxY;

    int px, py;

    double x, y;

    double velX, velY;

    _Bool moved;

    minX = (int) ((obj->rect.left + 1) / GRID_SPSIZE);
    maxX = (int) ((obj->rect.right - 1) / GRID_SPSIZE);

    minY = (int) ((obj->rect.top + 1) / GRID_SPSIZE);
    maxY = (int) ((obj->rect.bottom - 1) / GRID_SPSIZE);

    velX = obj->velocity.x;
    velY = obj->velocity.y;

    x = obj->rect.left + velX;
    y = obj->rect.top  + velY;

    moved = 0;

    //printf("%lf, %lf\n", dx, dy);

    if(velX != 0)
    {
        px = (int) ((velX < 0 ? x : x + (obj->rect.right - obj->rect.left)) / GRID_SPSIZE);
        if(OverflowX(minY, maxY, px))
        {
            if(velX > 0)
            {
                obj->rect.left -= obj->rect.right;
                obj->rect.right = px * GRID_SPSIZE;
                obj->rect.left += obj->rect.right;
            }
            else
            {
                obj->rect.right -= obj->rect.left;
                obj->rect.left = (px + 1) * GRID_SPSIZE;
                obj->rect.right += obj->rect.left;
            }
            if(!obj->ObjectHandle(obj, NULL, OBJH_HITWALL, 0))
                obj->velocity.x = 0;
            moved = 0;
        }
        else
        {
            obj->rect.left  += velX;
            obj->rect.right += velX;
            obj->properties |= OBJPROP_XMOVING;
            moved = 1;
        }
    }
    if(velY != 0)
    {
        py = (int) ((velY < 0 ? y : y + (obj->rect.bottom - obj->rect.top)) / GRID_SPSIZE);
        if(OverflowY(minX, maxX, py))
        {
            if(velY < 0)
            {
                if(obj->ObjectHandle(obj, NULL, OBJH_HITCEILING, 0))
                    return moved;
            }
            else
            {
                obj->properties |= OBJPROP_GROUNDED;
                obj->properties &= ~OBJPROP_JUMP;
                obj->rect.top -= obj->rect.bottom;
                obj->rect.bottom = py * GRID_SPSIZE;
                obj->rect.top += obj->rect.bottom;
                if(obj->ObjectHandle(obj, NULL, OBJH_HITFLOOR, 0))
                    return moved;
            }
            obj->velocity.y = 0;
        }
        else
        {
            obj->rect.top    += velY;
            obj->rect.bottom += velY;
            moved = 1;
        }
    }
    return moved;
}

_Bool Object_Update(OBJECT *object, time_t delta)
{
    _Bool ret = 1;
    OBJANIMATION *animation;
    ANIMATION *current, *next;
    FRAME *currentFrame;
    int cnt;
    int overflow;
    time_t cDelta;

    cDelta = delta;
    while(cDelta--)
    {
        /*printf("RECT[%lf, %lf, %lf, %lf], VEL[%lf, %lf]\n",
               object->rect.left, object->rect.top, object->rect.right, object->rect.bottom,
               object->velocity.x, object->velocity.y);
        Sleep(300);*/
        object->velocity.y += object->weight;
        if(CheckCollision(object))
            object->ObjectHandle(object, NULL, OBJH_MOVED, 0);
        object->velocity.x *= object->friction;
        if(object->velocity.x > -0.01 && object->velocity.x < 0.01)
        {
            object->velocity.x = 0;
            object->properties &= ~OBJPROP_XMOVING;
        }
    }
    //printf("%lf, %lf\n", object->velocity.x, object->velocity.y);

    //printf("RECT[%lf, %lf, %lf, %lf]\n", object->rect.left, object->rect.top, object->rect.right, object->rect.bottom);

    if(object->properties & OBJPROP_DEFANIMATION)
    {
        animation = &object->animation;
        if((cnt = animation->animationCount))
        {
            current = animation->animations + animation->animationIndex;
            next = animation->animations;
            while(cnt--)
            {
                if(object->properties & next->propertyMask)
                    break;
                next++;
            }
            if(cnt < 0)
            {
                animation->animationIndex = 0;
                animation->frameIndex = 0;
                animation->currFrameTime = current->frames->frameTime;
            }
            else
            {
                currentFrame = current->frames + animation->frameIndex;
                if(next != current)
                {
                    animation->frameIndex = 0;
                    animation->currFrameTime = currentFrame->frameTime;
                    animation->animationIndex = animation->animationCount - cnt - 1;
                    currentFrame = (animation->animations + animation->animationIndex)->frames;
                }
                animation->currFrameTime -= delta;
                if(animation->currFrameTime < 0)
                {
                    overflow = -animation->currFrameTime;
                    animation->currFrameTime = currentFrame->frameTime;
                    if(animation->frameIndex + 1 == current->frameCount)
                    {
                        ret &= object->ObjectHandle(object, NULL,

OBJH_ANIMATIONENDED, 0);
                        animation->frameIndex = current->frameLoopIndex;
                        currentFrame = current->frames;
                    }
                    else
                    {
                        animation->frameIndex++;
                        currentFrame++;
                    }
                    animation->currFrameTime = currentFrame->frameTime;
                }
                else
                    overflow = 0;
                animation->currFrameTime -= overflow;
            }
        }
    }
    return ret;
}
