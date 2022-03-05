#include "jglayout.h"
#include "jg.h"

static void JGFlowLayoutFunc(JGLAYOUT *layout, JGCONTROL *chn, int cnt, const JGRECT *bounds)
{
    if(!cnt)
        return;
    JGCONTROL control;
    JGRECT iRect;
    JGRECT rect;
    JGRECT maxRect;
    int dx, dy;
    int cw;

    int cCnt;
    JGCONTROL *cChn;

    int widths[cnt];
    int width = bounds->right - bounds->left;
    int *pWidths = widths;
    int notFixedCnt = 0;
    cCnt = cnt;
    cChn = chn;
    while(cnt--)
    {
        control = *(chn++);
        switch(control->state & JGSTYLE_FIXEDSIZE)
        {
        case JGSTYLE_FIXEDSIZE:
        case JGSTYLE_FIXEDHEIGHT:
            if((control->state & JGSTYLE_CORNERED))
            {
                width -= control->bottom - control->top;
                *pWidths = control->bottom - control->top;
                break;
            }
        case 0:
        case JGSTYLE_FIXEDWIDTH:
            *pWidths = -1;
            notFixedCnt++;
            break;
        }
        pWidths++;
    }
    cw = !notFixedCnt ? 0 : width / notFixedCnt;
    iRect = *bounds;
    pWidths = widths;
    while(cCnt--)
    {
        control = *(cChn++);
        width = *(pWidths++);
        iRect.right = iRect.left + (width < 0 ? cw : width);
        maxRect = iRect;
        maxRect.left += control->insets.left;
        maxRect.top += control->insets.top;
        maxRect.right -= control->insets.right;
        maxRect.bottom -= control->insets.bottom;
        if(width < 0)
        {
            switch(control->state & JGSTYLE_FIXEDSIZE)
            {
            case 0: rect = maxRect; break;
            case JGSTYLE_FIXEDSIZE:
                rect.left = maxRect.left;
                rect.top = maxRect.top;
                rect.right = rect.left - control->rect.left + control->rect.right;
                rect.bottom = rect.top - control->rect.top + control->rect.bottom;
                if(!(control->state & JGSTYLE_CORNERED))
                {
                    dx = maxRect.right  - rect.right;
                    dy = maxRect.bottom - rect.bottom;
                    dx >>= 1;
                    dy >>= 1;
                    rect.left += dx;
                    rect.top += dy;
                    rect.right += dx;
                    rect.bottom += dy;
                }
                break;
            case JGSTYLE_FIXEDWIDTH:
                rect.left = maxRect.left;
                rect.top = maxRect.top;
                rect.right = rect.left - control->rect.left + control->rect.right;
                rect.bottom = maxRect.bottom;
                if(!(control->state & JGSTYLE_CORNERED))
                {
                    dx = maxRect.right  - rect.right;
                    dx >>= 1;
                    rect.left += dx;
                    rect.right += dx;
                }
                break;
            case JGSTYLE_FIXEDHEIGHT:
                rect.left = maxRect.left;
                rect.top = maxRect.top;
                rect.right = maxRect.right;
                rect.bottom = rect.top - control->rect.top + control->rect.bottom;
                if(!(control->state & JGSTYLE_CORNERED))
                {
                    dy = maxRect.bottom - rect.bottom;
                    dy >>= 1;
                    rect.top += dy;
                    rect.bottom += dy;
                }
                break;
            }
        }
        else
        {
            rect = maxRect;
        }
        JGSetBounds(control, &rect);
        iRect.left = iRect.right;
    }
}

static void JGStackLayoutFunc(JGLAYOUT *layout, JGCONTROL *chn, int cnt, const JGRECT *bounds)
{
    if(!cnt)
        return;
    JGCONTROL control;
    JGRECT iRect;
    JGRECT rect;
    JGRECT maxRect;
    int dx, dy;
    int ch;

    int cCnt;
    JGCONTROL *cChn;

    int heights[cnt];
    int height = bounds->bottom - bounds->top;
    int *pHeights = heights;
    int notFixedCnt = 0;
    cCnt = cnt;
    cChn = chn;
    while(cnt--)
    {
        control = *(chn++);
        switch(control->state & JGSTYLE_FIXEDSIZE)
        {
        case JGSTYLE_FIXEDSIZE:
        case JGSTYLE_FIXEDHEIGHT:
            if((control->state & JGSTYLE_CORNERED))
            {
                height -= control->bottom - control->top;
                *pHeights = control->bottom - control->top;
                break;
            }
        case 0:
        case JGSTYLE_FIXEDWIDTH:
            *pHeights = -1;
            notFixedCnt++;
            break;
        }
        pHeights++;
    }
    ch = !notFixedCnt ? 0 : height / notFixedCnt;
    iRect = *bounds;
    pHeights = heights;
    while(cCnt--)
    {
        control = *(cChn++);
        height = *(pHeights++);
        iRect.bottom = iRect.top + (height < 0 ? ch : height);
        maxRect = iRect;
        maxRect.left += control->insets.left;
        maxRect.top += control->insets.top;
        maxRect.right -= control->insets.right;
        maxRect.bottom -= control->insets.bottom;
        if(height < 0)
        {
            switch(control->state & JGSTYLE_FIXEDSIZE)
            {
            case 0: rect = maxRect; break;
            case JGSTYLE_FIXEDSIZE:
                rect.left = maxRect.left;
                rect.top = maxRect.top;
                rect.right = rect.left - control->rect.left + control->rect.right;
                rect.bottom = rect.top - control->rect.top + control->rect.bottom;
                if(!(control->state & JGSTYLE_CORNERED))
                {
                    dx = maxRect.right  - rect.right;
                    dy = maxRect.bottom - rect.bottom;
                    dx >>= 1;
                    dy >>= 1;
                    rect.left += dx;
                    rect.top += dy;
                    rect.right += dx;
                    rect.bottom += dy;
                }
                break;
            case JGSTYLE_FIXEDWIDTH:
                rect.left = maxRect.left;
                rect.top = maxRect.top;
                rect.right = rect.left - control->rect.left + control->rect.right;
                rect.bottom = maxRect.bottom;
                if(!(control->state & JGSTYLE_CORNERED))
                {
                    dx = maxRect.right  - rect.right;
                    dx >>= 1;
                    rect.left += dx;
                    rect.right += dx;
                }
                break;
            case JGSTYLE_FIXEDHEIGHT:
                rect.left = maxRect.left;
                rect.top = maxRect.top;
                rect.right = maxRect.right;
                rect.bottom = rect.top - control->rect.top + control->rect.bottom;
                if(!(control->state & JGSTYLE_CORNERED))
                {
                    dy = maxRect.bottom - rect.bottom;
                    dy >>= 1;
                    rect.top += dy;
                    rect.bottom += dy;
                }
                break;
            }
        }
        else
        {
            rect = maxRect;
        }
        JGSetBounds(control, &rect);
        iRect.top = iRect.bottom;
    }
}

static inline _Bool FindPartner(JGGRIDBAGLAYOUT *layout, JGCONTROL p, JGGRIDBC *dest)
{
    int cnt = layout->gridBcCount;
    JGGRIDBC *gbc = layout->gridBcs;
    while(cnt--)
    {
        if(gbc->partner == p)
        {
            *dest = *gbc;
            return 1;
        }
        gbc++;
    }
    return 0;
}

static void JGGridBagLayoutFunc(JGLAYOUT *layout, JGCONTROL *chn, int cnt, const JGRECT *bounds)
{
    const int cCnt = cnt;
    JGCONTROL *cChn = chn;
    JGGRIDBC c;
    int gridX, gridY;
    int oldGridY = 0;
    int dx, dy;
    int w, h;
    JGRECT rect;
    JGRECT maxRect;

    int totalGridWidth = 1, totalGridHeight = 1;
    int currX = 0;
    JGCONTROL control;
    while(cnt--)
    {
        control = *(chn++);
        if(!FindPartner(&layout->gbg, control, &c))
            c = layout->gbg.defaultGridBc;
        gridX = c.gridX;
        gridY = c.gridY;
        if(gridY < 0)
        {
            gridY = oldGridY;
        }
        else if(gridY != oldGridY)
        {
            currX = 0;
            oldGridY = gridY;
        }
        if(gridX < 0)
        {
            gridX = currX;
        }
        currX += c.gridWidth;
        totalGridWidth  = __max(gridX + c.gridWidth, totalGridWidth);
        totalGridHeight = __max(gridY + c.gridHeight, totalGridHeight);
    }
    w = (bounds->right - bounds->left) / totalGridWidth;
    h = (bounds->bottom - bounds->top) / totalGridHeight;
    currX = 0;
    cnt = cCnt;
    chn = cChn;
    oldGridY = 0;
    while(cnt--)
    {
        control = *(chn++);
        if(!FindPartner(&layout->gbg, control, &c))
            c = layout->gbg.defaultGridBc;
        maxRect.top = c.gridY;
        if(c.gridY < 0)
        {
            maxRect.top = oldGridY;
        }
        else if(c.gridY != oldGridY)
        {
            currX = 0;
            maxRect.top = c.gridY;
            oldGridY = c.gridY;
        }
        maxRect.left = c.gridX < 0 ? currX : c.gridX;
        currX += c.gridWidth;
        maxRect.right = maxRect.left + c.gridWidth;
        maxRect.bottom = maxRect.top + c.gridHeight;

        maxRect.left *= w;
        maxRect.top  *= h;
        maxRect.right *= w;
        maxRect.bottom *= h;

        maxRect.left += control->insets.left;
        maxRect.top += control->insets.top;
        maxRect.right -= control->insets.right;
        maxRect.bottom -= control->insets.bottom;

        switch(control->state & JGSTYLE_FIXEDSIZE)
        {
        case 0: rect = maxRect; break;
        case JGSTYLE_FIXEDSIZE:
            rect.left = maxRect.left;
            rect.top = maxRect.top;
            rect.right = rect.left - control->rect.left + control->rect.right;
            rect.bottom = rect.top - control->rect.top + control->rect.bottom;
            if(!(control->state & JGSTYLE_CORNERED))
            {
                dx = maxRect.right  - rect.right;
                dy = maxRect.bottom - rect.bottom;
                dx >>= 1;
                dy >>= 1;
                rect.left += dx;
                rect.top += dy;
                rect.right += dx;
                rect.bottom += dy;
            }
            break;
        case JGSTYLE_FIXEDWIDTH:
            rect.left = maxRect.left;
            rect.top = maxRect.top;
            rect.right = rect.left - control->rect.left + control->rect.right;
            rect.bottom = maxRect.bottom;
            if(!(control->state & JGSTYLE_CORNERED))
            {
                dx = maxRect.right  - rect.right;
                dx >>= 1;
                rect.left += dx;
                rect.right += dx;
            }
            break;
        case JGSTYLE_FIXEDHEIGHT:
            rect.left = maxRect.left;
            rect.top = maxRect.top;
            rect.right = maxRect.right;
            rect.bottom = rect.top - control->rect.top + control->rect.bottom;
            if(!(control->state & JGSTYLE_CORNERED))
            {
                dy = maxRect.bottom - rect.bottom;
                dy >>= 1;
                rect.top += dy;
                rect.bottom += dy;
            }
            break;
        }
        rect.left   += bounds->left;
        rect.top    += bounds->top;
        rect.right  += bounds->left;
        rect.bottom += bounds->top;
        JGSetBounds(control, &rect);
    }
}

void JGGridBagLayout_AddLayoutControl(JGCONTROL parent, JGGRIDBC *gbc)
{
    JGLAYOUT *layout;
    int cnt;
    layout = JGGetLayout(parent);
    cnt = layout->gbg.gridBcCount++;
    if(layout->gbg.gridBcCount > layout->gbg.gridBcCapacity)
    {
        layout->gbg.gridBcCapacity *= 2;
        layout->gbg.gridBcCapacity++;
        layout->gbg.gridBcs = realloc(layout->gbg.gridBcs, sizeof(*layout->gbg.gridBcs) * layout->gbg.gridBcCapacity);
    }
    *(layout->gbg.gridBcs + cnt) = *gbc;
}

bool JGGridBagLayout_RemoveLayoutControl(JGLAYOUT *layout, JGCONTROL control)
{
    int cnt = layout->gbg.gridBcCount;
    JGGRIDBC *gbc = layout->gbg.gridBcs;
    while(cnt--)
    {
        if(gbc->partner == control)
        {
            memmove(gbc, gbc + 1, sizeof(*gbc) * cnt);
            return 1;
        }
        gbc++;
    }
    return 0;
}

bool JGGetStockLayout(JGLAYOUT *layout, int type)
{
    layout->type = type;
    switch(type)
    {
    case JGLAYOUT_FLOW:
        layout->layoutFunc = JGFlowLayoutFunc;
        break;
    case JGLAYOUT_STACK:
        layout->layoutFunc = JGStackLayoutFunc;
        break;
    case JGLAYOUT_GRIDBAG:
        layout->layoutFunc = JGGridBagLayoutFunc;
        layout->gbg.gridBcs = NULL;
        layout->gbg.gridBcCount = 0;
        layout->gbg.gridBcCapacity = 0;
        layout->gbg.defaultGridBc = (JGGRIDBC) {
            .gridX = -1,
            .gridY = -1,
            .gridWidth = 1,
            .gridHeight = 1,
        };
        break;
    default:
        return 0;
    }
    return 1;
}
