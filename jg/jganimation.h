#ifndef __JGANIMATION_H__
#define __JGANIMATION_H__

#include "jgdefs.h"

typedef enum {
    JGKFH_START,
    JGKFH_TICK,
    JGKFH_END
} jgam_t;

typedef enum {
    JGKFH_OK,
    JGKFH_SKIP,
    JGKFH_STOP,
    JGKFH_PAUSE,
    JGKFH_SETTIME,
    JGKFH_SETFRAME
} jgar_t;

struct keyframe;

typedef jgar_t (*JGKEYFRAMEHANDLE)(jgam_t, time_t, uint32_t, struct keyframe*);

typedef struct keyframe {
    time_t runtime;
    time_t duration;
    void *param;
    JGKEYFRAMEHANDLE handle;
} JGKEYFRAME;

void JGKeyFrame_Init(JGKEYFRAME*, time_t, void*, JGKEYFRAMEHANDLE);

typedef struct {
    JGKEYFRAME *frames;
    uint32_t frameCount;
    uint32_t frameIndex;
} JGANIMATION;

void JGAnimation_Init(JGANIMATION*, const JGKEYFRAME*, uint32_t);
void JGAnimation_Play(JGANIMATION*);
void JGAnimation_Expell(JGANIMATION*);
bool JGAnimation_Update(JGANIMATION*, time_t);
void JGAnimation_Forward(time_t);

#endif // __JGANIMATION_H__
