#include "jg.h"

//////////////////////////////////
//          SLIDER              //
//////////////////////////////////

static void JGSliderPainter(JGCOMPONENT comp, JGGRAPHICS g)
{
    if(!(comp->state & JGCOMP_STATE_NOTDRAWBG))
    {
        JGCOLORPALETTE pal = g->palette;
        color_t old = JGSetFillColor(g, pal.bgB0Color);
        JGRECT r = comp->rect;
        JGFillRect(g, &r);

        r.x += comp->slider.pos;
        r.width = JGSLIDER_SLIDER_WIDTH;
        JGSetFillColor(g, pal.bgC0Color);
        JGFillRect(g, &r);
        JGSetFillColor(g, old);
    }
    // suspended until further notice
    /*JGTEXT text = comp->text;
    HFONT font = CreateFont(comp->height, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    HFONT oldFont = SelectFont(g->dc, font);
    JGDrawCenteredText(g, text->text, text->len, comp->x, comp->y, comp->width, comp->height);
    SelectObject(g->dc, oldFont);*/
}

static int JGSliderListener(JGCOMPONENT comp, CLP(JGEVENT) event)
{
    JGSLIDER *slider = &comp->slider;
    switch(event->id)
    {
    case JGEVENT_ID_MOUSEPRESSED:
    {
        JGRECT r = comp->rect;
        r.x += slider->pos;
        r.width = JGSLIDER_SLIDER_WIDTH;
        if(JGRectContains(&r, &event->mousePos))
            comp->state |= JGCOMP_STATE_SLIDER_MOUSEIN;
        else if(comp->state & JGCOMP_STATE_MOUSEIN)
        {
            slider->pos += (event->x < r.x ? -1 : 1) * comp->width / JGSLIDER_INCREMENT_FACTOR;
            slider->pos = imax(slider->pos, 0);
            slider->pos = imin(slider->pos, comp->width - JGSLIDER_SLIDER_WIDTH);
            comp->state |= JGCOMP_STATE_REDRAW;
        }
        break;
    }
    case JGEVENT_ID_MOUSEMOVED:
        if((comp->state & JGCOMP_STATE_SLIDER_MOUSEIN) && event->x > comp->x && event->x < comp->x + comp->width)
        {
            unit_t dx = event->dx;
            slider->pos += dx;
            slider->pos = imax(slider->pos, 0);
            slider->pos = imin(slider->pos, comp->width - JGSLIDER_SLIDER_WIDTH);
            comp->state |= JGCOMP_STATE_REDRAW;
        }
        break;
    case JGEVENT_ID_MOUSERELEASED:
        comp->state &= ~JGCOMP_STATE_SLIDER_MOUSEIN;
        break;
    case JGEVENT_ID_SIZE:
        slider->pos *= comp->width / (event->oldSizeX == 0 ? (slider->maxValue - slider->minValue) : event->oldSizeX);
        break;
    }
    return 0;
}

JGCOMPONENT JGCreateSlider(unit_t min, unit_t v, unit_t max)
{
    JGCOMPONENT comp = JGCreateComponent(JGCOMP_TYPE_SLIDER, JGCOMP_STATE_PRESSED, JGSliderListener, JGSliderPainter);
    JGSLIDER slider = { .minValue = min, .maxValue = max, .pos = v };
    comp->slider = slider;
    return comp;
}
