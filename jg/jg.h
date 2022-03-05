#ifndef __JG_H__
#define __JG_H__

#include "jglayout.h"
#include "jganimation.h"
#include "jgevent.h"
#include "jggeom.h"
#include "jgtext.h"
#include "jgutil.h"

#ifdef __WIN32
#include <windows.h>
#include <windowsx.h>

#define JGDestroyImage(img) DeleteDC((img)->imageDc), DeleteDC((img)->flippedDc), free(img)
#define JGImageWidth(img) ((img)->bmp.bmWidth)
#define JGImageHeight(img) ((img)->bmp.bmHeight)

typedef HFONT JGFONT;
typedef HCURSOR JGCURSOR;

#define JGGetKeyState(vk) GetKeyState(vk)

#define JGRGB(r, g, b) (color_t) RGB(r, g, b)

#define JGALIGN_LEFT TA_LEFT
#define JGALIGN_TOP TA_TOP
#define JGALIGN_RIGHT TA_RIGHT
#define JGALIGN_BOTTOM TA_BOTTOM
#define JGALIGN_CENTER TA_CENTER

#define JGSTROKE_STYLE_SOLID PS_SOLID
#define JGSTROKE_STYLE_DASH PS_DASH

#define JGBLACK 0xFF000000
#define JGDKGRAY 0xFF444444
#define JGGRAY 0xFF999999
#define JGWHITE 0xFFFFFFFF
#define JGBLUE 0xFFFF0000
#define JGGREEN 0xFF00FF00
#define JGRED 0xFF0000FF
#define JGPINK 0xFFFF00FF
#define JGCYAN 0xFFFFFF00
#define JGYELLOW 0xFF00FFFF
#define JGPURPLE 0xFFFF0087
#define JGORANGE 0xFF0087FF
#define JGSKYBLUE 0xFF87CEEB
#define JGSKYRED 0xFFBF4C1A
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
#define JGSKYBLUE 0x87CEEB
#define JGSKYRED 0xBF4C1A

#endif // __WIN32

typedef struct {
    color_t *pixels;
    int width;
    int height;
} *JGIMAGE;

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

void JGSetCursor(JGCURSOR);
JGCURSOR JGGetCursor(void);
JGCURSOR JGLoadCursor(cursor_t);

#define JGCONSUME 0x10

#define JGCA_LEFT 0x0
#define JGCA_TOP 0x0
#define JGCA_HCENTER 0x1
#define JGCA_RIGHT 0x2
#define JGCA_VCENTER 0x4
#define JGCA_BOTTOM 0x8
#define JGCA_FTARGET 0x10

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
    void *target;
} JGCAMERA;

void JGCameraTranslate(float, float);
void JGSetCameraPos(float, float);
void JGSetCameraTarget(void*);
void JGSetCamera(JGCAMERA*);
void JGGetCamera(JGCAMERA*);
void JGGetCameraPos(JGPOINT2D*);
void JGResetCamera(void);

JGIMAGE JGCreateImage(int, int, color_t*);
JGIMAGE JGCreateImageSection(const JGIMAGE, int, int, int, int);
JGIMAGE JGLoadImage(const char*);
void JGCopyImage(JGIMAGE, int, int, int, int, JGIMAGE, int, int, int, int);
void JGGetImageSize(const JGIMAGE, JGSIZE*);
bool JGDrawImage(const JGIMAGE, int, int, int, int);
bool JGDrawImageSection(const JGIMAGE, int, int, int, int, int, int, int, int);
bool JGDrawImageFlipped(const JGIMAGE, int, int, int, int);
bool JGDrawImageSectionFlipped(const JGIMAGE, int, int, int, int, int, int, int, int);

enum {
    JGTEXTURE_COLOR,
    JGTEXTURE_IMAGE,
    JGTEXTURE_FILLIMAGE,
    JGTEXTURE_ANIMATED,
};

typedef struct ObjectTextureTag {
    int type;
    union {
        color_t color;
        JGIMAGE image;
    };
    float ratioX, ratioY;
    int offsetX, offsetY;
} *JGTEXTURE, __JGTEXTURE;

JGTEXTURE JGCreateImageTexture(JGIMAGE, float, float, int, int, bool);
JGTEXTURE JGCreateImageTextureEx(string_t, int, int, float, float, int, int, bool);
JGTEXTURE JGCreateColoredTexture(color_t, float, float, int, int);
JGTEXTURE JGCreateAnimatedTexture(JGIMAGE*, int, int, bool, float, float, int, int);
JGTEXTURE JGCreateAnimatedTextureEx(JGIMAGE, int, int, int, int, int, int, int, bool, float, float, int, int);
JGTEXTURE JGCreateAnimatedTextureIndirect(JGANIMATION*);
void JGDestroyTexture(JGTEXTURE);
bool JGDrawTexture(const JGTEXTURE, int, int, int, int);

void JGGetMousePosition(JGPOINT*);
void JGGetPMousePosition(JGPOINT*);
int JGGetMouseX(void);
int JGGetMouseY(void);
int JGGetPMouseX(void);
int JGGetPMouseY(void);

void JGGetWindowSize(JGSIZE*);

color_t *JGGetBuffer(void);
int JGGetBufferWidth(void);
int JGGetBufferHeight(void);
float JGGetBufferStretchX(void);
float JGGetBufferStretchY(void);

void JGInit(int, char**);
void __JGControlInit(void);
void JGSetFPSLimit(double);
void JGSetBufferSize(int, int);
void __JGSetBuffer(color_t*, int, int);
void JGStep(time_t, time_t);
void JGStop(void);
void JGRun(void);
void JGStart(void);
void JGEvent(const JGEVENT*);
struct ControlTag;
struct ControlTag *JGGetMouseHoverControl(void);
struct ControlTag *JGGetMouseFocusControl(void);
struct ControlTag *JGGetKeyboardFocusControl(void);
void JGRequestFocus(struct ControlTag*);
void JGKillFocus(struct ControlTag*);
void JGDraw(void);
void JGSave(void);
void JGRestore(void);
void JGClip(int, int, int, int);
bool JGClipRect(const JGRECT*);
void JGFill(color_t);
#define JGNoFill() JGFill(0)
bool JGGradient(int, int, color_t, int, int, color_t, int);
void JGStroke(color_t);
void JGSetStroke(int, int, color_t);
#define JGNoStroke() JGStroke(0)
void JGFont(string_t, int);
void JGSetFont(JGFONT);
JGFONT JGCreateFont(string_t, int);
int JGTextWidth(string_t, int);
int JGTextHeight(void);
void JGTextColor(color_t);
void JGTextAlign(int);
void JGText(string_t, int, int, int);
#define JGText0(text, x, y) JGText(text, strlen(text), x, y)
bool JGRect(int, int, int, int);
bool JGOval(int, int, int, int);
bool JGLine(int, int, int, int);
void JGResetTransform(void);
void JGTranslate(float, float);
void JGScale(float, float);
void JGRotate(float);
void JGShear(float, float);
void JGRect2D(const JGRECT2D*);
void JGOval2D(const JGELLIPSE2D*);
void JGLine2D(const JGLINE2D*);

typedef enum {
    JGTYPE_STATIC,
    JGTYPE_GROUP,
    JGTYPE_BUTTON,
    JGTYPE_WINDOW,
    JGTYPE_SLIDER,
    JGTYPE_IMAGEVIEW,
    JGTYPE_TEXTINPUT,
    JGTYPE_TEXTAREA,
} control_t;

#define JGSTYLE_TRANSPARENT 0x1
#define JGSTYLE_NOTDRAWBG   0x8
#define JGSTATE_FOCUSED     0x10
#define JGSTATE_DESTRUCT    0x20
#define JGSTYLE_FIXEDWIDTH  0x40
#define JGSTYLE_FIXEDHEIGHT 0x80
#define JGSTYLE_FIXEDSIZE (JGSTYLE_FIXEDWIDTH|JGSTYLE_FIXEDHEIGHT)
#define JGSTYLE_CORNERED    0x100
#define JGSTYLE_INVISIBLE   0x200

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

void JGAddStyle(string_t, const JGCONTROLSTYLE*);
JGCONTROLSTYLE *JGGetStyle(string_t);

typedef struct ControlClassTag {
    char *id;
    int structSize;
    JGEVENTPROC proc;
    union {
        JGCONTROLSTYLE style;
        JGTEXTURE texture;
    };
} JGCLASS;

typedef struct ControlTag {
    JGCLASS *class_;
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
    char *id;
    JGINSETS insets;
    JGCONTROLSTYLE *style;
} *JGCONTROL, __JGCONTROL;

typedef struct StaticTag {
    __JGCONTROL control;
    JGTEXT text;
} *JGSTATIC, __JGSTATIC;

typedef struct GroupTag {
    __JGCONTROL control;
    JGLAYOUT layout;
    struct ControlTag **children;
    uint32_t childrenCount;
    uint32_t childrenCapacity;
} *JGGROUP, __JGGROUP;

#define JGSetHandle(control, handle) ((JGCONTROL) (control))->class_->proc(((JGCONTROL) (control)), JGEVENT_SETHANDLE, (JGEVENT*) (handle))

typedef struct ButtonTag {
    __JGCONTROL control;
    JGTEXT text;
    JGCONTROLHANDLE handle;
} *JGBUTTON, __JGBUTTON;

typedef struct CheckBoxTag {
    __JGCONTROL control;
    JGTEXT text;
    JGCONTROLHANDLE handle;
} *JGCHECKBOX, __JGCHECKBOX;

typedef struct ChoiceTag {
    __JGCONTROL control;
    JGTEXT text;
    JGCONTROLHANDLE handle;
} *JGCHOICE, __JGCHOICE;

#define JGSTATE_SLIDERIN      JGSTATE_RESERVED1
#define JGSTATE_SLIDERPRESSED JGSTATE_RESERVED2

typedef struct SliderTag {
    __JGCONTROL control;
    JGTEXT text;
    JGCONTROLHANDLE handle;
    int sliderPos;
} *JGSLIDER, __JGSLIDER;

#define JGSTYLE_BORDERLESS    JGSTATE_RESERVED1
#define JGSTYLE_XBUTTON       JGSTATE_RESERVED8
#define JGSTATE_RESIZE_LEFT   JGSTATE_RESERVED2
#define JGSTATE_RESIZE_TOP    JGSTATE_RESERVED3
#define JGSTATE_RESIZE_RIGHT  JGSTATE_RESERVED4
#define JGSTATE_RESIZE_BOTTOM JGSTATE_RESERVED5
#define JGSTATE_MOVE          JGSTATE_RESERVED6
#define JGSTATE_MOUSEINCLIENT JGSTATE_RESERVED7
#define JGSTATE_MOUSEINBUTTON JGSTATE_RESERVED9
#define JGSTATE_BUTTONPRESSED JGSTATE_RESERVED10
#define JGSTYLE_RESIZE JGSTATE_RESERVED11

typedef struct WindowTag {
    __JGCONTROL control;
    JGTEXT text;
    JGLAYOUT layout;
    struct ControlTag **children;
    uint32_t childrenCount;
    uint32_t childrenCapacity;
} *JGWINDOW, __JGWINDOW;

#define JGSetImage(control, image) ((JGCONTROL) (control))->class_->proc(((JGCONTROL) (control)), JGEVENT_SETIMAGE, (JGEVENT*) (image))

typedef struct ImageViewTag {
    __JGCONTROL control;
    JGIMAGE image;
    JGRECT cut;
} *JGIMAGEVIEW, __JGIMAGEVIEW;

#define JGSTATE_ACTIVE JGSTATE_RESERVED1

typedef struct RadioButtonTag {
    __JGCONTROL control;
    int group;
} *JGRADIOBUTTON, __JGRADIOBUTTON;

typedef struct TabbedPaneTag {
    __JGCONTROL control;
    JGLIST *tabs;
} *JGTABBEDPANE, __JGTABBEDPANE;

#define JGSTATE_SELECTED JGSTATE_RESERVED1
#define JGSTYLE_LEFT JGSTATE_RESERVED2
#define JGSTYLE_RIGHT JGSTATE_RESERVED3
#define JGSTYLE_TOP JGSTATE_RESERVED4
#define JGSTYLE_BOTTOM JGSTATE_RESERVED5
#define JGSTYLE_VCENTER JGSTATE_RESERVED6
#define JGSTYLE_HCENTER JGSTATE_RESERVED7

#define JGHANDLE_TEXT 2

typedef struct TextInputTag {
    __JGCONTROL control;
    JGTEXT text;
    int caretPos;
    int selectPos;
    JGCONTROLHANDLE handle;
} *JGTEXTINPUT, __JGTEXTINPUT;

typedef struct TextAreaTag {
    __JGCONTROL control;
    JGLIST lines;
    int caretX, caretY;
    int selectX, selectY;
    JGCONTROLHANDLE handle;
} *JGTEXTAREA, __JGTEXTAREA;

uint64_t JGStaticProc(JGCONTROL, uint32_t, JGEVENT*);
uint64_t JGButtonProc(JGCONTROL, uint32_t, JGEVENT*);
uint64_t JGGroupProc(JGCONTROL, uint32_t, JGEVENT*);
uint64_t JGWindowProc(JGCONTROL, uint32_t, JGEVENT*);
uint64_t JGSliderProc(JGCONTROL, uint32_t, JGEVENT*);
uint64_t JGImageViewProc(JGCONTROL, uint32_t, JGEVENT*);
uint64_t JGTextInputProc(JGCONTROL, uint32_t, JGEVENT*);
uint64_t JGTextAreaProc(JGCONTROL, uint32_t, JGEVENT*);
uint64_t JGDefProc(JGCONTROL, uint32_t, JGEVENT*);

void JGAddClass(const JGCLASS*);
JGCLASS *JGGetClass(string_t);
JGCONTROL JGCreateControl(string_t, string_t, int, string_t, int, int, int, int);
void JGAddControl(JGCONTROL);
void JGAddChildControl(JGCONTROL, JGCONTROL);
bool JGRemoveControl(JGCONTROL);
bool JGDestroyControl(JGCONTROL);

JGCONTROL JGGetControl(string_t);

void JGRequestFocus(JGCONTROL);
void JGKillFocus(JGCONTROL);

uint64_t JGDispatchEvent(uint32_t, JGEVENT*);

#define JGSendEvent(control, msg, event) ((JGCONTROL) (control))->class_->proc((JGCONTROL) control, msg, (JGEVENT*) event)

#define JGIsVisible(control) !(((JGCONTROL) control)->state & JGSTYLE_INVISIBLE)

#define JGSetState(control, stFlag, bool) ((bool)?(((JGCONTROL) (control))->state|=stFlag):(((JGCONTROL) (control))->state&=~stFlag))
#define JGSetStateTrue(control, stFlag) (((JGCONTROL) (control))->state|=stFlag)
#define JGSetStateFalse(control, stFlag) (((JGCONTROL) (control))->state&=~stFlag)
#define JGGetState(control, stFlag) (((JGCONTROL) (control))->state&(stFlag))
#define JGSetStyle(control, style) ((JGCONTROL) (control))->style=(style)
#define JGShowControl(control) ((JGCONTROL) (control))->class_->proc(((JGCONTROL) (control)), JGEVENT_SHOW, NULL), ((JGCONTROL) (control))->state &= ~JGSTYLE_INVISIBLE
#define JGHideControl(control) ((JGCONTROL) (control))->class_->proc(((JGCONTROL) (control)), JGEVENT_HIDE, NULL), ((JGCONTROL) (control))->state |= JGSTYLE_INVISIBLE

#define JGSetText(control, str) ((JGCONTROL) (control))->class_->proc((control), JGEVENT_SETTEXT, (JGEVENT*) (str))
#define JGGetText(control, buf, bufLen)

#define JGSetLayout(control, type) ((JGCONTROL) (control))->class_->proc(((JGCONTROL) (control)), JGEVENT_SETLAYOUT, (JGEVENT*) (uintptr_t) (type))
#define JGGetLayout(control) (JGLAYOUT*) ((JGCONTROL) (control))->class_->proc((JGCONTROL) (control), JGEVENT_GETLAYOUT, NULL)
#define JGRelayout(control) ((JGCONTROL) (control))->class_->proc(((JGCONTROL) (control)), JGEVENT_SIZE, 0)
bool JGSetBounds(JGCONTROL, const JGRECT*);
bool JGSetPos(JGCONTROL, JGPOINT);
bool JGSetSize(JGCONTROL, JGSIZE);

void JGMoveControl(JGCONTROL, int32_t, int32_t);
void JGResizeControl(JGCONTROL, int32_t, int32_t);

#endif // __JG_H__

