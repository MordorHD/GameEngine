#include "jgtext.h"

#include <string.h>

char *JGGetString(JGTEXT *text, char * restrict buf, int bufLen)
{
    if(buf == NULL || bufLen == 0)
        return NULL;
    int len = bufLen <= text->length ? bufLen - 1 : text->length;
    memcpy(buf, text->text, len);
    buf[len] = 0;
    return buf;
}

bool JGSetString(JGTEXT *text, string_t str)
{
    int len;
    if(str && (len = strlen(str)))
    {
        text->length = text->capacity = len;
        strcpy(text->text = malloc(len), str);
        return 1;
    }
    text->capacity = 12;
    text->length = 0;
    text->text = NULL;
    return 0;
}

bool JGAppendString(JGTEXT *text, string_t apd)
{
    return JGInsertString(text, apd, text->length);
}

bool JGInsertString(JGTEXT *text, string_t ins, int index)
{
    int len;
    if(ins == NULL || !(len = strlen(ins)))
        return 0;
    int textLen = text->length;
    int newLen = textLen + len;
    if(newLen > text->capacity)
    {
        text->text = realloc(text->text, len + (text->capacity *= 2));
        text->length = newLen;
    }
    memmove(text->text + index + len, text->text + index, textLen - index);
    memcpy(text->text + index, ins, len);
    return 1;
}

bool JGRemoveCharAt(JGTEXT *text, int index)
{
    return JGRemoveRangeAt(text, index, 1);
}

bool JGRemoveRangeAt(JGTEXT *text, int fromIndex, int removeLen)
{
    if(!removeLen)
        return 0;
    memmove(text->text + fromIndex, text->text + fromIndex + removeLen, text->length - fromIndex - removeLen);
    text->length -= removeLen;
    return 1;
}

int JGFindChar(JGTEXT *text, char ch, int fromIndex)
{
    char *str = text->text + fromIndex;
    int len = text->length - fromIndex;
    while(len--)
        if(*(str++) == ch)
            return text->length - len - 1;
    return -1;
}

static inline int JGFindString0(char *restrict str, int len, string_t restrict find, int findLen, int fromIndex)
{
    int sLen = len;
    int matchCnt = 0;
    while(len--)
    {
        if(*(find + matchCnt) == *str)
        {
            matchCnt++;
            if(matchCnt == findLen)
                return sLen - len - matchCnt;
        }
        else if(matchCnt != 0)
        {
            if(*find == *str)
                matchCnt = 1;
            else
                matchCnt = 0;
        }
        str++;
    }
    return -1;
}

int JGFindString(JGTEXT *text, string_t restrict find, int fromIndex)
{
    int fLen;
    if(find == NULL || !(fLen = strlen(find)))
        return -1;
    return JGFindString0(text->text, text->length, find, fLen, fromIndex);
}

int JGReplaceString(JGTEXT *text, string_t restrict find, int fromIndex, string_t restrict replace)
{
    int fLen;
    if(find == NULL || !(fLen = strlen(find)))
        return -1;
    int index;
    if((index = JGFindString0(text->text, text->length, find, fLen, fromIndex)) < 0)
        return -1;
    int rLen = strlen(replace);
    int aLen = fLen - rLen;
    memmove(text->text + index, text->text + index + aLen, text->length - index - aLen);
    memcpy(text->text + index, replace, rLen);
    text->length -= aLen;
    return index;
}
