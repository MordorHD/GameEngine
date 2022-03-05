#include "jgobj.h"

JGOBJECT JGCreateObject(string_t className, uint32_t state, JGTEXTURE texture, float x, float y, float width, float height, float rotation, float rotAlignX, float rotAlignY, float weight, float friction)
{
    JGOBJECT object = malloc(sizeof(*object));
    object->class_ = JGGetClass(className);
    object->state = state;
    object->rect = (JGRECT2D) {
        .left = x,
        .top = y,
        .right = x + width,
        .bottom = y + height
    };
    object->rotation = rotation;
    object->rotationAlignmentX = rotAlignX;
    object->rotationAlignmentY = rotAlignY;
    object->weight = weight;
    object->friction = friction;
    object->texture = texture ? texture : object->class_->texture;
    object->class_->proc((JGCONTROL) object, JGEVENT_CREATE, NULL);
    object->class_->proc((JGCONTROL) object, JGEVENT_SIZE, NULL);
    object->velocity.x = 0.0d;
    object->velocity.y = 0.0d;
    return object;
}

void JGSetObjectPos(JGOBJECT object, JGPOINT2D pos)
{
    object->right -= object->left;
    object->bottom -= object->top;
    object->left = pos.x;
    object->top = pos.y;
    object->right += object->left;
    object->bottom += object->top;
}

uint64_t JGDefObjectProc(JGOBJECT object, uint32_t msg, JGEVENT *event)
{
    JGPOINT2D camPos;
    JGPOINT2D point;
    int width, height;
    switch(msg)
    {
    case JGEVENT_REDRAW:
        JGGetCameraPos(&camPos);
        width = object->right - object->left + 1;
        height = object->bottom - object->top + 1;
        if(object->state & JGSTATE_FLIPPED)
            width = -width;
        JGDrawTexture(object->texture, object->left - camPos.x, object->top - camPos.y, width, height);
        break;
    case JGEVENT_HITTEST:
        JGGetCameraPos(&camPos);
        point = (JGPOINT2D) {
            .x = event->x + camPos.x,
            .y = event->y + camPos.y,
        };
        return JGRect2DContains(&object->rect, &point);
    case JGEVENT_SETPOS:
        object->right -= object->left;
        object->bottom -= object->top;
        object->left = ((JGPOINT*) event)->x;
        object->top = ((JGPOINT*) event)->y;
        object->right += object->left;
        object->bottom += object->top;
        break;
    case JGEVENT_SETSIZE:
        object->right = object->left + ((JGSIZE*) event)->width;
        object->bottom = object->top + ((JGSIZE*) event)->height;
        break;
    case JGEVENT_MOVE:
        object->left += event->dx;
        object->right += event->dx;
        object->top += event->dy;
        object->bottom += event->dy;
        break;
    }
    return 0;
}
