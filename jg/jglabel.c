#include "jg.h"

//////////////////////////////////
//           LABEL              //
//////////////////////////////////

static void JGLabelPainter(JGCONTROL comp, JGGRAPHICS g)
{
    if(!(comp->state & JGCOMP_STATE_NOTDRAWBG))
        JGFillRect(g, &comp->rect);
    // suspended until further notice
    /*JGTEXT text = comp->text;
    HFONT font = CreateFont(comp->height, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    HFONT oldFont = SelectFont(g->dc, font);
    JGDrawCenteredText(g, text->text, text->len, comp->x, comp->y, comp->width, comp->height);
    SelectObject(g->dc, oldFont);*/
}

JGCOMPONENT JGCreateLabel(string_t text)
{
    JGCOMPONENT comp = JGCreateComponent(JGCOMP_TYPE_BUTTON, 0, NULL, JGLabelPainter);
    JGSetString(&comp->text, text);
    return comp;
}
