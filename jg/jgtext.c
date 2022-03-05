#include "jgtext.h"

#include <string.h>

char *JGText_Get(JGTEXT *text, char * restrict buf, int bufLen)
{
    int len;
    if(buf == NULL || !bufLen)
        return NULL;
    len = bufLen <= text->length ? bufLen - 1 : text->length;
    memcpy(buf, text->text, len);
    buf[len] = 0;
    return buf;
}

bool JGText_Set0(JGTEXT *text, string_t str)
{
    if(!str)
    {
        free(text->text);
        text->length = text->capacity = 0;
        return 0;
    }
    return JGText_Set(text, str, strlen(str));
}

bool JGText_Set(JGTEXT *text, string_t str, int len)
{
    if(str)
    {
        if(text->capacity < len)
            text->text = realloc(text->text, len);
        text->length = text->capacity = len;
        memcpy(text->text, str, len);
        return 1;
    }
    if(!len)
    {
        free(text->text);
        text->length = text->capacity = 0;
    }
    else if(text->capacity < len)
    {
        free(text->text);
        text->capacity = len;
        text->length = 0;
        text->text = malloc(len);
    }
    return 0;
}

bool JGText_AppendString0(JGTEXT *text, string_t apd)
{
    return JGText_InsertString(text, apd, strlen(apd), text->length);
}

bool JGText_AppendString(JGTEXT *text, string_t apd, int len)
{
    return JGText_InsertString(text, apd, len, text->length);
}

bool JGText_InsertChar(JGTEXT *text, char ch, int index)
{
    char *txt;
    int textLen = text->length++;
    if(text->length > text->capacity)
    {
        text->capacity *= 2;
        text->capacity++;
        text->text = realloc(text->text, text->capacity);
    }
    txt = text->text + index;
    memmove(txt + 1, txt, textLen - index);
    *(text->text + index) = ch;
    return 1;
}

bool JGText_AppendChar(JGTEXT *text, char ch)
{
    return JGText_InsertChar(text, ch, text->length);
}

bool JGText_InsertString0(JGTEXT *text, string_t ins, int index)
{
    return JGText_InsertString(text, ins, strlen(ins), index);
}

bool JGText_InsertString(JGTEXT *text, string_t ins, int len, int index)
{
    char *txt;
    int textLen;
    if(!len)
        return 0;
    textLen = text->length;
    text->length += len;
    if(text->length > text->capacity)
    {
        text->capacity *= 2;
        text->capacity += len;
        text->text = realloc(text->text, text->capacity);
    }
    txt = text->text + index;
    memmove(txt + len, txt, textLen - index);
    if(ins)
        memcpy(txt, ins, len);
    return 1;
}

bool JGText_RemoveCharAt(JGTEXT *text, int index)
{
    return JGText_RemoveRangeAt(text, index, 1);
}

bool JGText_RemoveRangeAt(JGTEXT *text, int fromIndex, int removeLen)
{
    if(!removeLen)
        return 0;
    memmove(text->text + fromIndex, text->text + fromIndex + removeLen, text->length - fromIndex - removeLen);
    text->length -= removeLen;
    return 1;
}

int JGText_FindChar(JGTEXT *text, char ch, int fromIndex)
{
    char *str = text->text + fromIndex;
    int len = text->length - fromIndex;
    while(len--)
        if(*(str++) == ch)
            return text->length - len - 1;
    return -1;
}

static inline int _JGText_FindString(char *restrict str, int len, string_t restrict find, int findLen, int fromIndex)
{
    int sLen, matchCnt;
    sLen = len;
    matchCnt = 0;
    while(len--)
    {
        if(*(find + matchCnt) == *str)
        {
            matchCnt++;
            if(matchCnt == findLen)
                return sLen - len - matchCnt;
        }
        else if(matchCnt)
        {
            matchCnt = *find == *str;
        }
        str++;
    }
    return -1;
}

int JGText_FindString(JGTEXT *text, string_t restrict find, int fromIndex)
{
    int fLen;
    if(!find || !(fLen = strlen(find)))
        return -1;
    return _JGText_FindString(text->text, text->length, find, fLen, fromIndex);
}

int JGText_FindReplaceString(JGTEXT *text, string_t restrict find, int fromIndex, string_t restrict replace)
{
    char *txt;
    int fLen, rLen, aLen;
    int index;
    if(!find || !(fLen = strlen(find)))
        return -1;
    if((index = _JGText_FindString(text->text, text->length, find, fLen, fromIndex)) < 0)
        return -1;
    rLen = strlen(replace);
    aLen = fLen - rLen;
    txt = text->text + index;
    memmove(txt, txt + aLen, text->length - index - aLen);
    memcpy(txt, replace, rLen);
    text->length -= aLen;
    return index;
}
