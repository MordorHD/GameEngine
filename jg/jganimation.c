#include "jganimation.h"
#include <stdlib.h>
#include <string.h>

void JGKeyFrame_Init(JGKEYFRAME *keyFrame, time_t duration, void *param, JGKEYFRAMEHANDLE handle)
{
    keyFrame->runtime = 0;
    keyFrame->duration = duration;
    keyFrame->param = param;
    keyFrame->handle = handle;
    keyFrame->nextFrame = NULL;
}

void JGAnimation_Init(JGANIMATION *animation, JGKEYFRAME *frames)
{
    animation->frames = animation->currentFrame = frames;
}

bool JGAnimation_Update(JGANIMATION *animation, time_t diff)
{
    if(!diff)
        return 1;
    JGKEYFRAME *currentFrame = animation->currentFrame;
    time_t runtime = currentFrame->runtime;
    time_t duration = currentFrame->duration;
    if(runtime >= duration)
        return 0;
    time_t newRuntime = runtime + diff;
    timediff_t overflow = newRuntime - duration;
    while(overflow >= 0)
    {
        currentFrame->handle(JGKFH_END, duration - runtime, currentFrame);
        currentFrame->runtime = currentFrame->duration;
        if(!(currentFrame = animation->currentFrame = currentFrame->nextFrame))
            return 0;
        currentFrame->handle(JGKFH_START, 0, currentFrame);
        currentFrame->runtime = 0;
        if(!overflow)
            return 1;
        duration = currentFrame->duration;
        newRuntime = diff = overflow;
        overflow = newRuntime - duration;
    }
    currentFrame->handle(JGKFH_TICK, diff, currentFrame);
    currentFrame->runtime = newRuntime;
    return 1;
}

static JGANIMATION *ActiveAnimations;
static uint32_t ActiveAnimationCount;
static uint32_t ActiveAnimationCapacity;

void JGAnimation_Play(JGANIMATION *animation)
{
    uint32_t cnt = ActiveAnimationCount++;
    if(ActiveAnimationCount > ActiveAnimationCapacity)
    {
        ActiveAnimationCapacity *= 2;
        ActiveAnimationCapacity++;
        ActiveAnimations = realloc(ActiveAnimations, sizeof(*animation) * ActiveAnimationCapacity);
    }
    *(ActiveAnimations + cnt) = *animation;
    JGKEYFRAME *currentFrame = animation->frames;
    currentFrame->handle(JGKFH_START, 0, currentFrame);
}

void JGAnimation_Forward(time_t diff)
{
    uint32_t cnt = ActiveAnimationCount;
    JGANIMATION *animation = ActiveAnimations;
    while(cnt--)
    {
        if(!JGAnimation_Update(animation, diff))
        {
            memmove(animation, animation + 1, sizeof(*animation) * cnt);
            ActiveAnimationCount--;
            continue;
        }
        animation++;
    }
}
