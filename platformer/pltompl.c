#include "pltobj.h"
#include "pltcommon.h"
#include <stdlib.h>
#include "token.h"

#define ARROW_KEEP_DURATION 10000

typedef struct {
    const char *className;
    OBJECT classInfo;
} OBJECTCLASS;

OBJECTCLASS *ObjectClasses;
int ObjectClassCount;
int ObjectClassCapacity;

void Object_AddClass(const char *nameStart, int nameLen, OBJECT *object)
{
    int cnt = ObjectClassCount++;
    if(ObjectClassCount > ObjectClassCapacity)
    {
        ObjectClassCapacity = ObjectClassCapacity * 2 + 1;
        ObjectClasses = realloc(ObjectClasses, sizeof(*ObjectClasses) * ObjectClassCapacity);
    }
    (ObjectClasses + cnt)->className = CreateStringCopy(nameStart, nameLen);
    (ObjectClasses + cnt)->classInfo = *object;
}

int ArrowHandle(OBJECT *arrow, OBJECT *object, int why, uintptr_t intention)
{
    int t;
    switch(why)
    {
    case OBJH_CREATE:
        arrow->activeHurtBox = Object_CreateHurtBox(&arrow->rect, arrow->combat.damage, INFINITE, HBSIDE_LEFT);
        arrow->param = -1;
        break;
    case OBJH_MOVED:
        if(arrow->activeHurtBox)
            arrow->activeHurtBox->rect = arrow->rect;
        arrow->rotation = -atan2(arrow->velocity.y, arrow->velocity.x);
        break;
    case OBJH_UPDATE:
        t = arrow->param;
        if(t > 0)
        {
            t -= intention;
            arrow->param = t;
            if(t < 0)
            {
                Objects_Remove(arrow);
            }
        }
        break;
    case OBJH_HITFLOOR:
    case OBJH_HITWALL:
    case OBJH_HITCEILING:
        arrow->properties |= OBJPROP_DEACTIVATED;
        if(arrow->activeHurtBox)
        {
            Object_RemoveHurtBox(arrow->activeHurtBox);
            arrow->activeHurtBox = NULL;
            arrow->param = ARROW_KEEP_DURATION;
        }
        return 1;
    }
    return 0;
}

int PlayerHandle(OBJECT *player, OBJECT *object, int why, uintptr_t intention)
{
    static OBJECT *weapon;
    OBJECT **objects, *object_;
    char *keys;
    int cnt;
    switch(why)
    {
    case OBJH_CREATE:
        weapon = Object_Create("Bow");
        break;
    case OBJH_MOVED:
        objects = Objects_Base() + 1;
        cnt = Objects_Count() - 1;
        while(cnt--)
        {
            object_ = *objects;
            if(JGRect2DIntersect(&player->rect, &object_->rect))
                object_->ObjectHandle(object_, player, OBJH_TOUCH, 0);
            objects++;
        }
        weapon->rect = player->rect;
        weapon->velocity.x = weapon->velocity.y = 0;
        break;
    case OBJH_UPDATE:
        keys = Game_GetKeys();
        if(keys['W'] && (player->properties & OBJPROP_GROUNDED))
        {
            player->velocity.y = -1.0d;
            player->properties &= ~OBJPROP_GROUNDED;
            player->properties |= OBJPROP_JUMP;
        }
        if(keys['A'])
        {
            player->velocity.x -= intention / 50.0d;
            player->properties |= OBJPROP_FLIPPED;
        }
        if(keys['S'])
            player->velocity.y += intention / 50.0d;
        if(keys['D'])
        {
            player->velocity.x += intention / 50.0d;
            player->properties &= ~OBJPROP_FLIPPED;
        }
        break;
    case OBJH_HURT:
        if(player->combat.health <= 0.0d)
            Game_SetState(GS_GAMEOVER);
        break;
    case OBJH_PICKITEM:
        Objects_Remove(object);
        break;
    }
    return 0;
}

int FallingSandHandle(OBJECT *sand, OBJECT *object, int why, uintptr_t intention)
{
    switch(why)
    {
    case OBJH_CREATE:
        sand->activeHurtBox = Object_CreateHurtBox(&sand->rect, sand->combat.damage, INFINITE, HBSIDE_RIGHT);
        break;
    case OBJH_MOVED:
        if(sand->activeHurtBox)
            sand->activeHurtBox->rect = sand->rect;
        break;
    case OBJH_HITFLOOR:
        if(sand->activeHurtBox)
        {
            Object_RemoveHurtBox(sand->activeHurtBox);
            sand->activeHurtBox = NULL;
        }
        break;
    case OBJH_ANIMATIONENDED:
        return !Objects_Remove(sand);
    }
    return 0;
}

int CoinHandle(OBJECT *coin, OBJECT *object, int why, uintptr_t intention)
{
    switch(why)
    {
    case OBJH_MOVED:
        break;
    case OBJH_TOUCH:
        if(intention != OBJH_HARM)
            object->ObjectHandle(object, coin, OBJH_PICKITEM, OBJH_ITEMCOIN);
        break;
    }
    return 0;
}

int BowHandle(OBJECT *bow, OBJECT *object, int why, uintptr_t intention)
{
    const JGEVENT *event;
    JGCAMERA cam;
    OBJECT *arrow;
    double dx, dy, s;
    time_t downTime;
    switch(why)
    {
    case OBJH_MOVED:
        break;
    case OBJH_MOUSEMOVED:
        event = (const JGEVENT*) intention;
        JGGetCamera(&cam);
        dx = event->x + cam.x - (bow->rect.left + 16.0d);
        dy = event->y + cam.y - (bow->rect.top  + 8.0d);
        bow->rotation = -atan2(dy, dx);
        break;
    case OBJH_MOUSERELEASED:
        if((downTime = JGGetMouseDownTime()) < 150)
            break;
        event = (const JGEVENT*) intention;
        JGGetCamera(&cam);
        arrow = Object_Create("Arrow");
        arrow->rect.left   = (bow->rect.left + bow->rect.right - 32.0d) * 0.5d;
        arrow->rect.top    = (bow->rect.top + bow->rect.bottom - 16.0d) * 0.5d;
        arrow->rect.right  = arrow->rect.left + 32.0d;
        arrow->rect.bottom = arrow->rect.top + 16.0d;
        dx = event->x + cam.x - (arrow->rect.left + 16.0d);
        dy = event->y + cam.y - (arrow->rect.top  + 8.0d);
        s = -1.0d / (__min(downTime / 200, 3) + 1.0d) + 1.3d;
        s /= sqrt(dx * dx + dy * dy);
        dx *= s;
        dy *= s;
        arrow->velocity = (JGVEC2D) {
            .x = dx * 3,
            .y = dy * 3
        };
        break;
    case OBJH_UPDATE:
        if(IsMouseDown())
        {
            downTime = JGGetMouseDownTime();
            bow->animation.frameIndex = __min(downTime / 200, 3);
        }
        else
            bow->animation.frameIndex = 0;
        break;
    case OBJH_TOUCH:
        //if(intention != OBJH_HARM)
        //    object->ObjectHandle(object, bow, OBJH_PICKITEM, OBJH_ITEMBOW);
        break;
    case OBJH_HITFLOOR:
        bow->velocity.x = 0;
        break;
    }
    return 0;
}

int CactusEnemyHandle(OBJECT *cactus, OBJECT *object, int why, uintptr_t intention)
{
    OBJECT *player;
    double dx;
    switch(why)
    {
    case OBJH_CREATE:
        cactus->activeHurtBox = Object_CreateHurtBox(&cactus->rect, cactus->combat.damage, INFINITE, HBSIDE_RIGHT);
        break;
    case OBJH_DRAW:
        if(cactus->combat.currentCooldown > 0)
        {
            JGFill(JGRED);
            JGRect(0, 0, -(cactus->rect.right - cactus->rect.left) * 0.5, -(cactus->rect.bottom - cactus->rect.top) * 0.5);
        }
        break;
    case OBJH_UPDATE:
        player = *Objects_Base();
        if((dx = cactus->rect.left - player->rect.right) > 0 && dx < 800)
            cactus->velocity.x -= 0.9d * rand() / RAND_MAX;
        else if((dx = player->rect.left - cactus->rect.right) > 0 && dx < 800)
            cactus->velocity.x += 0.9d * rand() / RAND_MAX;
        else
            cactus->velocity.x += 1.8d * rand() / RAND_MAX - 0.9d;
        break;
    case OBJH_HURT:
        if(cactus->combat.health <= 0.0001d)
            Objects_Remove(cactus);
        break;
    case OBJH_MOVED:
        cactus->activeHurtBox->rect = cactus->rect;
        break;
    }
    return 0;
}

void Objects_Init(void)
{
    OBJECT defaultObject;
    defaultObject.side = HBSIDE_MIDDLE;
    defaultObject.friction = 0.9d;
    defaultObject.weight = 0.005d;
    defaultObject.velocity.x = 0.0d;
    defaultObject.velocity.y = 0.0d;
    defaultObject.combat = (OBJCOMBAT) {
        .currentCooldown = 0
    };
    defaultObject.properties = 0;
    defaultObject.ObjectHandle = DefObjectHandle;
    defaultObject.animation.currFrameTime = 100;
    defaultObject.rotationAlignmentX = 0.5d;
    defaultObject.rotationAlignmentY = 0.5d;

    OBJECT player, fallingSand, coin, cactusEnemy, bow, arrow;
    player = fallingSand = coin = cactusEnemy = bow = arrow = defaultObject;
    FRAME *frames;

    player.side = HBSIDE_LEFT;
    player.friction = 0.9d;
    player.weight = 0.005d;
    player.rect = (JGRECT2D) {
        .left   = 200.0d,
        .top    = 1.0d,
        .right  = 200.0d + 32.0d,
        .bottom = 33.0d
    };
    player.combat = (OBJCOMBAT) {
        .health = 3.0d,
        .maxHealth = 3.0d,
        .damage = 10.0d,
        .cooldown = 800,
        .currentCooldown = 0
    };
    player.properties = OBJPROP_DEFANIMATION;
    player.ObjectHandle = PlayerHandle;
    frames = malloc(sizeof(*frames) * 5);
    for(int i = 0; i < 5; i++)
    {
        (frames + i)->frameTime = 100;
        (frames + i)->framePos = (JGRECT) {
            .left   = i * 32,
            .top    = 160,
            .right  = (i + 1) * 32,
            .bottom = 160 + 32
        };
    }
    player.animation.animationCount = 1;
    player.animation.animations = malloc(sizeof(*player.animation.animations));
    *(player.animation.animations) = (ANIMATION) {
        .frames = frames,
        .frameCount = 5,
        .frameLoopIndex = 0,
        .propertyMask = OBJPROP_XMOVING,
    };
    player.animation.currFrameTime = 100;
    player.animation.frameIndex = 0;
    player.animation.animationIndex = 0;
    Object_AddClass("Player", 6, &player);

    // make falling sand animation
    frames = malloc(sizeof(*frames) * 4);
    frames->frameTime = 100;
    frames->framePos = (JGRECT) {
        .left   = 208,
        .top    = 32,
        .right  = 208 + 16,
        .bottom = 32 + 32
    };
    for(int i = 1; i < 4; i++)
    {
        (frames + i)->frameTime = 100;
        (frames + i)->framePos = (JGRECT) {
            .left   = 160 + (i - 1) * 16,
            .top    = 128,
            .right  = 160 + i * 16,
            .bottom = 128 + 16
        };
    }
    fallingSand.animation = (OBJANIMATION) {
        .animationCount = 1,
        .animations = malloc(sizeof(*(fallingSand.animation.animations))),
        .currFrameTime = 100,
        .frameIndex = 0,
        .animationIndex = 0,
    };
    *(fallingSand.animation.animations) = (ANIMATION) {
        .frames = frames,
        .frameCount = 4,
        .frameLoopIndex = 1,
        .propertyMask = OBJPROP_GROUNDED,
    };

    fallingSand.side = HBSIDE_RIGHT;
    fallingSand.friction = 0.9d;
    fallingSand.weight = 0.005d;
    fallingSand.velocity.x = 0.0d;
    fallingSand.velocity.y = 0.0d;
    fallingSand.ObjectHandle = FallingSandHandle;
    fallingSand.properties = OBJPROP_DEFANIMATION;
    fallingSand.combat.damage = 0.8d;
    fallingSand.rect = (JGRECT2D) {
        .left   = 200.0d,
        .top    = 1.0d,
        .right  = 200.0d + 32.0d,
        .bottom = 33.0d
    };
    Object_AddClass("FallingSand", 11, &fallingSand);

    coin.weight = 0.0d;
    coin.friction = 0.0d;
    coin.ObjectHandle = CoinHandle;
    coin.properties = 0;
    coin.velocity.x = 0.0d;
    coin.velocity.y = 0.0d;
    frames = malloc(sizeof(*frames));
    frames->frameTime = 0;
    frames->framePos = (JGRECT) {
        .left   = 64,
        .top    = 80,
        .right  = 80,
        .bottom = 96
    };
    coin.side = HBSIDE_MIDDLE;
    coin.animation = (OBJANIMATION) {
        .animationCount = 1,
        .animations = malloc(sizeof(*(coin.animation.animations))),
        .currFrameTime = 0,
        .frameIndex = 0,
        .animationIndex = 0,
    };
    *(coin.animation.animations) = (ANIMATION) {
        .frames = frames,
        .frameCount = 1,
        .frameLoopIndex = 0,
        .propertyMask = 0,
    };
    Object_AddClass("Coin", 4, &coin);

    cactusEnemy.side = HBSIDE_RIGHT;
    cactusEnemy.weight = 0.005d;
    cactusEnemy.friction = 0.9d;
    cactusEnemy.velocity.x = 0.0d;
    cactusEnemy.velocity.y = 0.0d;
    cactusEnemy.ObjectHandle = CactusEnemyHandle;
    cactusEnemy.properties = 0;
    cactusEnemy.combat = (OBJCOMBAT) {
        .damage = 0.6d,
        .health = 30.0d,
        .maxHealth = 30.0d,
        .cooldown = 400,
        .currentCooldown = 0
    };
    frames = malloc(sizeof(*frames));
    frames->frameTime = 0;
    frames->framePos = (JGRECT) {
        .left   = 48,
        .top    = 64,
        .right  = 64,
        .bottom = 80
    };
    cactusEnemy.animation = (OBJANIMATION) {
        .animationCount = 1,
        .animations = malloc(sizeof(*(cactusEnemy.animation.animations))),
        .currFrameTime = 0,
        .frameIndex = 0,
        .animationIndex = 0,
    };
    *(cactusEnemy.animation.animations) = (ANIMATION) {
        .frames = frames,
        .frameCount = 1,
        .frameLoopIndex = 0,
        .propertyMask = 0,
    };
    Object_AddClass("CactusEnemy", 11, &cactusEnemy);

    bow.weight = 0.004d;
    bow.friction = 1.0d;
    bow.ObjectHandle = BowHandle;
    bow.combat.damage = 14.0d;
    frames = malloc(sizeof(*frames) * 4);
    frames->frameTime = 100;
    frames->framePos = (JGRECT) {
        .left   = 176,
        .top    = 208,
        .right  = 192,
        .bottom = 224
    };
    (frames + 1)->frameTime = 100;
    (frames + 1)->framePos = (JGRECT) {
        .left   = 192,
        .top    = 208,
        .right  = 208,
        .bottom = 224
    };
    (frames + 2)->frameTime = 100;
    (frames + 2)->framePos = (JGRECT) {
        .left   = 208,
        .top    = 208,
        .right  = 224,
        .bottom = 224
    };
    (frames + 3)->frameTime = 100;
    (frames + 3)->framePos = (JGRECT) {
        .left   = 192,
        .top    = 224,
        .right  = 208,
        .bottom = 240
    };
    bow.animation = (OBJANIMATION) {
        .animationCount = 1,
        .animations = malloc(sizeof(*(bow.animation.animations))),
        .currFrameTime = 100,
        .frameIndex = 0,
        .animationIndex = 0,
    };
    *(bow.animation.animations) = (ANIMATION) {
        .frames = frames,
        .frameCount = 4,
        .frameLoopIndex = 0,
        .propertyMask = 0,
    };
    Object_AddClass("Bow", 3, &bow);

    arrow.weight = 0.003d;
    arrow.friction = 0.999d;
    arrow.ObjectHandle = ArrowHandle;
    arrow.combat.damage = 20.0d;
    frames = malloc(sizeof(*frames));
    frames->frameTime = 0;
    frames->framePos = (JGRECT) {
        .left   = 176,
        .top    = 224,
        .right  = 192,
        .bottom = 240
    };
    arrow.animation = (OBJANIMATION) {
        .animationCount = 1,
        .animations = malloc(sizeof(*(arrow.animation.animations))),
        .currFrameTime = 0,
        .frameIndex = 0,
        .animationIndex = 0,
    };
    *(arrow.animation.animations) = (ANIMATION) {
        .frames = frames,
        .frameCount = 1,
        .frameLoopIndex = 0,
        .propertyMask = 0,
    };
    Object_AddClass("Arrow", 6, &arrow);
}

OBJECT *Object_CreateFromID(int id)
{
    OBJECTCLASS *classes;
    OBJECT **objs, *obj;
    classes = ObjectClasses + id;
    objs = Objects_Grow();
    obj = malloc(sizeof(*obj));
    *objs = obj;
    *obj = classes->classInfo;
    obj->ObjectHandle(obj, NULL, OBJH_CREATE, 0);
    return obj;
}

OBJECT *Object_Create(const char *className)
{
    OBJECTCLASS *classes;
    int cnt;
    OBJECT **objs, *obj;
    classes = ObjectClasses;
    cnt = ObjectClassCount;
    while(cnt--)
    {
        if(!strcmp(className, classes->className))
        {
            objs = Objects_Grow();
            obj = malloc(sizeof(*obj));
            *objs = obj;
            *obj = classes->classInfo;
            obj->ObjectHandle(obj, NULL, OBJH_CREATE, 0);
            return obj;
        }
        classes++;
    }
   // printf("Could not find %s\n", className);
    return NULL;
}
