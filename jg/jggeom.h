#ifndef __JGGEOM_H__
#define __JGGEOM_H__

// also includes some math

#include "jgdefs.h"

typedef struct PointTag {
    int32_t x;
    int32_t y;
} JGPOINT;

typedef struct SizeTag {
    int32_t width;
    int32_t height;
} JGSIZE;

typedef struct RectTag {
    union {
        struct {
            int32_t left;
            int32_t top;
        };
        JGPOINT point1;
    };
    union {
        struct {
            int32_t right;
            int32_t bottom;
        };
        JGPOINT point2;
    };
} JGRECT, JGRECTANGLE, JGELL, JGELLIPSE, JGINSETS;

void JGSetRect(JGRECT*, int32_t, int32_t, int32_t, int32_t);
void JGMoveRect(JGRECT*, int32_t, int32_t);
void JGResizeRect(JGRECT*, int32_t, int32_t);
bool JGRectContains(const JGRECT*, const JGPOINT*);
bool JGRectsIntersect(const JGRECT*, const JGRECT*);
bool JGRectIntersection(const JGRECT*, const JGRECT*, JGRECT*);

#define JGSetEll JGSetRect
#define JGMoveEll JGMoveRect
#define JGResizeEll JGResizeRect
bool JGEllipseContains(const JGELLIPSE*, const JGPOINT*);
bool JGEllipsesIntersect(const JGELLIPSE*, const JGELLIPSE*);

typedef struct LineTag {
    union {
        struct {
            int32_t x1;
            int32_t y1;
        };
        JGPOINT point1;
    };
    union {
        struct {
            int32_t x2;
            int32_t y2;
        };
        JGPOINT point2;
    };
} JGLINE;

void JGSetLine(JGLINE*, int32_t, int32_t, int32_t, int32_t);
bool JGLinesIntersect(const JGLINE*, const JGLINE*);
bool JGLineIntersection(const JGLINE*, const JGLINE*, JGPOINT*);
bool JGLineRectIntersect(const JGRECT*, const JGRECT*);
bool JGLineRectIntersection(const JGRECT*, const JGLINE*, JGLINE*);
bool JGClipLineRect(const JGLINE*, const JGRECT*, JGLINE*);

// floating point math
#include <math.h>

typedef struct Size2DTag {
    double width;
    double height;
} JGSIZE2D;

typedef struct Vec2DTag {
    double x;
    double y;
} JGVECTOR2D, JGVEC2D, JGPOINT2D;

typedef struct Rect2DTag {
    union {
        struct {
            double left;
            double top;
        };
        JGPOINT2D point1;
    };
    union {
        struct {
            double right;
            double bottom;
        };
        JGPOINT2D point2;
    };
} JGRECT2D, JGRECTANGLE2D, JGELL2D, JGELLIPSE2D;

void JGSetRect2D(JGRECT2D*, double, double, double, double);
void JGMoveRect2D(JGRECT2D*, double, double);
void JGResizeRect2D(JGRECT2D*, double, double);
bool JGRect2DContains(const JGRECT2D*, const JGPOINT2D*);
bool JGRect2DIntersect(const JGRECT2D*, const JGRECT2D*);
bool JGRect2DIntersection(const JGRECT2D*, const JGRECT2D*, JGRECT2D*);

#define JGSetEll2D JGSetRect2D
#define JGMoveEll2D JGMoveRect2D
#define JGResizeEll2D JGResizeRect2D
bool JGEll2DContains(const JGELLIPSE2D*, const JGPOINT2D*);
bool JGEll2DIntersect(const JGELLIPSE2D*, const JGELLIPSE2D*);
bool JGCircle2DIntersect(const JGELLIPSE2D*, const JGELLIPSE2D*);

typedef struct Line2DTag {
    union {
        struct {
            double x1;
            double y1;
        };
        JGPOINT2D point1;
    };
    union {
        struct {
            double x2;
            double y2;
        };
        JGPOINT2D point2;
    };
} JGLINE2D;

void JGSetLine2D(JGLINE2D*, double, double, double, double);
bool JGLine2DIntersect(const JGLINE2D*, const JGLINE2D*);
bool JGLine2DIntersection(const JGLINE2D*, const JGLINE2D*, JGPOINT2D*);

void JGVecPoints(JGVECTOR2D*, const JGVECTOR2D*, const JGVECTOR2D*);
void JGVecAddf(JGVECTOR2D*, double, double);
void JGVecSubf(JGVECTOR2D*, double, double);
void JGVecMulf(JGVECTOR2D*, double, double);
void JGVecDivf(JGVECTOR2D*, double, double);
void JGVecAdd(const JGVECTOR2D*, const JGVECTOR2D*, JGVECTOR2D*);
void JGVecSub(const JGVECTOR2D*, const JGVECTOR2D*, JGVECTOR2D*);
void JGVecMult(const JGVECTOR2D*, const JGVECTOR2D*, JGVECTOR2D*);
void JGVecDiv(const JGVECTOR2D*, const JGVECTOR2D*, JGVECTOR2D*);
void JGVecSetAngle(JGVECTOR2D*, double);
#define JGVecRotate(vec, a_) JGVectfSetAngle(vec, (vec)->a += a_)
double JGVecSetMagnitude(JGVECTOR2D*, double);
#define JGVecNormalize(vec) JGVectSetMagnitude(vec, 1.0d)

#endif // __JGGEOM_H__
