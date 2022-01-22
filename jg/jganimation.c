#include "jganimation.h"
#include <stdlib.h>
#include <string.h>

void JGKeyFrame_Init(JGKEYFRAME *keyFrame, time_t duration, void *param, JGKEYFRAMEHANDLE handle)
{
    keyFrame->runtime = 0;
    keyFrame->duration = duration;
    keyFrame->param = param;
    keyFrame->handle = handle;
}

void JGAnimation_Init(JGANIMATION *animation, const JGKEYFRAME *frames, uint32_t frameCount)
{
    animation->frames = malloc(sizeof(*frames) * frameCount);
    memcpy(animation->frames, frames, sizeof(*frames) * frameCount);
    animation->frameCount = frameCount;
    animation->frameIndex = 0;
}

bool JGAnimation_Update(JGANIMATION *animation, time_t diff)
{
    if(!diff)
        return 1;
    int currentIndex = animation->frameIndex;
    JGKEYFRAME *currentFrame = animation->frames + currentIndex;
    time_t runtime = currentFrame->runtime;
    time_t duration = currentFrame->duration;
    if(runtime >= duration)
        return 0;
    time_t newRuntime = runtime + diff;
    timediff_t overflow = newRuntime - duration;
    while(overflow >= 0)
    {
        currentFrame->handle(JGKFH_END, duration - runtime, currentIndex, currentFrame);
        currentFrame->runtime = currentFrame->duration;
        if(animation->frameIndex + 1 == animation->frameCount)
            return 0;
        currentIndex = ++animation->frameIndex;
        currentFrame++;
        currentFrame->handle(JGKFH_START, 0, currentIndex, currentFrame);
        currentFrame->runtime = 0;
        if(!overflow)
            return 1;
        duration = currentFrame->duration;
        newRuntime = diff = overflow;
        overflow = newRuntime - duration;
    }
    currentFrame->handle(JGKFH_TICK, diff, currentIndex, currentFrame);
    currentFrame->runtime = newRuntime;
    return 1;
}

static JGANIMATION *ActiveAnimations = NULL;
static uint32_t ActiveAnimationCount = 0;

void JGAnimation_Play(JGANIMATION *animation)
{
    uint32_t newCnt = ActiveAnimationCount + 1;
    ActiveAnimations = realloc(ActiveAnimations, newCnt * sizeof(*animation));
    *(ActiveAnimations + ActiveAnimationCount) = *animation;
    ActiveAnimationCount = newCnt;
    JGKEYFRAME *currentFrame = animation->frames;
    currentFrame->handle(JGKFH_START, 0, 0, currentFrame);
}

void JGAnimation_Forward(time_t diff)
{
    uint32_t cnt = ActiveAnimationCount;
    JGANIMATION *animation = ActiveAnimations;
    while(cnt--)
    {
        JGAnimation_Update(animation, diff);
        animation++;
    }
}
