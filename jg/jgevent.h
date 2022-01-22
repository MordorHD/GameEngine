#ifndef __JGEVENT_H__
#define __JGEVENT_H__

#include "jgdefs.h"
#include "jggeom.h"

enum {
    JGEVENT_CREATE,
    JGEVENT_DESTROY,

    JGEVENT_SIZE,
    JGEVENT_POS,
    JGEVENT_MOVE,

    JGEVENT_FORWARD_MIN,

    JGEVENT_SETCURSOR,
    JGEVENT_REDRAW,

    JGEVENT_KEYPRESSED,
    JGEVENT_KEYRELEASED,
    JGEVENT_KEYTYPED,

    JGEVENT_MOUSEPRESSED,
    JGEVENT_MOUSERELEASED,
    JGEVENT_MOUSEMOVED,
    JGEVENT_MOUSEDRAGGED,
    JGEVENT_MOUSEWHEEL,

    JGEVENT_FORWARD_MAX,

    JGEVENT_HIDE,
    JGEVENT_SHOW,

    JGEVENT_HOVER,
    JGEVENT_LEAVE,

    JGEVENT_PRESS,
    JGEVENT_TOGGLE,

    JGEVENT_FOCUSGAINED,
    JGEVENT_FOCUSLOST,
} event_t;

#define JGEVENT_MB_LEFT 0x1
#define JGEVENT_MB_RIGHT 0x2
#define JGEVENT_MB_MIDDLE 0x10
#define JGEVENT_MB_X1 0x20
#define JGEVENT_MB_X2 0x40

typedef struct EventTag {
    int id; // 4
    time_t time; // 12
    int flags; // 16
    union {
        // mouse event
        struct {
            union {
                JGPOINT mousePos;
                struct {
                    int32_t x; // 16
                    int32_t y; // 20
                };
            };
            int pressedButton; // 24
            union {
                // mouse wheel
                int deltaWheel; // 28
                // mouse moved
                union {
                    JGPOINT mouseMotion;
                    struct {
                        int32_t dx; // 28
                        int32_t dy; // 32
                    };
                };
            };
        };
        // key event
        struct {
            int vkCode; // 16
            int keyChar; // 20
        };
    };
} JGEVENT;

#endif // __JGEVENT_H__
