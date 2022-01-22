#ifndef __JG_H__
#define __JG_H__

#include "jglayout.h"
#include "jganimation.h"
#include "jgevent.h"
#include "jggeom.h"
#include "jgtext.h"

#ifdef __WIN32
#include <windows.h>
#include <windowsx.h>

typedef struct ImageTag {
    HBITMAP hbmp;
    HDC imageDc;
    BITMAP bmp;
    HDC flippedDc;
    HBITMAP flippedHbmp;
} *JGIMAGE;

#define JGDestroyImage(img) DeleteDC((img)->imageDc); DeleteDC((img)->flippedDc); free(img)

typedef HFONT JGFONT;

#define JGRGB(r, g, b) (color_t) RGB(r, g, b)

#define JGALIGN_LEFT TA_LEFT
#define JGALIGN_TOP TA_TOP
#define JGALIGN_RIGHT TA_RIGHT
#define JGALIGN_BOTTOM TA_BOTTOM
#define JGALIGN_CENTER TA_CENTER

#define JGSTROKE_STYLE_SOLID PS_SOLID
#define JGSTROKE_STYLE_DASH PS_DASH

#define JGBLACK 0
#define JGDKGRAY 0x444444
#define JGGRAY 0x999999
#define JGWHITE 0xFFFFFF
#define JGBLUE 0xFF0000
#define JGGREEN 0x00FF00
#define JGRED 0x0000FF
#define JGPINK 0xFF00FF
#define JGCYAN 0xFFFF00
#define JGYELLOW 0x00FFFF
#define JGPURPLE 0xFF0087
#define JGORANGE 0x0087FF
#else
#include <X11/X.h>
#include <X11/Xlib.h>

typedef Font JGFONT;

#define JGRGB(r, g, b) ((color_t) (((unsigned char)(b)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned int)(unsigned char)(r))<<16)))

#define JGBLACK 0
#define JGDKGRAY 0x444444
#define JGGRAY 0x999999
#define JGWHITE 0xFFFFFF
#define JGBLUE 0x0000FF
#define JGGREEN 0x00FF00
#define JGRED 0xFF0000
#define JGPINK 0xFF00FF
#define JGCYAN 0x00FFFF
#define JGYELLOW 0xFFFF00
#define JGPURPLE 0x8700FF
#define JGORANGE 0xFF8700

#endif // __WIN32

typedef enum {
    JGCURSOR_MIN = 0,
    JGCURSOR_DEFAULT = 0,
    JGCURSOR_LR = 1,
    JGCURSOR_UD = 2,
    JGCURSOR_LU_RD = 3,
    JGCURSOR_LD_RU = 4,
    JGCURSOR_ROTATE = 5,
    JGCURSOR_HAND = 6,
    JGCURSOR_CARET = 7,
    JGCURSOR_MOVE = 8,
    JGCURSOR_MAX = 8,
} cursor_t;

void JGSetCursor(cursor_t);

#define JGCONSUME 0x10

#define JGCA_LEFT 0x0
#define JGCA_TOP 0x0
#define JGCA_HCENTER 0x1
#define JGCA_RIGHT 0x2
#define JGCA_VCENTER 0x4
#define JGCA_BOTTOM 0x8

typedef struct CameraTag {
    // camera position
    float x;
    float y;
    // constraints
    int cstr_l;
    int cstr_r;
    int cstr_t;
    int cstr_b;
    // camera align
    int cFlags;
    JGPOINT2D *target;
} JGCAMERA;

void JGCameraTranslate(float, float);
void JGCameraSetTarget(JGPOINT2D*);
void JGSetCamera(JGCAMERA*);
void JGGetCamera(JGCAMERA*);
void JGResetCamera(void);

JGIMAGE JGLoadImage(const char*, bool);

JGIMAGE JGCreateImage(int, int, int*);
JGIMAGE JGGetScreenImage(void);
JGIMAGE JGCreateImageSection(JGIMAGE, int, int, int, int);
void JGGetScreenSize(JGSIZE*);
void JGCopyImage(JGIMAGE, int, int, int, int, JGIMAGE, int, int, int, int);
void JGGetImageSize(const JGIMAGE, JGSIZE*);
void JGDrawImage(JGIMAGE, int, int, int, int);
void JGDrawImageSection(JGIMAGE, int, int, int, int, int, int, int, int);
void JGDrawImageFlipped(JGIMAGE, int, int, int, int);
void JGDrawImageSectionFlipped(JGIMAGE, int, int, int, int, int, int, int, int);

void JGInit(int, char**);
void JGSetFPSLimit(double);
void JGStep(time_t, time_t);
void JGStop(void);
void JGRun(void);
void JGStart(void);
void JGEvent(const JGEVENT*);
void JGDraw(void);
void JGSave(void);
void JGRestore(void);
void JGClip(int, int, int, int);
void JGClipRect(const RECT*);
void JGFill(color_t);
void JGGradient(int, int, color_t, int, int, color_t, int);
void JGNoFill(void);
void JGStroke(color_t);
void JGSetStroke(int, int, color_t);
void JGNoStroke(void);
void JGFont(const char*, int);
void JGSetFont(JGFONT);
JGFONT JGCreateFont(const char*, int);
void JGTextColor(color_t);
void JGBkColor(color_t);
#define JGOPAQUE OPAQUE
#define JGTRANSPARENT TRANSPARENT
void JGBkMode(int);
void JGTextAlign(int);
void JGText(string_t, int, int, int);
#define JGText0(text, x, y) JGText(text, strlen(text), x, y)
void JGRect(int, int, int, int);
void JGOval(int, int, int, int);
void JGLine(int, int, int, int);
void JGResetTransform(void);
void JGTranslate(float, float);
void JGScale(float, float);
void JGRotate(float);
void JGShear(float, float);
void JGRect2D(const JGRECT2D*);
void JGOval2D(const JGELLIPSE2D*);
void JGLine2D(const JGLINE2D*);

void JGGetWindowSize(JGSIZE*);

typedef enum {
    JGTYPE_STATIC,
    JGTYPE_GROUP,
    JGTYPE_BUTTON,
    JGTYPE_SLIDER,
    JGTYPE_WINDOW,
    JGTYPE_IMAGEVIEW
} control_t;

#define JGSTATE_TRANSPARENT 0x1
#define JGSTATE_TOGGLED     0x4
#define JGSTATE_NOTDRAWBG   0x8
#define JGSTATE_FOCUSED     0x10
#define JGSTATE_DESTRUCT    0x20
#define JGSTATE_FIXEDWIDTH  0x40
#define JGSTATE_FIXEDHEIGHT 0x80
#define JGSTATE_FIXEDSIZE (JGSTATE_FIXEDWIDTH|JGSTATE_FIXEDHEIGHT)
#define JGSTATE_CORNERED    0x100
#define JGSTATE_INVISIBLE   0x200

// These bits are used by component bases
#define JGSTATE_RESERVED1  0x00001000
#define JGSTATE_RESERVED2  0x00002000
#define JGSTATE_RESERVED3  0x00004000
#define JGSTATE_RESERVED4  0x00008000
#define JGSTATE_RESERVED5  0x00010000
#define JGSTATE_RESERVED6  0x00020000
#define JGSTATE_RESERVED7  0x00040000
#define JGSTATE_RESERVED8  0x00080000
#define JGSTATE_RESERVED9  0x00100000
#define JGSTATE_RESERVED10 0x00200000
#define JGSTATE_RESERVED11 0x00400000
#define JGSTATE_RESERVED12 0x00800000

struct ControlTag;

typedef uint64_t (*JGEVENTPROC)(struct ControlTag*, uint32_t, JGEVENT*);

#define JGHANDLE_TOGGLED 1

typedef void (*JGCONTROLHANDLE)(struct ControlTag*, uint32_t, const JGEVENT*);

typedef struct ControlStyleTag {
    char *name;
    JGFONT font;
    color_t colorBackground;
    color_t colorForeground;
    color_t colorText;
    color_t colorHover;
} JGCONTROLSTYLE;

void JGSetDefaultStyle(const JGCONTROLSTYLE*);
void JGGetDefaultStyle(JGCONTROLSTYLE*);

void JGAddStyle(string_t, const JGCONTROLSTYLE*);
JGCONTROLSTYLE *JGGetStyle(string_t);

typedef struct ControlTag {
    char *id;
    uint32_t state;
    union {
        JGRECT rect;
        struct {
            int32_t left;
            int32_t top;
            int32_t right;
            int32_t bottom;
        };
    };
    JGINSETS insets;
    JGCONTROLSTYLE *style;
    JGEVENTPROC proc;
} *JGCONTROL, __JGCONTROL;

typedef struct StaticTag {
    __JGCONTROL control;
    JGTEXT text;
} *JGSTATIC, __JGSTATIC;

typedef struct GroupTag {
    __JGCONTROL control;
    JGTEXT padding;
    JGLAYOUT layout;
    struct ControlTag **children;
    uint32_t childrenCount;
    uint32_t childrenCapacity;
} *JGGROUP, __JGGROUP;

#define JGSetHandle(control, handle_) ((JGBUTTON) (control))->handle = (handle_);

typedef struct ButtonTag {
    __JGCONTROL control;
    JGTEXT text;
    JGCONTROLHANDLE handle;
} *JGBUTTON, __JGBUTTON;

#define JGSTATE_SLIDERIN      JGSTATE_RESERVED1
#define JGSTATE_SLIDERPRESSED JGSTATE_RESERVED2

typedef struct SliderTag {
    __JGCONTROL control;
    JGTEXT text;
    JGCONTROLHANDLE handle;
    int sliderPos;
} *JGSLIDER, __JGSLIDER;

#define JGSTATE_BORDERLESS    JGSTATE_RESERVED1
#define JGSTATE_XBUTTON       JGSTATE_RESERVED8
#define JGSTATE_RESIZE_LEFT   JGSTATE_RESERVED2
#define JGSTATE_RESIZE_TOP    JGSTATE_RESERVED3
#define JGSTATE_RESIZE_RIGHT  JGSTATE_RESERVED4
#define JGSTATE_RESIZE_BOTTOM JGSTATE_RESERVED5
#define JGSTATE_MOVE          JGSTATE_RESERVED6
#define JGSTATE_MOUSEINCLIENT JGSTATE_RESERVED7
#define JGSTATE_MOUSEINBUTTON JGSTATE_RESERVED9
#define JGSTATE_BUTTONPRESSED JGSTATE_RESERVED10

typedef struct WindowTag {
    __JGCONTROL control;
    JGTEXT text;
    JGLAYOUT layout;
    struct ControlTag **children;
    uint32_t childrenCount;
    uint32_t childrenCapacity;
} *JGWINDOW, __JGWINDOW;

#define JGSetImage(control, image) ((JGIMAGEVIEW) (control))->image = (image);

typedef struct ImageViewTag {
    __JGCONTROL control;
    JGIMAGE image;
    JGRECT cut;
} *JGIMAGEVIEW, __JGIMAGEVIEW;

JGCONTROL JGCreateControl(control_t, string_t, string_t, int, string_t, int, int, int, int);
void JGAddControl(JGCONTROL);
void JGAddChildControl(JGCONTROL, JGCONTROL);
bool JGRemoveControl(JGCONTROL);
bool JGDestroyControl(JGCONTROL);

JGCONTROL JGGetControl(const char*);

void JGRequestFocus(JGCONTROL);
void JGKillFocus(JGCONTROL);

uint64_t JGDispatchEvent(uint32_t, JGEVENT*);

#define JGSetState(control, stFlag, bool) ((bool)?((control)->state|=stFlag):((control)->state&=~stFlag))
#define JGSetStateTrue(control, stFlag) ((control)->state|=stFlag)
#define JGSetStateFalse(control, stFlag) ((control)->state&=~stFlag)
#define JGGetState(control, stFlag) ((control)->state&(stFlag))
#define JGSetText(control, str) JGSetString(&(((JGSTATIC) (control))->text), (str))
#define JGGetText(control, buf, bufLen) JGGetString(&(((JGSTATIC) (control))->text), buf, bufLen)

#define JGShowControl(control) (control)->proc((control), JGEVENT_SHOW, NULL); (control)->state &= ~JGSTATE_INVISIBLE
#define JGHideControl(control) (control)->proc((control), JGEVENT_HIDE, NULL); (control)->state |= JGSTATE_INVISIBLE

#define JGSetStyle(control, style) (control)->style=(style)

#define JGSetLayout(control, type) JGGetStockLayout(&(((JGGROUP) (control))->layout), (type))
#define JGRelayout(control) (control)->proc((control), JGEVENT_SIZE, 0)
bool JGSetBounds(JGCONTROL, const JGRECT*);
bool JGSetPos(JGCONTROL, JGPOINT);
bool JGSetSize(JGCONTROL, JGSIZE);

void JGMoveControl(JGCONTROL, int32_t, int32_t);

#endif // __JG_H__

