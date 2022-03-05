#include "jg.h"
#include "jgobj.h"
#include "jgutil.h"

static JGCONTROL *Controls;
static uint32_t ControlCount;
static uint32_t ControlCapacity;

static JGCONTROL TrashControl;

static uint32_t *RemoveQueue;
static uint32_t RemoveQueueIndex;
static uint32_t RemoveQueueCapacity;

JGCONTROL MouseHoverControl;
JGCONTROL MouseFocusControl;
JGCONTROL KeyboardFocusControl;

JGCONTROLSTYLE *ControlStyles;
int ControlStyleCount;
int ControlStyleCapacity;

JGCLASS **Classes;
int ClassCount;
int ClassCapacity;

JGCONTROL JGGetMouseHoverControl(void)
{
    return MouseHoverControl;
}

JGCONTROL JGGetMouseFocusControl(void)
{
    return MouseFocusControl;
}

JGCONTROL JGGetKeyboardFocusControl(void)
{
    return KeyboardFocusControl;
}

// TODO: MAKE HASHTABLE

void JGAddStyle(string_t name, const JGCONTROLSTYLE *style)
{
    int cnt = ControlStyleCount++;
    if(ControlStyleCount > ControlStyleCapacity)
    {
        ControlStyleCapacity *= 2;
        ControlStyleCapacity++;
        ControlStyles = realloc(ControlStyles, sizeof(*ControlStyles) * ControlStyleCapacity);
    }
    *(ControlStyles + cnt) = *style;
    (ControlStyles + cnt)->name = JGUtil_Createstrcpy0(name);
}

JGCONTROLSTYLE *JGGetStyle(string_t name)
{
    int cnt = ControlStyleCount;
    JGCONTROLSTYLE *styles = ControlStyles;
    while(cnt--)
    {
        if(!strcmp(name, styles->name))
            return styles;
        styles++;
    }
    return NULL;
}

void JGAddClass(const JGCLASS *class_)
{
    JGCLASS **nextClass;
    int cnt;
    JGCLASS *allocClass;
    cnt = ClassCount++;
    if(ClassCount > ClassCapacity)
    {
        ClassCapacity *= 2;
        ClassCapacity++;
        Classes = realloc(Classes, sizeof(*Classes) * ClassCapacity);
    }
    nextClass = Classes + cnt;
    *nextClass = allocClass = malloc(sizeof(*class_));
    *allocClass = *class_;
    allocClass->id = JGUtil_Createstrcpy0(class_->id);
}


JGCLASS *JGGetClass(string_t classId)
{
    int cnt;
    JGCLASS **classes;
    classes = Classes;
    cnt = ClassCount;
    while(cnt--)
    {
        if(!strcmp(classId, (*classes)->id))
            return *classes;
        classes++;
    }
    return NULL;
}

void __JGControlInit(void)
{
    JGCLASS *allocClass;
    JGCLASS **nextClass;
    JGCONTROLSTYLE style = (JGCONTROLSTYLE) {
        .name = "Default",
        .font = NULL,
        .colorText = JGWHITE,
        .colorHover = JGGRAY,
        .colorBackground = JGDKGRAY,
        .colorForeground = 0xFFBBBBBB,
    };
    TrashControl = malloc(sizeof(__JGCONTROL));
    memset(TrashControl, 0, sizeof(__JGCONTROL));
    TrashControl->class_ = malloc(sizeof(*TrashControl->class_));
    TrashControl->class_->id = JGUtil_Createstrcpy0("Trash");
    TrashControl->class_->proc = JGDefProc;
    TrashControl->class_->structSize = 0;
    TrashControl->state = JGSTYLE_INVISIBLE;

    ClassCapacity = 12;
    nextClass = Classes = malloc(sizeof(*Classes) * ClassCapacity);
    ClassCount = 8;
    allocClass = malloc(sizeof(*allocClass));
    allocClass->id = JGUtil_Createstrcpy0("Label");
    allocClass->proc = JGStaticProc;
    allocClass->structSize = sizeof(__JGSTATIC);
    allocClass->style = style;
    *nextClass = allocClass;
    nextClass++;
    allocClass = malloc(sizeof(*allocClass));
    allocClass->id = JGUtil_Createstrcpy0("Group");
    allocClass->proc = JGGroupProc;
    allocClass->structSize = sizeof(__JGGROUP);
    allocClass->style = style;
    *nextClass = allocClass;
    nextClass++;
    allocClass = malloc(sizeof(*allocClass));
    allocClass->id = JGUtil_Createstrcpy0("Button");
    allocClass->proc = JGButtonProc;
    allocClass->structSize = sizeof(__JGBUTTON);
    allocClass->style = style;
    *nextClass = allocClass;
    nextClass++;
    allocClass = malloc(sizeof(*allocClass));
    allocClass->id = JGUtil_Createstrcpy0("Window");
    allocClass->proc = JGWindowProc;
    allocClass->structSize = sizeof(__JGWINDOW);
    allocClass->style = style;
    *nextClass = allocClass;
    nextClass++;
    allocClass = malloc(sizeof(*allocClass));
    allocClass->id = JGUtil_Createstrcpy0("Slider");
    allocClass->proc = JGSliderProc;
    allocClass->structSize = sizeof(__JGSLIDER);
    allocClass->style = style;
    *nextClass = allocClass;
    nextClass++;
    allocClass = malloc(sizeof(*allocClass));
    allocClass->id = JGUtil_Createstrcpy0("Image");
    allocClass->proc = JGImageViewProc;
    allocClass->structSize = sizeof(__JGIMAGEVIEW);
    allocClass->style = style;
    *nextClass = allocClass;
    nextClass++;
    allocClass = malloc(sizeof(*allocClass));
    allocClass->id = JGUtil_Createstrcpy0("TextInput");
    allocClass->proc = JGTextInputProc;
    allocClass->structSize = sizeof(__JGTEXTINPUT);
    allocClass->style = style;
    *nextClass = allocClass;
    nextClass++;
    allocClass = malloc(sizeof(*allocClass));
    allocClass->id = JGUtil_Createstrcpy0("TextArea");
    allocClass->proc = JGTextAreaProc;
    allocClass->structSize = sizeof(__JGTEXTAREA);
    allocClass->style = style;
    *nextClass = allocClass;
    allocClass = malloc(sizeof(*allocClass));
    allocClass->id = JGUtil_Createstrcpy0("Object");
    allocClass->proc = (JGEVENTPROC) JGDefObjectProc;
    allocClass->structSize = sizeof(__JGOBJECT);
    allocClass->style = style;
    *nextClass = allocClass;
}

JGCONTROL JGCreateControl(string_t classId, string_t styleId, int state, string_t text, int x, int y, int width, int height)
{
    JGCONTROL control;
    JGCLASS *class_;
    if(!(class_ = JGGetClass(classId)))
        return NULL;
    control = malloc(class_->structSize);
    memset(control, 0, class_->structSize);
    control->class_ = class_;
    control->style = styleId ? JGGetStyle(styleId) : &class_->style;
    if(!control->style)
        control->style = &class_->style;
    control->id = styleId ? JGUtil_Createstrcpy0(styleId) : NULL;
    control->state |= state;
    control->rect = (JGRECT) {
        .left = x,
        .top = y,
        .right = x + width,
        .bottom = y + height
    };
    control->class_->proc(control, JGEVENT_CREATE, NULL);
    control->class_->proc(control, JGEVENT_SETTEXT, (JGEVENT*) text);
    control->class_->proc(control, JGEVENT_SIZE, NULL);
    return control;
}

bool JGRemoveControl(JGCONTROL control)
{
    int index, oldRqIndex;
    int cnt;
    JGCONTROL *controls;
    index = -1;
    cnt = ControlCount;
    controls = Controls;
    while(cnt--)
    {
        if(*controls == control)
        {
            index = ControlCount - cnt - 1;
            break;
        }
        controls++;
    }
    if(cnt >= 0)
    {
        //printf("Removing %d, storing inside %d, %I64d\n", index, RemoveQueueIndex, object);
        oldRqIndex = RemoveQueueIndex++;
        if(RemoveQueueIndex > RemoveQueueCapacity)
        {
            RemoveQueueCapacity *= 2;
            RemoveQueueCapacity++;
            RemoveQueue = realloc(RemoveQueue, sizeof(*RemoveQueue) * RemoveQueueCapacity);
        }
        RemoveQueue[oldRqIndex] = index;
        *controls = TrashControl;
    }
    return 1;
}

bool JGDestroyControl(JGCONTROL control)
{
    JGRemoveControl(control);
    control->class_->proc(control, JGEVENT_DESTROY, NULL);
    free(control);
    return 1;
}

bool AddControlLock;

void JGAddControl(JGCONTROL control)
{
    uint32_t cnt;
    JGCONTROL *dest;
    while(AddControlLock);
    AddControlLock = 1;
    if(RemoveQueueIndex)
    {
        dest = Controls + RemoveQueue[--RemoveQueueIndex];
    }
    else
    {
        cnt = ControlCount++;
        if(ControlCount > ControlCapacity)
        {
            ControlCapacity *= 2;
            ControlCapacity++;
            Controls = realloc(Controls, sizeof(*Controls) * ControlCapacity);
        }
        dest = Controls + cnt;
    }
    *dest = control;
    AddControlLock = 0;
}

JGCONTROL LockedParent;

void JGAddChildControl(JGCONTROL parent, JGCONTROL child)
{
    while(LockedParent == parent);
    LockedParent = parent;
    parent->class_->proc(parent, JGEVENT_ADDCHILD, (JGEVENT*) child);
    if(parent->state & JGSTYLE_INVISIBLE)
        child->state |= JGSTYLE_INVISIBLE;
    JGAddControl(child);
    LockedParent = NULL;
}

JGCONTROL JGGetControl(const char *id)
{
    JGCONTROL control;
    int cnt = ControlCount;
    while(cnt--)
    {
        control = *(Controls + cnt);
        if(control && control->id)
        if(!strcmp(control->id, id))
            return control;
    }
    return NULL;
}

bool JGSetBounds(JGCONTROL control, const JGRECT *rect)
{
    int dx = rect->left - control->left;
    int dy = rect->top - control->top;
    bool r = JGSetSize(control, (JGSIZE) { rect->right - rect->left, rect->bottom - rect->top });
    if(!(dx | dy) && !r)
        return 0;
    JGMoveControl(control, dx, dy);
    return 1;
}

bool JGSetPos(JGCONTROL control, JGPOINT point)
{
    int dx = point.x - control->left;
    int dy = point.y - control->top;
    if(!(dx | dy))
        return 0;
    JGMoveControl(control, dx, dy);
    return 1;
}

bool JGSetSize(JGCONTROL control, JGSIZE size)
{
    control->class_->proc(control, JGEVENT_SETSIZE, (JGEVENT*) &size);
    control->class_->proc(control, JGEVENT_SIZE, NULL);
    return 1;
}

void JGMoveControl(JGCONTROL control, int32_t dx, int32_t dy)
{
    JGEVENT event;
    event.dx = dx;
    event.dy = dy;
    control->class_->proc(control, JGEVENT_MOVE, &event);
}

void JGRequestFocus(JGCONTROL control)
{
    if(KeyboardFocusControl)
        JGKillFocus(KeyboardFocusControl);
    KeyboardFocusControl = control;
    KeyboardFocusControl->class_->proc(KeyboardFocusControl, JGEVENT_FOCUSGAINED, NULL);
}

void JGKillFocus(JGCONTROL control)
{
    if(KeyboardFocusControl == control)
    {
        KeyboardFocusControl->class_->proc(KeyboardFocusControl, JGEVENT_FOCUSLOST, NULL);
        KeyboardFocusControl = NULL;
    }
}

uint64_t JGDispatchEvent(uint32_t msg, JGEVENT *event)
{
    int i;
    int cnt;
    JGCONTROL control;
    switch(msg)
    {
    case JGEVENT_SETCURSOR:
        if(MouseHoverControl)
            MouseHoverControl->class_->proc(MouseHoverControl, msg, event);
        return 0;
    case JGEVENT_MOUSEMOVED:
        if(MouseFocusControl)
        {
            if(MouseHoverControl && !MouseHoverControl->class_->proc(MouseHoverControl, JGEVENT_HITTEST, event))
                MouseHoverControl = NULL;
            else if(!MouseHoverControl && MouseFocusControl->class_->proc(MouseFocusControl, JGEVENT_HITTEST, event))
                MouseHoverControl = MouseFocusControl;
            return MouseFocusControl->class_->proc(MouseFocusControl, JGEVENT_MOUSEDRAGGED, event);
        }
        for(i = 0; i < ControlCount; i++)
        {
            control = *(Controls + i);
            if(!(control->state & (JGSTYLE_TRANSPARENT | JGSTYLE_INVISIBLE)) && control->class_->proc(control, JGEVENT_HITTEST, event))
                break;
            control = NULL;
        }
        //printf("HOVERED: %d\n", control);
        if(MouseHoverControl)
        {
            if(MouseHoverControl != control)
            {
                MouseHoverControl->class_->proc(MouseHoverControl, JGEVENT_LEAVE, event);
            }
            else
            {
                return MouseHoverControl->class_->proc(MouseHoverControl, JGEVENT_MOUSEMOVED, event);
            }
        }
        if(i < ControlCount)
        {
            MouseHoverControl = control;
            MouseHoverControl->class_->proc(MouseHoverControl, JGEVENT_HOVER, event);
            MouseHoverControl->class_->proc(MouseHoverControl, JGEVENT_MOUSEMOVED, event);
        }
        else
            MouseHoverControl = NULL;
        return 0;
    case JGEVENT_MOUSERELEASED:
        if(MouseFocusControl)
        {
            i = MouseFocusControl->class_->proc(MouseFocusControl, JGEVENT_MOUSERELEASED, event);
            MouseFocusControl = NULL;
            return i;
        }
        return 0;
    case JGEVENT_MOUSEPRESSED:
        if(MouseHoverControl)
            MouseFocusControl = MouseHoverControl;
    case JGEVENT_MOUSEWHEEL:
        return MouseHoverControl ? MouseHoverControl->class_->proc(MouseHoverControl, msg, event) : 0;
    case JGEVENT_KEYPRESSED:
    case JGEVENT_KEYRELEASED:
    case JGEVENT_KEYTYPED:
        return KeyboardFocusControl ? KeyboardFocusControl->class_->proc(KeyboardFocusControl, msg, event) : 0;
    }
    cnt = ControlCount;
    if(msg == JGEVENT_REDRAW)
    {
        while(cnt--)
        {
            control = *(Controls + cnt);
            if(!(control->state & JGSTYLE_INVISIBLE))
                control->class_->proc(control, JGEVENT_REDRAW, event);
        }
    }
    else
    {
        while(cnt--)
        {
            control = *(Controls + (ControlCount - 1 - cnt));
            if(!(control->state & JGSTYLE_INVISIBLE))
                control->class_->proc(control, msg, event);
        }
    }
    return 0;
}

uint64_t JGDefProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    switch(msg)
    {
    case JGEVENT_HITTEST:
        return JGRectContains(&control->rect, &event->mousePos);
    case JGEVENT_SETPOS:
        control->right -= control->left;
        control->bottom -= control->top;
        control->left = ((JGPOINT*) event)->x;
        control->top = ((JGPOINT*) event)->y;
        control->right += control->left;
        control->bottom += control->top;
        break;
    case JGEVENT_SETSIZE:
        control->right = control->left + ((JGSIZE*) event)->width;
        control->bottom = control->top + ((JGSIZE*) event)->height;
        break;
    case JGEVENT_MOVE:
        control->left += event->dx;
        control->right += event->dx;
        control->top += event->dy;
        control->bottom += event->dy;
        break;
    }
    return 0;
}

uint64_t JGGroupProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGGROUP group = (JGGROUP) control;
    int cnt;
    switch(msg)
    {
    case JGEVENT_DESTROY:
        cnt = group->childrenCount;
        while(cnt--)
            JGDestroyControl(*(group->children + cnt));
        return 0;
    case JGEVENT_SETLAYOUT:
        return !JGGetStockLayout(&group->layout, (int) (uintptr_t) event);
    case JGEVENT_GETLAYOUT:
        return (uint64_t) &group->layout;
    case JGEVENT_ADDCHILD:
        cnt = group->childrenCount++;
        if(group->childrenCount > group->childrenCapacity)
        {
            group->childrenCapacity *= 2;
            group->childrenCapacity++;
            group->children = realloc(group->children, sizeof(*group->children) * group->childrenCapacity);
        }
        *(group->children + cnt) = (JGCONTROL) event;
        return 0;
    case JGEVENT_SETCURSOR:
        JGSetCursor(JGLoadCursor(JGCURSOR_DEFAULT));
        return 0;
    case JGEVENT_MOVE:
        cnt = group->childrenCount;
        while(cnt--)
            JGMoveControl(*(group->children + cnt), event->dx, event->dy);
        break;
    case JGEVENT_POS:
    case JGEVENT_SIZE:
        if(group->layout.type)
            group->layout.layoutFunc(&group->layout, group->children, group->childrenCount, &control->rect);
        return 0;
    case JGEVENT_SHOW:
        if(control->state & JGSTYLE_INVISIBLE)
        {
            cnt = group->childrenCount;
            while(cnt--)
                JGShowControl(*(group->children + cnt));
        }
        return 0;
    case JGEVENT_HIDE:
        if(!(control->state & JGSTYLE_INVISIBLE))
        {
            cnt = group->childrenCount;
            while(cnt--)
                JGHideControl(*(group->children + cnt));
        }
        return 0;
    }
    return JGDefProc(control, msg, event);
}

uint64_t JGWindowProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGWINDOW window = (JGWINDOW) control;
    JGTEXT text = window->text;
    uint32_t state = control->state;
    JGRECT rect;
    //int ccnt;
    _Bool isSize;
    int cnt;
    switch(msg)
    {
    case JGEVENT_DESTROY:
        cnt = window->childrenCount;
        while(cnt--)
            JGDestroyControl(*(window->children + cnt));
        return 0;
    case JGEVENT_SETLAYOUT:
        return !JGGetStockLayout(&window->layout, (int) (uintptr_t) event);
    case JGEVENT_GETLAYOUT:
        return (uint64_t) &window->layout;
    case JGEVENT_SETTEXT:
        JGText_Set0(&window->text, (string_t) event);
        return 0;
    case JGEVENT_ADDCHILD:
        cnt = window->childrenCount++;
        if(window->childrenCount > window->childrenCapacity)
        {
            window->childrenCapacity *= 2;
            window->childrenCapacity++;
            window->children = realloc(window->children, sizeof(*window->children) * window->childrenCapacity);
        }
        *(window->children + cnt) = (JGCONTROL) event;
        return 0;
    case JGEVENT_REDRAW:
        JGNoStroke();
        JGFill(control->style->colorBackground);
        JGRect(control->left, control->top, control->right, control->bottom);
        if(!(state & JGSTYLE_BORDERLESS))
        {
            JGFill(control->style->colorForeground);
            JGTextColor(control->style->colorText);
            // top border
            JGRect(control->left, control->top, control->right, control->top + 30);
            // left border
            JGRect(control->left, control->top + 30, control->left + 8, control->bottom);
            // right border
            JGRect(control->right - 8, control->top + 30, control->right, control->bottom);
            // bottom border
            JGRect(control->left, control->bottom - 8, control->right, control->bottom);
            JGSetFont(control->style->font);
            JGTextAlign(JGALIGN_LEFT | JGALIGN_TOP);
            JGText(text.text, text.length, control->left + 8, control->top + 8);
            if(state & JGSTYLE_XBUTTON)
            {
                if(state & JGSTATE_MOUSEINBUTTON)
                {
                    if(state & JGSTATE_BUTTONPRESSED)
                        JGFill(control->style->colorHover * 2);
                    else
                        JGFill(control->style->colorHover / 2);
                }
                else
                    JGFill(control->style->colorHover);
                JGRect(control->right - 24, control->top + 8, control->right - 8, control->top + 24);
                JGStroke(control->style->colorText);
                JGLine(control->right - 22, control->top + 10, control->right - 10, control->top + 22);
                JGLine(control->right - 10, control->top + 10, control->right - 22, control->top + 22);
            }
        }
        return 0;
    case JGEVENT_MOVE:
        cnt = window->childrenCount;
        while(cnt--)
            JGMoveControl(*(window->children + cnt), event->dx, event->dy);
        break;
    case JGEVENT_POS:
    case JGEVENT_SIZE:
        rect = (state & JGSTYLE_BORDERLESS) ? control->rect : (JGRECT) {
            .left   = control->left + 9,
            .top    = control->top + 30,
            .right  = control->right - 7,
            .bottom = control->bottom - 8,
        };
        if(window->layout.type)
            window->layout.layoutFunc(&window->layout, window->children, window->childrenCount, &rect);
        return 0;
    case JGEVENT_SHOW:
        if(state & JGSTYLE_INVISIBLE)
        {
            cnt = window->childrenCount;
            while(cnt--)
                JGShowControl(*(window->children + cnt));
        }
        return 0;
    case JGEVENT_HIDE:
        if(!(state & JGSTYLE_INVISIBLE))
        {
            cnt = window->childrenCount;
            while(cnt--)
                JGHideControl(*(window->children + cnt));
        }
        return 0;
    case JGEVENT_SETCURSOR:
        switch(state & (JGSTATE_MOUSEINCLIENT | JGSTATE_MOUSEINBUTTON | JGSTATE_MOVE | JGSTATE_RESIZE_LEFT | JGSTATE_RESIZE_TOP | JGSTATE_RESIZE_RIGHT | JGSTATE_RESIZE_BOTTOM))
        {
        case JGSTATE_MOUSEINCLIENT:
            JGSetCursor(JGLoadCursor(JGCURSOR_DEFAULT));
            break;
        case JGSTATE_MOUSEINBUTTON:
            JGSetCursor(JGLoadCursor(JGCURSOR_HAND));
            break;
        case JGSTATE_MOVE:
            JGSetCursor(JGLoadCursor(JGCURSOR_MOVE));
            break;
        case JGSTATE_RESIZE_RIGHT:
        case JGSTATE_RESIZE_LEFT:
            JGSetCursor(JGLoadCursor(JGCURSOR_LR));
            break;
        case JGSTATE_RESIZE_TOP:
        case JGSTATE_RESIZE_BOTTOM:
            JGSetCursor(JGLoadCursor(JGCURSOR_UD));
            break;
        case JGSTATE_RESIZE_RIGHT | JGSTATE_RESIZE_TOP:
        case JGSTATE_RESIZE_LEFT | JGSTATE_RESIZE_BOTTOM:
            JGSetCursor(JGLoadCursor(JGCURSOR_LD_RU));
            break;
        case JGSTATE_RESIZE_LEFT | JGSTATE_RESIZE_TOP:
        case JGSTATE_RESIZE_RIGHT | JGSTATE_RESIZE_BOTTOM:
            JGSetCursor(JGLoadCursor(JGCURSOR_LU_RD));
            break;
        }
        return 0;
    case JGEVENT_MOUSEDRAGGED:
        if(state & (JGSTATE_MOUSEINBUTTON | JGSTATE_BUTTONPRESSED))
        {
            rect = (JGRECT) {
                .left   = control->right - 24,
                .top    = control->top + 7,
                .right  = control->right - 7,
                .bottom = control->top + 24,
            };
            if(!JGRectContains(&rect, &event->mousePos))
                control->state &= ~JGSTATE_MOUSEINBUTTON;
            else
                control->state |= JGSTATE_MOUSEINBUTTON;
        }
        if(state & JGSTATE_MOVE)
            JGMoveControl(control, event->dx, event->dy);
        else
        {
            if(state & JGSTATE_RESIZE_LEFT)
                control->left = __min(control->right - 30, event->x - 4);
            else if(state & JGSTATE_RESIZE_RIGHT)
                control->right = __max(control->left + 30, event->x + 4);
            if(state & JGSTATE_RESIZE_TOP)
                control->top = __min(control->bottom - 30, event->y - 4);
            else if(state & JGSTATE_RESIZE_BOTTOM)
                control->bottom = __max(control->top + 30, event->y + 4);
            JGRelayout(control);
        }
        return 0;
    case JGEVENT_MOUSEMOVED:
        control->state &= ~(JGSTATE_MOUSEINCLIENT | JGSTATE_MOUSEINBUTTON | JGSTATE_MOVE | JGSTATE_RESIZE_LEFT | JGSTATE_RESIZE_TOP | JGSTATE_RESIZE_RIGHT | JGSTATE_RESIZE_BOTTOM);
        if(control->state & JGSTYLE_XBUTTON)
        {
            rect = (JGRECT) {
                .left   = control->right - 24,
                .top    = control->top + 7,
                .right  = control->right - 7,
                .bottom = control->top + 24,
            };
            if(JGRectContains(&rect, &event->mousePos))
            {
                control->state |= JGSTATE_MOUSEINBUTTON;
                return 0;
            }
        }
        isSize = (control->state & JGSTYLE_RESIZE) && (event->x < control->left + 8 || event->x > control->right - 8 || event->y < control->top + 8 || event->y > control->bottom - 8);
        if(isSize)
        {
            if(event->x < control->left + 8)
                control->state |= JGSTATE_RESIZE_LEFT;
            else if(event->x > control->right - 8)
                control->state |= JGSTATE_RESIZE_RIGHT;
            if(event->y < control->top + 8)
                control->state |= JGSTATE_RESIZE_TOP;
            else if(event->y > control->bottom - 8)
                control->state |= JGSTATE_RESIZE_BOTTOM;
        }
        else
        {
            if(event->y < control->top + 30)
                control->state |= JGSTATE_MOVE;
            else
                control->state |= JGSTATE_MOUSEINCLIENT;
        }
        return 0;
    case JGEVENT_MOUSEPRESSED:
        // ccnt = ControlCount - 1;
        // move window to top
        if(control->state & JGSTATE_MOUSEINBUTTON)
            control->state |= JGSTATE_BUTTONPRESSED;
		return 0;
	case JGEVENT_MOUSERELEASED:
	    if((control->state & (JGSTATE_BUTTONPRESSED | JGSTATE_MOUSEINBUTTON)) == (JGSTATE_BUTTONPRESSED | JGSTATE_MOUSEINBUTTON))
        {
            // TODO: Destroy window
            // printf("DESTROYING WINDOW\n");
            if(!control->class_->proc(control, JGEVENT_CLOSE, event))
                JGDestroyControl(control);
            return 0;
        }
        control->state &= ~JGSTATE_BUTTONPRESSED;
	    break;
    case JGEVENT_LEAVE:
        control->state &= ~(JGSTATE_MOUSEINCLIENT | JGSTATE_MOUSEINBUTTON | JGSTATE_MOVE | JGSTATE_RESIZE_LEFT | JGSTATE_RESIZE_TOP | JGSTATE_RESIZE_RIGHT | JGSTATE_RESIZE_BOTTOM);
        break;
    }
    return JGDefProc(control, msg, event);
}

uint64_t JGSliderProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGTEXT text = ((JGSLIDER) control)->text;
    JGRECT rect;
    uint32_t state = control->state;
    int sp = ((JGSLIDER) control)->sliderPos;
    switch(msg)
    {
    case JGEVENT_SETTEXT:
        JGText_Set0(&((JGSLIDER) control)->text, (string_t) event);
        return 0;
    case JGEVENT_SETCURSOR:
        JGSetCursor(JGLoadCursor(JGCURSOR_DEFAULT));
        return 0;
    case JGEVENT_REDRAW:
        JGNoStroke();
        JGFill(control->style->colorBackground);
        JGRect(control->left, control->top, control->right, control->bottom);
        JGFill(control->style->colorForeground);
        JGRect(control->left + sp, control->top, control->left + sp + 15, control->bottom);
        if(text.text)
        {
            JGSetFont(control->style->font);
            JGTextAlign(JGALIGN_CENTER);
            JGTextColor(control->style->colorText);
            JGText(text.text, text.length, (control->left + control->right) / 2, (control->top + control->bottom) / 2);
        }
        return 0;
    case JGEVENT_MOUSEPRESSED:
        if(control->state & JGSTATE_SLIDERIN)
        {
            control->state |= JGSTATE_SLIDERPRESSED;
        }
        else
        {
            sp += (event->mousePos.x < control->left + sp + 7 ? -1 : 1) * (control->right - control->left) / 5;
            sp = __max(sp, 0);
            sp = __min(sp, (control->right - control->left) - 15);
            ((JGSLIDER) control)->sliderPos = sp;
        }
        return JGCONSUME;
    case JGEVENT_MOUSEDRAGGED:
        if(event->mousePos.x > control->left && event->mousePos.x < control->right)
        {
            sp += event->dx;
            sp = __max(sp, 0);
            sp = __min(sp, control->right - control->left - 15);
            ((JGSLIDER) control)->sliderPos = sp;
        }
        return 0;
    case JGEVENT_MOUSEMOVED:
        rect = control->rect;
        rect.left += sp;
        rect.right = rect.left + 15;
        if(!(state & JGSTATE_SLIDERIN))
        {
            if(JGRectContains(&rect, &event->mousePos))
                control->state |= JGSTATE_SLIDERIN;
        }
        else if(!JGRectContains(&rect, &event->mousePos))
            control->state &= ~JGSTATE_SLIDERIN;
        break;
    case JGEVENT_SIZE:
        //control->sliderPos *= (control->right - control->left) / (!event->oldSizeX ? (slider->maxValue - slider->minValue) : event->oldSizeX);
        break;
    }
    return JGDefProc(control, msg, event);
}

uint64_t JGImageViewProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGIMAGE image = ((JGIMAGEVIEW) control)->image;
    JGRECT cut = ((JGIMAGEVIEW) control)->cut;
    switch(msg)
    {
    case JGEVENT_SETIMAGE:
        ((JGIMAGEVIEW) control)->image = (JGIMAGE) event;
        return 0;
    case JGEVENT_SETCURSOR:
        JGSetCursor(JGLoadCursor(JGCURSOR_DEFAULT));
        return 0;
    case JGEVENT_REDRAW:
        if(image)
        {
            if(cut.left >= cut.right)
                JGDrawImage(image, control->left, control->top, control->right - control->left, control->bottom - control->top);
            else
                JGDrawImageSection(image, cut.left, cut.top, cut.right - cut.left, cut.bottom - cut.top, control->left, control->top, control->right - control->left, control->bottom - control->top);
        }
        return 0;
    }
    return JGDefProc(control, msg, event);
}

uint64_t JGStaticProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGTEXT text = ((JGSTATIC) control)->text;
    uint32_t state = control->state;
    switch(msg)
    {
    case JGEVENT_SETTEXT:
        JGText_Set0(&((JGSTATIC) control)->text, (string_t) event);
        return 0;
    case JGEVENT_SETCURSOR:
        JGSetCursor(JGLoadCursor(JGCURSOR_DEFAULT));
        return 0;
    case JGEVENT_REDRAW:
        JGNoStroke();
        if(!(state & JGSTYLE_NOTDRAWBG))
        {
            JGFill(control->style->colorBackground);
            JGRect(control->left, control->top, control->right, control->bottom);
        }
        if(text.text)
        {
            JGSetFont(control->style->font);
            JGTextAlign(JGALIGN_CENTER | 24);
            JGTextColor(control->style->colorText);
            JGText(text.text, text.length, (control->left + control->right) / 2, (control->top + control->bottom + 7) / 2);
        }
        return 0;
    }
    return JGDefProc(control, msg, event);
}

uint64_t JGButtonProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGTEXT text = ((JGBUTTON) control)->text;
    uint32_t state = control->state;
    JGCONTROLHANDLE handle;
    switch(msg)
    {
    case JGEVENT_SETTEXT:
        JGText_Set0(&((JGBUTTON) control)->text, (string_t) event);
        return 0;
    case JGEVENT_SETCURSOR:
        JGSetCursor(JGLoadCursor(JGCURSOR_DEFAULT));
        return 0;
    case JGEVENT_SETHANDLE:
        ((JGBUTTON) control)->handle = (JGCONTROLHANDLE) event;
        return 0;
    case JGEVENT_REDRAW:
        JGNoStroke();
        if(!(state & JGSTYLE_NOTDRAWBG))
        {
            if(MouseHoverControl == control)
                JGFill(MouseFocusControl ? control->style->colorForeground : control->style->colorHover);
            else
                JGFill(control->style->colorBackground);
            JGRect(control->left, control->top, control->right, control->bottom);
        }
        if(text.text)
        {
            JGSetFont(control->style->font);
            JGTextAlign(JGALIGN_CENTER | 24);
            JGTextColor(control->style->colorText);
            JGText(text.text, text.length, (control->left + control->right) / 2, (control->top + control->bottom + 7) / 2);
        }
        return 0;
    case JGEVENT_MOUSERELEASED:
        if(MouseHoverControl && (handle = ((JGBUTTON) control)->handle))
            handle(control, JGHANDLE_TOGGLED, event);
        break;
    }
    return JGDefProc(control, msg, event);
}

static inline char *FindSR(char *text, int len)
{
    _Bool cat, cat2;
    if(!len)
        return text;
    cat = isalnum(*text) || *text == '_' ? 0 : 1;
    len--;
    text--;
    if(!len)
        return text;
    while(len--)
    {
        cat2 = isalnum(*text) || *text == '_' ? 0 : 1;
        if(cat != cat2)
            return text;
        text--;
    }
    return text;
}

static inline char *FindS(char *text, int len)
{
    _Bool cat, cat2;
    if(!len)
        return text;
    cat = isalnum(*text) || *text == '_' ? 0 : 1;
    len--;
    text++;
    if(!len)
        return text;
    while(len--)
    {
        cat2 = isalnum(*text) || *text == '_' ? 0 : 1;
        if(cat != cat2)
            return text;
        text++;
    }
    return text;
}

uint64_t JGTextInputProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGTEXTINPUT textInput = (JGTEXTINPUT) control;
    JGTEXT *text = &textInput->text;
    uint32_t state = control->state;
    int textWidth, textHeight;
    int textY;
    JGCONTROLHANDLE handle;
    char *txt;
    int len;
    int x, xe;
    int newCaretPos;
    switch(msg)
    {
    case JGEVENT_SETTEXT:
        JGText_Set0(text, (string_t) event);
        textInput->caretPos = text->length;
        textInput->selectPos = textInput->caretPos;
        return 0;
    case JGEVENT_SETHANDLE:
        textInput->handle = (JGCONTROLHANDLE) event;
        return 0;
    case JGEVENT_REDRAW:
        JGNoStroke();
        if(!(state & JGSTYLE_NOTDRAWBG))
        {
            JGFill(control->style->colorBackground);
            JGRect(control->left, control->top, control->right, control->bottom);
        }
        if(text->text)
        {
            JGClipRect(&control->rect);
            JGSetFont(control->style->font);
            JGTextColor(control->style->colorText);
            // TODO: ?
            textHeight = JGTextHeight();
            textY = (control->top + control->bottom - textHeight) / 2;
            if(control->state & JGSTATE_SELECTED)
            {
                JGFill(0xAA4433);
                if(textInput->caretPos < textInput->selectPos)
                {
                    x = textInput->caretPos;
                    xe = textInput->selectPos;
                }
                else
                {
                    x = textInput->selectPos;
                    xe = textInput->caretPos;
                }
                xe = JGTextWidth(text->text + x, xe - x);
                textWidth = JGTextWidth(text->text + x, text->length - x);
                x = JGTextWidth(text->text, x);
                textWidth += x;
                if(control->state & JGSTYLE_LEFT)
                    x += control->left;
                else if(control->state & JGSTYLE_RIGHT)
                    x += control->right - textWidth;
                else
                    x += (control->left + control->right - textWidth) / 2;
                xe += x;
                JGRect(x, textY, xe, textY + textHeight);
            }
            if(control->state & JGSTYLE_LEFT)
            {
                JGTextAlign(JGALIGN_LEFT);
                JGText(text->text, text->length, control->left, textY);
            }
            else if(control->state & JGSTYLE_RIGHT)
            {
                JGTextAlign(JGALIGN_RIGHT);
                JGText(text->text, text->length, control->right, textY);
            }
            else
            {
                JGTextAlign(JGALIGN_CENTER);
                JGText(text->text, text->length, (control->left + control->right) / 2, textY);
            }
            if(JGGetKeyboardFocusControl() == control)
            {
                x = JGTextWidth(text->text, textInput->caretPos);
                textWidth = x + JGTextWidth(text->text + textInput->caretPos, text->length - textInput->caretPos);
                if(control->state & JGSTYLE_LEFT)
                    x += control->left;
                else if(control->state & JGSTYLE_RIGHT)
                    x += control->right - textWidth;
                else
                    x += (control->left + control->right - textWidth) / 2;
                JGFill(control->style->colorForeground);
                JGRect(x, textY, x + 2, textY + textHeight);
            }
            JGClipRect(NULL);
        }
        return 0;
    case JGEVENT_SETCURSOR:
        JGSetCursor(JGLoadCursor(JGCURSOR_CARET));
        return 0;
    case JGEVENT_MOUSEPRESSED:
        JGRequestFocus(control);
        txt = text->text;
        len = text->length;
        if(control->state & JGSTYLE_LEFT)
            x = control->left;
        else if(control->state & JGSTYLE_RIGHT)
        {
            textWidth = JGTextWidth(text->text, text->length);
            x = control->right - textWidth;
        }
        else
        {
            textWidth = JGTextWidth(text->text, text->length);
            x = (control->left + control->right - textWidth) / 2;
        }
        while(len--)
        {
            textWidth = JGTextWidth(txt, 1);
            if(event->x < x + textWidth / 2)
                break;
            x += textWidth;
            txt++;
        }
        textInput->caretPos = textInput->selectPos = text->length - len - 1;
        control->state |= JGSTATE_SELECTED;
        return 0;
    case JGEVENT_MOUSEDRAGGED:
        txt = text->text;
        len = text->length;
        if(control->state & JGSTYLE_LEFT)
            x = control->left;
        else if(control->state & JGSTYLE_RIGHT)
        {
            textWidth = JGTextWidth(text->text, text->length);
            x = control->right - textWidth;
        }
        else
        {
            textWidth = JGTextWidth(text->text, text->length);
            x = (control->left + control->right - textWidth) / 2;
        }
        while(len--)
        {
            textWidth = JGTextWidth(txt, 1);
            if(event->x < x + textWidth / 2)
                break;
            x += textWidth;
            txt++;
        }
        textInput->caretPos = text->length - len - 1;
        return 0;
    case JGEVENT_KEYPRESSED:
        switch(event->vkCode)
        {
        case VK_BACK:
            if(!textInput->caretPos)
                break;
            if(control->state & JGSTATE_SELECTED)
            {
                goto rem_selection;
            }
            else if(JGGetKeyState(VK_CONTROL) & 0x8000)
            {
                txt = FindSR(text->text + textInput->caretPos - 1, textInput->caretPos - 1);
                JGText_RemoveRangeAt(text, txt - text->text, text->text + textInput->caretPos - txt);
                textInput->caretPos = txt - text->text;
                goto text_changed;
            }
            else
            {
                textInput->caretPos--;
                JGText_RemoveCharAt(text, textInput->caretPos);
                goto text_changed;
            }
            break;
        case VK_DELETE:
            if(textInput->caretPos == text->length)
                break;
            if(control->state & JGSTATE_SELECTED)
            {
                goto rem_selection;
            }
            else if(JGGetKeyState(VK_CONTROL) & 0x8000)
            {
                txt = FindS(text->text + textInput->caretPos + 1, text->length - textInput->caretPos - 1);
                JGText_RemoveRangeAt(text, textInput->caretPos, txt - text->text - textInput->caretPos);
                goto text_changed;
            }
            else
            {
                JGText_RemoveCharAt(text, textInput->caretPos);
                goto text_changed;
            }
            break;
        case VK_LEFT:
            if(!textInput->caretPos)
                break;
            if(JGGetKeyState(VK_CONTROL) & 0x8000)
            {
                txt = FindSR(text->text + textInput->caretPos - 1, textInput->caretPos - 1);
                newCaretPos = txt - text->text;
                goto rm_select;
            }
            else
            {
                newCaretPos = textInput->caretPos - 1;
                goto rm_select;
            }
            break;
        case VK_RIGHT:
            if(textInput->caretPos == text->length)
                break;
            if(JGGetKeyState(VK_CONTROL) & 0x8000)
            {
                txt = FindS(text->text + textInput->caretPos + 1, text->length - textInput->caretPos - 1);
                newCaretPos = txt - text->text;
                goto rm_select;
            }
            else
            {
                newCaretPos = textInput->caretPos + 1;
                goto rm_select;
            }
            break;
        case VK_HOME:
            newCaretPos = 0;
            goto rm_select;
        case VK_END:
            newCaretPos = text->length;
            goto rm_select;
        case 'A':
            if(JGGetKeyState(VK_CONTROL) & 0x8000)
            {
                textInput->selectPos = 0;
                textInput->caretPos = text->length;
                control->state |= JGSTATE_SELECTED;
            }
            break;
        case 'C':
        case 'V':
        case 'X':
            // TODO: ADD COPY PASTE
            // ...
            break;
        }
        return 0;
    case JGEVENT_KEYTYPED:
        if(event->vkCode < ' ' || event->vkCode >= 127)
            return 0;
        if(control->state & JGSTATE_SELECTED)
        {
            if(textInput->caretPos < textInput->selectPos)
            {
                x = textInput->caretPos;
                xe = textInput->selectPos;
            }
            else
            {
                x = textInput->selectPos;
                xe = textInput->caretPos;
            }
            JGText_RemoveRangeAt(text, x, xe - x);
            control->state &= ~JGSTATE_SELECTED;
            textInput->caretPos = x;
        }
        JGText_InsertChar(text, event->keyChar, textInput->caretPos);
        textInput->caretPos++;
        goto text_changed;
    }
    return JGDefProc(control, msg, event);
    rm_select:
        if(!(JGGetKeyState(VK_SHIFT) & 0x8000))
            control->state &= ~JGSTATE_SELECTED;
        else if(!(control->state & JGSTATE_SELECTED))
        {
            control->state |= JGSTATE_SELECTED;
            textInput->selectPos = textInput->caretPos;
        }
        textInput->caretPos = newCaretPos;
        return 0;
    rem_selection:
        if(textInput->caretPos < textInput->selectPos)
        {
            x = textInput->caretPos;
            xe = textInput->selectPos;
        }
        else
        {
            x = textInput->selectPos;
            xe = textInput->caretPos;
        }
        JGText_RemoveRangeAt(text, x, xe - x);
        control->state &= ~JGSTATE_SELECTED;
        textInput->caretPos = x;
    text_changed:
        if((handle = textInput->handle))
            handle(control, JGHANDLE_TEXT, NULL);
        return 0;
}

void _JGTATranslate(JGCONTROL control, JGTEXT *line, int px, int yIndex, int *x, int *y)
{
    int textWidth;
    char *txt;
    int len;
    txt = line->text;
    len = line->length;
    while(len--)
    {
        textWidth = JGTextWidth(txt, 1);
        if(px < textWidth / 2)
            break;
        px -= textWidth;
        txt++;
    }
    *x = line->length - len - 1;
    *y = yIndex;
}

_Bool _JGTranslateMouse(JGCONTROL control, int mx, int my, int *x, int *y)
{
    int lineHeight;
    int yIndex;
    int left;
    JGTEXT *text;
    JGLIST *lines = &(((JGTEXTAREA) control)->lines);
    lineHeight = JGTextHeight();
    yIndex = my;
    if(control->state & JGSTYLE_BOTTOM)
        yIndex -= control->bottom - lineHeight * lines->count;
    else if(!(control->state & JGSTYLE_TOP))
        yIndex -= (control->bottom + control->top - lineHeight * lines->count) / 2;
    else
        yIndex -= control->top;
    yIndex /= lineHeight;
    if(yIndex < 0 || yIndex >= lines->count)
        return 0;
    text = JGList_Get(lines, yIndex);
    if(control->state & JGSTYLE_LEFT)
        left = control->left;
    else if(control->state & JGSTYLE_RIGHT)
        left = control->right - JGTextWidth(text->text, text->length);
    else
        left = (control->left + control->right - JGTextWidth(text->text, text->length)) / 2;
    mx -= left;
    _JGTATranslate(control, text, mx, yIndex, x, y);
    return 1;
}

void _JGRemoveSelection(JGTEXTAREA textArea)
{
    int x, xe, y, ye;
    JGTEXT *t1, *t2;
    JGLIST *lines = &textArea->lines;
    if(textArea->caretY < textArea->selectY)
    {
        y = textArea->caretY;
        ye = textArea->selectY;
        x = textArea->caretX;
        xe = textArea->selectX;
    }
    else if(textArea->caretY > textArea->selectY)
    {
        y = textArea->selectY;
        ye = textArea->caretY;
        x = textArea->selectX;
        xe = textArea->caretX;
    }
    else
    {
        y = ye = textArea->selectY;
        if(textArea->caretX < textArea->selectX)
        {
            x = textArea->caretX;
            xe = textArea->selectX;
        }
        else
        {
            x = textArea->selectX;
            xe = textArea->caretX;
        }
    }
    t1 = JGList_Get(lines, y);
    if(y == ye)
    {
        JGText_RemoveRangeAt(t1, x, xe - x);
    }
    else
    {
        t2 = JGList_Get(lines, ye);
        JGText_RemoveRangeAt(t1, x, t1->length - x);
        JGText_RemoveRangeAt(t2, 0, xe);
        JGText_AppendString(t1, t2->text, t2->length);
        JGList_RemoveElemsAtIndex(lines, y + 1, ye - y);
    }
    textArea->control.state &= ~JGSTATE_SELECTED;
    textArea->caretX = x;
    textArea->caretY = y;
}

uint64_t JGTextAreaProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGTEXTAREA textArea = (JGTEXTAREA) control;
    JGLIST *lines = &textArea->lines;
    JGTEXT line;
    JGTEXT *text, *prevText, *nextText;
    uint32_t state = control->state;
    int textWidth, textHeight;
    int lineHeight;
    int textY;
    JGCONTROLHANDLE handle;
    char *txt;
    int cnt;
    int x, xe;
    int y, ye;
    int dx;
    int l, r;
    int newCaretX, newCaretY;
    switch(msg)
    {
    case JGEVENT_CREATE:
        JGList_Init(lines, 8, sizeof(JGTEXT));
        return 0;
    case JGEVENT_SETHANDLE:
        textArea->handle = (JGCONTROLHANDLE) event;
        return 0;
    case JGEVENT_SETTEXT:
        if(lines->count)
            JGList_Clear(lines);
        JGText_Set0(&line, (string_t) event);
        JGList_AddElem(lines, &line);
        textArea->caretX = line.length;
        textArea->caretY = 0;
        return 0;
    case JGEVENT_SETCURSOR:
        JGSetCursor(JGLoadCursor(JGCURSOR_CARET));
        return 0;
    case JGEVENT_REDRAW:
        JGNoStroke();
        if(!(state & JGSTYLE_NOTDRAWBG))
        {
            JGFill(control->style->colorBackground);
            JGRect(control->left, control->top, control->right, control->bottom);
        }
        if(lines->count)
        {
            JGClipRect(&control->rect);
            JGSetFont(control->style->font);
            JGTextColor(control->style->colorText);
            lineHeight = JGTextHeight();
            textHeight = lineHeight * lines->count;
            JGTextAlign(0);
            if(control->state & JGSTATE_SELECTED)
            {
                JGFill(0xAA4433);
                if(textArea->caretY < textArea->selectY)
                {
                    y = textArea->caretY;
                    ye = textArea->selectY;
                    x = textArea->caretX;
                    xe = textArea->selectX;
                }
                else
                {
                    y = textArea->selectY;
                    ye = textArea->caretY;
                    x = textArea->selectX;
                    xe = textArea->caretX;
                }
                text = JGList_Get(lines, y);
                cnt = ye - y + 1;
                if(control->state & JGSTYLE_TOP)
                    textY = control->top;
                else if(control->state & JGSTYLE_BOTTOM)
                    textY = control->bottom - textHeight;
                else
                    textY = (control->top + control->bottom - textHeight) / 2;
                textY += lineHeight * y;
                l = JGTextWidth(text->text, x);
                while(cnt--)
                {
                    textWidth = JGTextWidth(text->text, text->length);
                    if(control->state & JGSTYLE_LEFT)
                        dx = control->left;
                    else if(control->state & JGSTYLE_RIGHT)
                        dx = control->right - textWidth;
                    else
                        dx = (control->left + control->right - textWidth) / 2;
                    r = !cnt ? JGTextWidth(text->text, xe) : textWidth;
                    JGRect(l + dx, textY, r + dx, textY + lineHeight);
                    l = 0;
                    textY += lineHeight;
                    text++;
                }
            }
            text = lines->elems;
            cnt = lines->count;
            if(control->state & JGSTYLE_TOP)
                textY = control->top;
            else if(control->state & JGSTYLE_BOTTOM)
                textY = control->bottom - textHeight;
            else
                textY = (control->top + control->bottom - textHeight) / 2;
            while(cnt--)
            {
                if(text->text)
                {
                    if(control->state & JGSTYLE_LEFT)
                        x = control->left;
                    else if(control->state & JGSTYLE_RIGHT)
                        x = control->right - JGTextWidth(text->text, text->length);
                    else
                        x = (control->left + control->right - JGTextWidth(text->text, text->length)) / 2;
                    JGText(text->text, text->length, x, textY);
                }
                textY += lineHeight;
                text++;
            }
            if(JGGetKeyboardFocusControl() == control)
            {
                text = JGList_Get(lines, textArea->caretY);
                if(control->state & JGSTYLE_TOP)
                    textY = control->top;
                else if(control->state & JGSTYLE_BOTTOM)
                    textY = control->bottom - textHeight;
                else
                    textY = (control->top + control->bottom - textHeight) / 2;
                textY += lineHeight * textArea->caretY;
                x = JGTextWidth(text->text, textArea->caretX);
                textWidth = x + JGTextWidth(text->text + textArea->caretX, text->length - textArea->caretX);
                if(control->state & JGSTYLE_LEFT)
                    x += control->left;
                else if(control->state & JGSTYLE_RIGHT)
                    x += control->right - textWidth;
                else
                    x += (control->left + control->right - textWidth) / 2;
                JGFill(control->style->colorForeground);
                JGRect(x, textY, x + 2, textY + lineHeight);
            }
            JGClipRect(NULL);
        }
        return 0;
    case JGEVENT_MOUSEPRESSED:
        JGRequestFocus(control);
        if(!lines->count)
            return 0;
        if(!_JGTranslateMouse(control, event->x, event->y, &textArea->selectX, &textArea->selectY))
            return 0;
        textArea->caretX = textArea->selectX;
        textArea->caretY = textArea->selectY;
        control->state |= JGSTATE_SELECTED;
        return 0;
    case JGEVENT_MOUSEDRAGGED:
        _JGTranslateMouse(control, event->x, event->y, &textArea->caretX, &textArea->caretY);
        return 0;
    case JGEVENT_KEYPRESSED:
        text = JGList_Get(lines, textArea->caretY);
        switch(event->vkCode)
        {
        case VK_BACK:
            if(control->state & JGSTATE_SELECTED)
            {
                _JGRemoveSelection(textArea);
                goto text_changed;
            }
            else if(textArea->caretX | textArea->caretY)
            {
                if(!textArea->caretX)
                {
                    prevText = text - 1;
                    JGText_AppendText(prevText, text);
                    x = text->length;
                    free(text->text);
                    textArea->caretX = prevText->length - x;
                    JGList_RemoveElemAtIndex(lines, textArea->caretY);
                    textArea->caretY--;
                    goto text_changed;
                }
                else if(JGGetKeyState(VK_CONTROL) & 0x8000)
                {
                    txt = FindSR(text->text + textArea->caretX - 1, textArea->caretX - 1);
                    JGText_RemoveRangeAt(text, txt - text->text, text->text + textArea->caretX - txt);
                    textArea->caretX = txt - text->text;
                    goto text_changed;
                }
                else
                {
                    textArea->caretX--;
                    JGText_RemoveCharAt(text, textArea->caretX);
                    goto text_changed;
                }
            }
            break;
        case VK_DELETE:
            if(control->state & JGSTATE_SELECTED)
            {
                _JGRemoveSelection(textArea);
                goto text_changed;
            }
            else if(textArea->caretY + 1 != lines->count || textArea->caretX != text->length)
            {
                if(textArea->caretX == text->length)
                {
                    nextText = text + 1;
                    JGText_AppendText(text, nextText);
                    JGList_RemoveElemAtIndex(lines, textArea->caretY + 1);
                    goto text_changed;
                }
                else if(JGGetKeyState(VK_CONTROL) & 0x8000)
                {
                    txt = FindS(text->text + textArea->caretX + 1, text->length - textArea->caretX - 1);
                    JGText_RemoveRangeAt(text, textArea->caretX, txt - text->text - textArea->caretX);
                    goto text_changed;
                }
                else
                {
                    JGText_RemoveCharAt(text, textArea->caretX);
                    goto text_changed;
                }
            }
            break;
        case VK_UP:
            if(!textArea->caretY)
                break;
            prevText = text - 1;
            x = JGTextWidth(text->text, textArea->caretX);
            if(control->state & JGSTYLE_RIGHT)
            {
                x -= JGTextWidth(text->text, text->length);
                x += JGTextWidth(prevText->text, prevText->length);
            }
            else if(!(control->state & JGSTYLE_LEFT))
            {
                x -= JGTextWidth(text->text, text->length) / 2;
                x += JGTextWidth(prevText->text, prevText->length) / 2;
            }
            _JGTATranslate(control, prevText, x, textArea->caretY - 1, &newCaretX, &newCaretY);
            goto rm_select;
        case VK_DOWN:
            if(textArea->caretY + 1 == lines->count)
                break;
            nextText = text + 1;
            x = JGTextWidth(text->text, textArea->caretX);
            if(control->state & JGSTYLE_RIGHT)
            {
                x -= JGTextWidth(text->text, text->length);
                x += JGTextWidth(nextText->text, nextText->length);
            }
            else if(!(control->state & JGSTYLE_LEFT))
            {
                x -= JGTextWidth(text->text, text->length) / 2;
                x += JGTextWidth(nextText->text, nextText->length) / 2;
            }
            _JGTATranslate(control, nextText, x, textArea->caretY + 1, &newCaretX, &newCaretY);
            goto rm_select;
        case VK_LEFT:
            if(!(textArea->caretX | textArea->caretY))
                break;
            if(!textArea->caretX)
            {
                newCaretX = (text - 1)->length;
                newCaretY = textArea->caretY - 1;
                goto rm_select;
            }
            else if(JGGetKeyState(VK_CONTROL) & 0x8000)
            {
                txt = FindSR(text->text + textArea->caretX - 1, textArea->caretX - 1);
                newCaretX = txt - text->text;
                newCaretY = textArea->caretY;
                goto rm_select;
            }
            else
            {
                newCaretX = textArea->caretX - 1;
                newCaretY = textArea->caretY;
                goto rm_select;
            }
            break;
        case VK_RIGHT:
            if(textArea->caretY + 1 == lines->count && textArea->caretX == text->length)
                break;
            if(textArea->caretX == text->length)
            {
                newCaretX = 0;
                newCaretY = textArea->caretY + 1;
                goto rm_select;
            }
            else if(JGGetKeyState(VK_CONTROL) & 0x8000)
            {
                txt = FindS(text->text + textArea->caretX + 1, text->length - textArea->caretX - 1);
                newCaretX = txt - text->text;
                newCaretY = textArea->caretY;
                goto rm_select;
            }
            else
            {
                newCaretX = textArea->caretX + 1;
                newCaretY = textArea->caretY;
                goto rm_select;
            }
            break;
        case VK_HOME:
            if(!lines->count)
                break;
            newCaretX = 0;
            newCaretY = textArea->caretY;
            goto rm_select;
        case VK_END:
            if(!lines->count)
                break;
            newCaretX = text->length;
            newCaretY = textArea->caretY;
            goto rm_select;
        case 'A':
            if(!lines->count)
                break;
            if(JGGetKeyState(VK_CONTROL) & 0x8000)
            {
                textArea->selectX = textArea->selectY = 0;
                textArea->caretY = lines->count - 1;
                textArea->caretX = ((JGTEXT*) JGList_Get(lines, textArea->caretY))->length;
                control->state |= JGSTATE_SELECTED;
            }
            break;
        case 'C':
        case 'V':
        case 'X':
            // TODO: ADD COPY PASTE
            // ...
            break;
        case VK_RETURN:
            if(control->state & JGSTATE_SELECTED)
                _JGRemoveSelection(textArea);
            x = textArea->caretX;
            xe = text->length;
            if(xe - x)
            {
                JGText_Set(&line, text->text + x, xe - x);
                text->length = x;
            }
            else
            {
                line.text = NULL;
                line.capacity = 0;
                line.length = 0;
            }
            JGList_InsertElem(lines, textArea->caretY + 1, &line);
            textArea->caretX = 0;
            textArea->caretY++;
            goto text_changed;
        }
        return 0;
    case JGEVENT_KEYTYPED:
        if(event->vkCode < ' ' || event->vkCode >= 127)
            return 0;
        if(control->state & JGSTATE_SELECTED)
            _JGRemoveSelection(textArea);
        JGText_InsertChar(JGList_Get(lines, textArea->caretY), event->keyChar, textArea->caretX);
        textArea->caretX++;
        goto text_changed;
    }
    return JGDefProc(control, msg, event);
    rm_select:
        if(!(JGGetKeyState(VK_SHIFT) & 0x8000))
            control->state &= ~JGSTATE_SELECTED;
        else if(!(control->state & JGSTATE_SELECTED))
        {
            control->state |= JGSTATE_SELECTED;
            textArea->selectX = textArea->caretX;
            textArea->selectY = textArea->caretY;
        }
        textArea->caretX = newCaretX;
        textArea->caretY = newCaretY;
        return 0;
    text_changed:
        if((handle = textArea->handle))
            handle(control, JGHANDLE_TEXT, NULL);
        return 0;
}
