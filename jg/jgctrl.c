#include "jg.h"
#include "jgutil.h"

static JGCONTROL *Controls;
static uint32_t ControlCount;
static uint32_t ControlCapacity;

static uint32_t *RemoveQueue;
static uint32_t RemoveQueueIndex;
static uint32_t RemoveQueueCapacity;

static uint64_t JGStaticProc(JGCONTROL, uint32_t, JGEVENT*);
static uint64_t JGButtonProc(JGCONTROL, uint32_t, JGEVENT*);
static uint64_t JGGroupProc(JGCONTROL, uint32_t, JGEVENT*);
static uint64_t JGWindowProc(JGCONTROL, uint32_t, JGEVENT*);
static uint64_t JGSliderProc(JGCONTROL, uint32_t, JGEVENT*);
static uint64_t JGImageViewProc(JGCONTROL, uint32_t, JGEVENT*);

static uint64_t JGDefEventProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    return 0;
}

static JGCONTROLSTYLE DefaultStyle = {
    .colorText = JGWHITE,
    .colorBackground = JGBLACK,
    .colorHover = JGGRAY,
    .colorForeground = JGDKGRAY
};

static JGCONTROLSTYLE BaseStyles[] = {
    { NULL, JGBLACK, JGGRAY, JGWHITE, JGDKGRAY },
    { NULL, JGBLACK, JGGRAY, JGWHITE, JGDKGRAY },
    { NULL, JGBLACK, JGGRAY, JGWHITE, JGDKGRAY },
    { NULL, JGBLACK, JGGRAY, JGWHITE, JGDKGRAY },
    { NULL, JGBLACK, JGGRAY, JGWHITE, JGDKGRAY },
    { NULL, JGBLACK, JGGRAY, JGWHITE, JGDKGRAY },
};

void JGSetDefaultStyle(const JGCONTROLSTYLE *style)
{
    DefaultStyle = *style;
    for(int i = 0; i < 6; i++)
        BaseStyles[i] = *style;
}

void JGGetDefaultStyle(JGCONTROLSTYLE *style)
{
    *style = DefaultStyle;
}

static inline int GetStyleIndex(const char *name)
{
    int n;
    n = *name - 66;
    n %= 18;
    n %= 15;
    n %= 6;
    return n;
}

JGCONTROLSTYLE *JGGetBaseStyle(const char *name)
{
    return BaseStyles + GetStyleIndex(name);
}

void JGSetBaseStyle(const char *name, const JGCONTROLSTYLE *style)
{
    BaseStyles[GetStyleIndex(name)] = *style;
}

// TODO: MAKE HASHTABLE
JGCONTROLSTYLE *ControlStyles;
int ControlStyleCount;
int ControlStyleCapacity;

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

JGCONTROL JGCreateControl(control_t type, string_t id, string_t styleName, int state, string_t text, int x, int y, int width, int height)
{
    JGCONTROL control;
    JGCONTROLSTYLE *style = NULL;
    if(styleName)
        style = JGGetStyle(styleName);
    if(!style && id)
        style = JGGetStyle(id);
    switch(type)
    {
    case JGTYPE_STATIC:    if(!style) style = JGGetBaseStyle("Label"); control = malloc(sizeof(__JGSTATIC));    memset(control, 0, sizeof(__JGSTATIC)); control->proc = JGStaticProc; JGSetText(control, text);  break;
    case JGTYPE_GROUP:     if(!style) style = JGGetBaseStyle("Group"); control = malloc(sizeof(__JGGROUP));     memset(control, 0, sizeof(__JGGROUP)); control->proc = JGGroupProc; control->state = JGSTATE_TRANSPARENT; break;
    case JGTYPE_BUTTON:    if(!style) style = JGGetBaseStyle("Button"); control = malloc(sizeof(__JGBUTTON));    memset(control, 0, sizeof(__JGBUTTON)); control->proc = JGButtonProc; JGSetText(control, text); break;
    case JGTYPE_WINDOW:    if(!style) style = JGGetBaseStyle("Window"); control = malloc(sizeof(__JGWINDOW));    memset(control, 0, sizeof(__JGWINDOW)); control->proc = JGWindowProc; JGSetText(control, text); break;
    case JGTYPE_SLIDER:    if(!style) style = JGGetBaseStyle("Slider"); control = malloc(sizeof(__JGSLIDER));    memset(control, 0, sizeof(__JGSLIDER)); control->proc = JGSliderProc; JGSetText(control, text); break;
    case JGTYPE_IMAGEVIEW: if(!style) style = JGGetBaseStyle("Image"); control = malloc(sizeof(__JGIMAGEVIEW)); memset(control, 0, sizeof(__JGIMAGEVIEW)); control->proc = JGImageViewProc; break;
    default:
        return NULL;
    }
    control->id = id ? JGUtil_Createstrcpy0(id) : NULL;
    control->style = style;
    control->state |= state;
    control->rect = (JGRECT) {
        .left = x,
        .top = y,
        .right = x + width,
        .bottom = y + height
    };
    control->proc(control, JGEVENT_CREATE, NULL);
    control->proc(control, JGEVENT_SIZE, NULL);
    return control;
}

bool JGDestroyControl(JGCONTROL control)
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
        if(RemoveQueueIndex >= RemoveQueueCapacity)
        {
            RemoveQueueCapacity *= 2;
            RemoveQueueCapacity++;
            RemoveQueue = realloc(RemoveQueue, sizeof(*RemoveQueue) * RemoveQueueCapacity);
        }
        RemoveQueue[oldRqIndex] = index;
    }
    control->proc(control, JGEVENT_DESTROY, NULL);
    free(control->id);
    if(cnt < 0)
        free(control);
    else
    {
        control->id = NULL;
        control->proc = JGDefEventProc;
    }
    return 1;
}

void JGAddControl(JGCONTROL control)
{
    uint32_t cnt;
    uint32_t newCnt;
    JGCONTROL *dest;
    if(RemoveQueueIndex)
    {
        dest = Controls + RemoveQueue[--RemoveQueueIndex];
        free(*dest);
    }
    else
    {
        cnt = ControlCount;
        newCnt = ControlCount = cnt + 1;
        if(newCnt > ControlCapacity)
        {
            ControlCapacity *= 2;
            ControlCapacity += newCnt;
            Controls = realloc(Controls, sizeof(*Controls) * ControlCapacity);
        }
        dest = Controls + cnt;
    }
    *dest = control;
}

void JGAddChildControl(JGCONTROL parent, JGCONTROL child)
{
    uint32_t cnt = ((JGGROUP) parent)->childrenCount;
    uint32_t newCnt = ((JGGROUP) parent)->childrenCount = cnt + 1;
    if(newCnt > ((JGGROUP) parent)->childrenCapacity)
    {
        ((JGGROUP) parent)->childrenCapacity *= 2;
        ((JGGROUP) parent)->childrenCapacity += newCnt;
        ((JGGROUP) parent)->children = realloc(((JGGROUP) parent)->children, sizeof(child) * ((JGGROUP) parent)->childrenCapacity);
    }
    *(((JGGROUP) parent)->children + cnt) = child;

    JGAddControl(child);
}

JGCONTROL JGGetControl(const char *id)
{
    JGCONTROL control;
    JGCONTROL *controls = Controls;
    int cnt = ControlCount;
    while(cnt--)
    {
        control = *(controls++);
        if(control->id)
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
    int dx = control->left - control->right + size.width;
    int dy = control->top - control->bottom + size.height;
    if(!(dx | dy))
        return 0;
    control->right = control->left + size.width;
    control->bottom = control->top + size.height;
    control->proc(control, JGEVENT_SIZE, NULL);
    return 1;
}

void JGMoveControl(JGCONTROL control, int32_t dx, int32_t dy)
{
    JGEVENT move;
    move.dx = dx;
    move.dy = dy;
    control->left += dx;
    control->right += dx;
    control->top += dy;
    control->bottom += dy;
    control->proc(control, JGEVENT_MOVE, &move);
}

uint64_t JGShareEvent(JGCONTROL *controls, uint32_t cnt, uint32_t msg, JGEVENT *event)
{
    JGCONTROL control;
    if(msg == JGEVENT_REDRAW)
    {
        controls += cnt - 1;
        while(cnt--)
        {
            control = *(controls--);
            if(!(control->state & JGSTATE_INVISIBLE))
                control->proc(control, JGEVENT_REDRAW, event);
        }
    }
    else
    {
        while(cnt--)
        {
            control = *(controls++);
            if(!(control->state & JGSTATE_INVISIBLE))
                control->proc(control, msg, event);
        }
    }
    return 0;
}

JGCONTROL MouseHoverControl;
JGCONTROL MouseFocusControl;

JGCONTROL JGGetMouseOverControl(void)
{
    return MouseHoverControl;
}

JGCONTROL KeyboardFocusControl;

void JGRequestFocus(JGCONTROL control)
{
    if(KeyboardFocusControl)
        JGKillFocus(KeyboardFocusControl);
    KeyboardFocusControl = control;
    KeyboardFocusControl->proc(KeyboardFocusControl, JGEVENT_FOCUSGAINED, NULL);
}

void JGKillFocus(JGCONTROL control)
{
    if(KeyboardFocusControl == control)
    {
        KeyboardFocusControl->proc(KeyboardFocusControl, JGEVENT_FOCUSLOST, NULL);
        KeyboardFocusControl = NULL;
    }
}

uint64_t JGDispatchEvent(uint32_t msg, JGEVENT *event)
{
    int i;
    JGCONTROL control;
    JGCONTROL *controls;
    switch(msg)
    {
    case JGEVENT_MOUSEMOVED:
        if(MouseFocusControl)
        {
            if(MouseHoverControl && !JGRectContains(&MouseHoverControl->rect, &event->mousePos))
                MouseHoverControl = NULL;
            else if(!MouseHoverControl && JGRectContains(&MouseFocusControl->rect, &event->mousePos))
                MouseHoverControl = MouseFocusControl;
            return MouseFocusControl->proc(MouseFocusControl, JGEVENT_MOUSEDRAGGED, event);
        }
        controls = Controls;
        for(i = 0; i < ControlCount; i++, controls++)
        {
            control = *controls;
            if(!(control->state & (JGSTATE_TRANSPARENT | JGSTATE_INVISIBLE)) && JGRectContains(&control->rect, &event->mousePos))
                break;
            control = NULL;
        }
        if(MouseHoverControl)
        {
            if(MouseHoverControl != control)
            {
                MouseHoverControl->proc(MouseHoverControl, JGEVENT_LEAVE, event);
            }
            else
            {
                return MouseHoverControl->proc(MouseHoverControl, JGEVENT_MOUSEMOVED, event);
            }
        }
        if(i < ControlCount)
        {
            MouseHoverControl = control;
            MouseHoverControl->proc(MouseHoverControl, JGEVENT_HOVER, event);
            MouseHoverControl->proc(MouseHoverControl, JGEVENT_MOUSEMOVED, event);
        }
        else
            MouseHoverControl = NULL;
        return 0;
    case JGEVENT_MOUSERELEASED:
        if(MouseFocusControl)
        {
            i = MouseFocusControl->proc(MouseFocusControl, JGEVENT_MOUSERELEASED, event);
            MouseFocusControl = NULL;
            return i;
        }
        return 0;
    case JGEVENT_MOUSEPRESSED:
        if(MouseHoverControl)
            MouseFocusControl = MouseHoverControl;
    case JGEVENT_MOUSEWHEEL:
        return MouseHoverControl ? MouseHoverControl->proc(MouseHoverControl, msg, event) : 0;
    }
    return JGShareEvent(Controls, ControlCount, msg, event);
}

static uint64_t JGGroupProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGGROUP group = (JGGROUP) control;
    int cnt;
    JGCONTROL *controls;
    switch(msg)
    {
    case JGEVENT_DESTROY:
        cnt = group->childrenCount;
        controls = group->children;
        while(cnt--)
        {
            JGDestroyControl(*controls);
            controls++;
        }
        return 0;
    case JGEVENT_MOVE:
        cnt = group->childrenCount;
        controls = group->children;
        while(cnt--)
        {
            JGMoveControl(*controls, event->dx, event->dy);
            controls++;
        }
        break;
    case JGEVENT_POS:
    case JGEVENT_SIZE:
        if(group->layout.type)
            group->layout.layoutFunc(&group->layout, group->children, group->childrenCount, &control->rect);
        return 0;
    case JGEVENT_SHOW:
        if(control->state & JGSTATE_INVISIBLE)
        {
            cnt = group->childrenCount;
            controls = group->children;
            while(cnt--)
            {
                JGShowControl(*controls);
                controls++;
            }
        }
        return 0;
    case JGEVENT_HIDE:
        if(!(control->state & JGSTATE_INVISIBLE))
        {
            cnt = group->childrenCount;
            controls = group->children;
            while(cnt--)
            {
                JGHideControl(*controls);
                controls++;
            }
        }
        return 0;
    }
    return 0;
}

static uint64_t JGWindowProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGWINDOW window = (JGWINDOW) control;
    JGTEXT text = window->text;
    uint32_t state = control->state;
    JGRECT rect;
    //int ccnt;
    _Bool isSize;
    int cnt;
    JGCONTROL *controls;
    switch(msg)
    {
    case JGEVENT_DESTROY:
        cnt = window->childrenCount;
        controls = window->children;
        while(cnt--)
        {
            JGDestroyControl(*controls);
            controls++;
        }
        return 0;
    case JGEVENT_REDRAW:
        JGNoStroke();
        JGFill(control->style->colorBackground);
        JGRect(control->left, control->top, control->right, control->bottom);
        if(!(state & JGSTATE_BORDERLESS))
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
            if(state & JGSTATE_XBUTTON)
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
                JGSetStroke(JGSTROKE_STYLE_DASH, 2, control->style->colorText);
                JGLine(control->right - 22, control->top + 10, control->right - 10, control->top + 22);
                JGLine(control->right - 10, control->top + 10, control->right - 22, control->top + 22);
            }
        }
        return 0;
    case JGEVENT_MOVE:
        cnt = window->childrenCount;
        controls = window->children;
        while(cnt--)
        {
            JGMoveControl(*controls, event->dx, event->dy);
            controls++;
        }
        break;
    case JGEVENT_POS:
    case JGEVENT_SIZE:
        rect = (state & JGSTATE_BORDERLESS) ? control->rect : (JGRECT) {
            .left   = control->left + 9,
            .top    = control->top + 30,
            .right  = control->right - 7,
            .bottom = control->bottom - 8,
        };
        if(window->layout.type)
            window->layout.layoutFunc(&window->layout, window->children, window->childrenCount, &rect);
        return 0;
    case JGEVENT_SHOW:
        if(state & JGSTATE_INVISIBLE)
        {
            cnt = window->childrenCount;
            controls = window->children;
            while(cnt--)
            {
                JGShowControl(*controls);
                controls++;
            }
        }
        return 0;
    case JGEVENT_HIDE:
        if(!(state & JGSTATE_INVISIBLE))
        {
            cnt = window->childrenCount;
            controls = window->children;
            while(cnt--)
            {
                JGHideControl(*controls);
                controls++;
            }
        }
        return 0;
    case JGEVENT_SETCURSOR:
        switch(state & (JGSTATE_MOUSEINCLIENT | JGSTATE_MOUSEINBUTTON | JGSTATE_MOVE | JGSTATE_RESIZE_LEFT | JGSTATE_RESIZE_TOP | JGSTATE_RESIZE_RIGHT | JGSTATE_RESIZE_BOTTOM))
        {
        case JGSTATE_MOUSEINCLIENT:
            JGSetCursor(JGCURSOR_DEFAULT);
            break;
        case JGSTATE_MOUSEINBUTTON:
            JGSetCursor(JGCURSOR_HAND);
            break;
        case JGSTATE_MOVE:
            JGSetCursor(JGCURSOR_MOVE);
            break;
        case JGSTATE_RESIZE_RIGHT:
        case JGSTATE_RESIZE_LEFT:
            JGSetCursor(JGCURSOR_LR);
            break;
        case JGSTATE_RESIZE_TOP:
        case JGSTATE_RESIZE_BOTTOM:
            JGSetCursor(JGCURSOR_UD);
            break;
        case JGSTATE_RESIZE_RIGHT | JGSTATE_RESIZE_TOP:
        case JGSTATE_RESIZE_LEFT | JGSTATE_RESIZE_BOTTOM:
            JGSetCursor(JGCURSOR_LD_RU);
            break;
        case JGSTATE_RESIZE_LEFT | JGSTATE_RESIZE_TOP:
        case JGSTATE_RESIZE_RIGHT | JGSTATE_RESIZE_BOTTOM:
            JGSetCursor(JGCURSOR_LU_RD);
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
        if(control->state & JGSTATE_XBUTTON)
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
        isSize = event->x < control->left + 8 || event->x > control->right - 8 || event->y < control->top + 8 || event->y > control->bottom - 8;
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
            printf("DESTROYING WINDOW\n");
            JGDestroyControl(control);
            return 0;
        }
        control->state &= ~JGSTATE_BUTTONPRESSED;
	    break;
    case JGEVENT_LEAVE:
        control->state &= ~(JGSTATE_MOUSEINCLIENT | JGSTATE_MOUSEINBUTTON | JGSTATE_MOVE | JGSTATE_RESIZE_LEFT | JGSTATE_RESIZE_TOP | JGSTATE_RESIZE_RIGHT | JGSTATE_RESIZE_BOTTOM);
        break;
    }
    return 0;
}

uint64_t JGSliderProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGTEXT text = ((JGSLIDER) control)->text;
    JGRECT rect;
    uint32_t state = control->state;
    int sp = ((JGSLIDER) control)->sliderPos;
    switch(msg)
    {
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
    return 0;
}

uint64_t JGImageViewProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGIMAGE image = ((JGIMAGEVIEW) control)->image;
    JGRECT cut = ((JGIMAGEVIEW) control)->cut;
    if(msg == JGEVENT_REDRAW && image)
    {
        if(cut.left >= cut.right)
            JGDrawImage(image, control->left, control->top, control->right - control->left, control->bottom - control->top);
        else
            JGDrawImageSection(image, cut.left, cut.top, cut.right - cut.left, cut.bottom - cut.top, control->left, control->top, control->right - control->left, control->bottom - control->top);
    }
    return 0;
}

uint64_t JGStaticProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGTEXT text = ((JGSTATIC) control)->text;
    uint32_t state = control->state;
    if(msg == JGEVENT_REDRAW)
    {
        JGNoStroke();
        if(!(state & JGSTATE_NOTDRAWBG))
        {
            JGFill(control->style->colorBackground);
            JGRect(control->left, control->top, control->right, control->bottom);
        }
        if(text.text)
        {
            JGSetFont(control->style->font);
            JGTextAlign(JGALIGN_CENTER | 24);
            JGTextColor(control->style->colorText);
            JGBkMode(JGTRANSPARENT);
            JGText(text.text, text.length, (control->left + control->right) / 2, (control->top + control->bottom + 7) / 2);
        }
    }
    return 0;
}

uint64_t JGButtonProc(JGCONTROL control, uint32_t msg, JGEVENT *event)
{
    JGTEXT text = ((JGBUTTON) control)->text;
    uint32_t state = control->state;
    JGCONTROLHANDLE handle;
    switch(msg)
    {
    case JGEVENT_REDRAW:
        JGNoStroke();
        if(!(state & JGSTATE_NOTDRAWBG))
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
            JGBkMode(JGTRANSPARENT);
            JGText(text.text, text.length, (control->left + control->right) / 2, (control->top + control->bottom + 7) / 2);
        }
        return 0;
    case JGEVENT_MOUSERELEASED:
        if(MouseHoverControl && (handle = ((JGBUTTON) control)->handle))
            handle(control, JGHANDLE_TOGGLED, event);
        break;
    }
    return 0;
}
