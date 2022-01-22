#include "jggeom.h"

void JGSetRect(JGRECT *rect, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

void JGMoveRect(JGRECT *rect, int32_t x, int32_t y)
{
    rect->left += x;
    rect->top += y;
    rect->right += x;
    rect->bottom += y;
}

void JGResizeRect(JGRECT *rect, int32_t x, int32_t y)
{
    rect->right += x;
    rect->bottom += y;
}

bool JGRectContains(const JGRECT *rect, const JGPOINT *point)
{
    return point->x > rect->left && point->x < rect->right
        && point->y > rect->top && point->y < rect->bottom;
}

bool JGRectIntersect(const JGRECT *rect1, const JGRECT *rect2)
{
    return rect1->left < rect2->right  && rect1->right > rect2->left
        && rect1->top < rect2->bottom && rect1->bottom > rect2->top;
}

bool JGRectIntersection(const JGRECT *rect1, const JGRECT *rect2, JGRECT *rectDest)
{
    int32_t left   = __max(rect1->left, rect2->left);
    int32_t top    = __max(rect1->top, rect2->top);
    int32_t right  = __min(rect1->right, rect2->right);
    int32_t bottom = __min(rect1->bottom, rect2->bottom);
    if(right > rect1->left && bottom > rect2->left)
    {
        rectDest->left   = left;
        rectDest->top    = top;
        rectDest->right  = right;
        rectDest->bottom = bottom;
        return 1;
    }
    return 0;
}

void JGSetLine(JGLINE *line, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    line->x1 = x1;
    line->y1 = y1;
    line->x2 = x2;
    line->y2 = y2;
}

bool JGLineIntersect(const JGLINE *line1, const JGLINE *line2)
{
		int32_t s1_x = line1->x2 - line1->x1;
		int32_t s1_y = line1->y2 - line1->y1;
		int32_t s2_x = line2->x2 - line2->x1;
		int32_t s2_y = line2->y2 - line2->y1;

		int32_t s = -s1_y * (line1->x1 - line2->x1) + s1_x * (line1->y1 - line2->y1);
		int32_t t =  s2_x * (line1->y1 - line2->y1) - s2_y * (line1->x1 - line2->x1);

		return s >= 0 && s <= (-s2_x * s1_y + s1_x * s2_y)
            && t >= 0 && t <= (-s2_x * s1_y + s1_x * s2_y);
}

bool JGLineIntersection(const JGLINE *line1, const JGLINE *line2, JGPOINT *pointDest)
{
		int32_t s1_x = line1->x2 - line1->x1;
		int32_t s1_y = line1->y2 - line1->y1;
		int32_t s2_x = line2->x2 - line2->x1;
		int32_t s2_y = line2->y2 - line2->y1;

		int32_t s = -s1_y * (line1->x1 - line2->x1) + s1_x * (line1->y1 - line2->y1);
        int32_t t =  s2_x * (line1->y1 - line2->y1) - s2_y * (line1->x1 - line2->x1);
		int32_t s_ = (-s2_x * s1_y + s1_x * s2_y);
		int32_t t_ = (-s2_x * s1_y + s1_x * s2_y);

		if(s >= 0 && s <= s_
        && t >= 0 && t <= t_)
        {
            pointDest->x = line1->x1 + (t * s1_x) / t_;
            pointDest->y = line1->y1 + (t * s1_y) / t_;
            return 1;
        }
        return 0;
}

#define INSIDE 0x0
#define LEFT 0x1
#define RIGHT 0x2
#define TOP 0x4
#define BOTTOM 0x8

static char ComputeCode(const JGRECT *rect, int32_t x, int32_t y)
{
    char code = INSIDE;

    if(x < rect->left)
        code |= LEFT;
    else if(x > rect->right)
        code |= RIGHT;
    if(y < rect->top)
        code |= BOTTOM;
    else if(y > rect->bottom)
        code |= TOP;

    return code;
}

bool JGClipLineRect(const JGLINE *src, const JGRECT *rect, JGLINE *dest)
{
    int32_t x1 = src->x1;
    int32_t y1 = src->y1;
    int32_t x2 = src->x2;
    int32_t y2 = src->y2;
    char code1 = ComputeCode(rect, x1, y1);
    char code2 = ComputeCode(rect, x2, y2);
    char code_out;
    int32_t x = 0, y = 0;
    int32_t side;

    while(1)
    {
        if(!(code1 | code2))
        {
            dest->x1 = x1;
            dest->y1 = y1;
            dest->x2 = x2;
            dest->y2 = y2;
            return 1;
        }
        if(code1 & code2)
            return 0;

        code_out = code1 ? code1 : code2;
        if(code_out & TOP)
        {
            side = rect->bottom;
            x = x1 + (x2 - x1) * (side - y1) / (y2 - y1);
            y = side;
        }
        else if(code_out & BOTTOM)
        {
            side = rect->top;
            x = x1 + (x2 - x1) * (side - y1) / (y2 - y1);
            y = side;
        }
        else if(code_out & RIGHT)
        {
            side = rect->right;
            y = y1 + (y2 - y1) * (side - x1) / (x2 - x1);
            x = side;
        }
        else if(code_out & LEFT)
        {
            side = rect->left;
            y = y1 + (y2 - y1) * (side - x1) / (x2 - x1);
            x = side;
        }
        if(code1)
        {
            x1 = x;
            y1 = y;
            code1 = ComputeCode(rect, x1, y1);
        }
        else
        {
            x2 = x;
            y2 = y;
            code2 = ComputeCode(rect, x2, y2);
        }
    }
}

bool JGEllContains(const JGELL *e, const JGPOINT *p)
{
    if(!JGRectContains((const JGRECT*) e, p))
        return 0;
    const int32_t width  = e->right  - e->left;
    const int32_t height = e->bottom - e->top;
    int32_t aa = (width * width) >> 2;
    int32_t bb = (height * height) >> 2;
    int32_t x = p->x - e->left - (width >> 1);
    int32_t y = p->y - e->top - (height >> 1);
    return x * x * bb + y * y * aa <= aa * bb;
}

bool JGEllIntersect(const JGELL *e1, const JGELL *e2)
{
    if(!JGRectIntersect((const JGRECT*) e1, (const JGRECT*) e2))
        return 0;
    const int32_t width  = e1->right  - e1->left;
    const int32_t height = e1->bottom - e1->top;
    const int32_t cx = e1->left + (width >> 1);
    const int32_t cy = e1->top + (height >> 1);
    if((e2->left < cx && e2->right > cx) || (e2->top < cy && e2->bottom > cy))
        return 1;
    JGPOINT p;
    p.x = e2->left < cx ? e2->right : e2->left;
    p.x = e2->top < cy ? e2->bottom : e2->top;
    return JGEllContains(e1, &p);
}

void JGSetRect2D(JGRECT2D *rect, double left, double top, double right, double bottom)
{
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

void JGMoveRect2D(JGRECT2D *rect, double x, double y)
{
    rect->left += x;
    rect->top += y;
    rect->right += x;
    rect->bottom += y;
}

void JGResizeRect2D(JGRECT2D *rect, double x, double y)
{
    rect->right += x;
    rect->bottom += y;
}

bool JGRect2DContains(const JGRECT2D *rect, const JGPOINT2D *point)
{
    return point->x > rect->left && point->x < rect->right
        && point->y > rect->top && point->y < rect->bottom;
}

bool JGRect2DIntersect(const JGRECT2D *rect1, const JGRECT2D *rect2)
{
    return rect1->left < rect2->right  && rect1->right > rect2->left
        && rect1->top < rect2->bottom && rect1->bottom > rect2->top;
}

bool JGRect2DIntersection(const JGRECT2D *rect1, const JGRECT2D *rect2, JGRECT2D *rectDest)
{
    double left   = fmax(rect1->left, rect2->left);
    double top    = fmax(rect1->top, rect2->top);
    double right  = fmin(rect1->right, rect2->right);
    double bottom = fmin(rect1->bottom, rect2->bottom);
    if(right > rect1->left && bottom > rect2->left)
    {
        rectDest->left   = left;
        rectDest->top    = top;
        rectDest->right  = right;
        rectDest->bottom = bottom;
        return 1;
    }
    return 0;
}

bool JGEll2DContains(const JGELL2D *e, const JGPOINT2D *p)
{
    if(!JGRect2DContains((const JGRECT2D*) e, p))
        return 0;
    const double width  = e->right  - e->left;
    const double height = e->bottom - e->top;
    const double aa = (width * width) * 0.25d;
    const double bb = (height * height) * 0.25d;
    const double x = p->x - e->left - (width * 0.5d);
    const double y = p->y - e->top - (height * 0.5d);
    return x * x * bb + y * y * aa <= aa * bb;
}

bool JGCircle2DIntersect(const JGELL2D *c1, const JGELL2D *c2)
{
    const double r1 = (c1->right - c1->left) * 0.5;
    const double r2 = (c2->right - c2->left) * 0.5;
    const double sr = r1 + r2;
    const double dx = c1->left + r1 - c2->left - r2;
    const double dy = c1->top  + r1 - c2->top  - r2;
    return dx * dx + dy * dy <= sr * sr;
}

bool JGEll2DIntersect(const JGELL2D *e1, const JGELL2D *e2)
{
    if(!JGRect2DIntersect((const JGRECT2D*) e1, (const JGRECT2D*) e2))
        return 0;
    const double cx = (e1->left + e1->right) * 0.5d;
    const double cy = (e1->top  + e1->bottom) * 0.5d;
    if((e2->left < cx && e2->right > cx) || (e2->top < cy && e2->bottom > cy))
        return 1;
    const JGPOINT2D p = {
        .x = e2->left < cx ? e2->right : e2->left,
        .y = e2->top < cy ? e2->bottom : e2->top
    };
    return JGEll2DContains(e1, &p);
}

void JGSetLine2D(JGLINE2D *line, double x1, double y1, double x2, double y2)
{
    line->x1 = x1;
    line->y1 = y1;
    line->x2 = x2;
    line->y2 = y2;
}

bool JGLine2DIntersect(const JGLINE2D *line1, const JGLINE2D *line2)
{
    const double s1_x = line1->x2 - line1->x1;
    const double s1_y = line1->y2 - line1->y1;
    const double s2_x = line2->x2 - line2->x1;
    const double s2_y = line2->y2 - line2->y1;

    const double s = -s1_y * (line1->x1 - line2->x1) + s1_x * (line1->y1 - line2->y1);
    const double t =  s2_x * (line1->y1 - line2->y1) - s2_y * (line1->x1 - line2->x1);

    return s >= 0 && s <= (-s2_x * s1_y + s1_x * s2_y)
        && t >= 0 && t <= (-s2_x * s1_y + s1_x * s2_y);
}

bool JGLine2DIntersection(const JGLINE2D *line1, const JGLINE2D *line2, JGPOINT2D *pointDest)
{
    const double s1_x = line1->x2 - line1->x1;
    const double s1_y = line1->y2 - line1->y1;
    const double s2_x = line2->x2 - line2->x1;
    const double s2_y = line2->y2 - line2->y1;

    const double s = -s1_y * (line1->x1 - line2->x1) + s1_x * (line1->y1 - line2->y1);
    double t =  s2_x * (line1->y1 - line2->y1) - s2_y * (line1->x1 - line2->x1);
    const double s_ = (-s2_x * s1_y + s1_x * s2_y);
    const double t_ = (-s2_x * s1_y + s1_x * s2_y);

    if(s >= 0 && s <= s_ && t >= 0 && t <= t_)
    {
        t /= t_;
        pointDest->x = line1->x1 + t * s1_x;
        pointDest->y = line1->y1 + t * s1_y;
        return 1;
    }
    return 0;
}

void JGVecPoints(JGVECTOR2D *vecDest, const JGVECTOR2D *v1, const JGVECTOR2D *v2)
{
    const double dx = v2->x - v1->x;
    const double dy = v2->y - v1->y;
    const double s = sqrt(dx * dx + dy * dy);
    vecDest->x = dx / s;
    vecDest->y = dy / s;
}

void JGVecAddf(JGVECTOR2D *vec, double x, double y)
{
    vec->x += x;
    vec->y += y;
}

void JGVecSubf(JGVECTOR2D *vec, double x, double y)
{
    vec->x -= x;
    vec->y -= y;
}

void JGVecMulf(JGVECTOR2D *vec, double x, double y)
{
    vec->x *= x;
    vec->y *= y;
}

void JGVecDivf(JGVECTOR2D *vec, double x, double y)
{
    vec->x /= x;
    vec->y /= y;
}

void JGVecAdd(const JGVECTOR2D *vec1, const JGVECTOR2D *vec2, JGVECTOR2D *vecDest)
{
    vecDest->x = vec1->x + vec2->x;
    vecDest->y = vec1->y + vec2->y;
}

void JGVecSub(const JGVECTOR2D *vec1, const JGVECTOR2D *vec2, JGVECTOR2D *vecDest)
{
    vecDest->x = vec1->x - vec2->x;
    vecDest->y = vec1->y - vec2->y;
}

void JGVecMult(const JGVECTOR2D *vec1, const JGVECTOR2D *vec2, JGVECTOR2D *vecDest)
{
    vecDest->x = vec1->x * vec2->x;
    vecDest->y = vec1->y * vec2->y;
}

void JGVecDiv(const JGVECTOR2D *vec1, const JGVECTOR2D *vec2, JGVECTOR2D *vecDest)
{
    vecDest->x = vec1->x / vec2->x;
    vecDest->y = vec1->y / vec2->y;
}

void JGVecSetAngle(JGVECTOR2D *vec, double a)
{
    const double s = sqrt(vec->x * vec->x + vec->y * vec->y);
    sincos(a, &vec->x, &vec->y);
    vec->x *= s;
    vec->y *= s;
}

double JGVecSetMagnitude(JGVECTOR2D *vec, double m)
{
    const double s = m / sqrt(vec->x * vec->x + vec->y * vec->y);
    vec->x *= s;
    vec->y *= s;
    return s;
}
