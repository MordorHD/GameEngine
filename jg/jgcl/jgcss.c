#include "jgxml.h"
#include "../jg.h"
#include "../jgutil.h"
#include <stdlib.h>

static struct {
    const char *name;
    int type;
    void *val;
} JGVARTABLE[] = {
    { "black", JGTOKEN_NUMBER, (void*) JGBLACK },
    { "dkgray", JGTOKEN_NUMBER, (void*) JGDKGRAY },
    { "gray", JGTOKEN_NUMBER, (void*) JGGRAY },
    { "white", JGTOKEN_NUMBER, (void*) JGWHITE },
    { "blue", JGTOKEN_NUMBER, (void*) JGBLUE },
    { "green", JGTOKEN_NUMBER, (void*) JGGREEN },
    { "red", JGTOKEN_NUMBER, (void*) JGRED },
    { "pink", JGTOKEN_NUMBER, (void*) JGPINK },
    { "cyan", JGTOKEN_NUMBER, (void*) JGCYAN },
    { "yellow", JGTOKEN_NUMBER, (void*) JGYELLOW },
    { "purple", JGTOKEN_NUMBER, (void*) JGPURPLE },
    { "orange", JGTOKEN_NUMBER, (void*) JGORANGE }
};

JGCSS *JGCSS_Read(FILE *file)
{
    JGCSS *css;
    JGCSSCLASS *cssClass;
    JGCSSPROPERTY cssProp;
    JGTOKEN tok;
    char ch;
    char classBuf[2048];
    char *str;
    int classLen;
    int i;
    _Bool quote = 0;
    _Bool run;
    css = malloc(sizeof(*css));
    css->classes = NULL;
    css->classCount = 0;
    css->classCapacity = 0;
    while((ch = fgetc(file)) != EOF)
    {
        if(isalpha(ch) || ch == '.')
        {
            classBuf[0] = ch;
            classLen = 1;
            while(isalpha(ch = fgetc(file)))
                classBuf[classLen++] = ch;
            while((ch = fgetc(file)) != EOF && ch != '{');
            if(ch != '{')
                goto error;
            i = css->classCount++;
            if(css->classCount > css->classCapacity)
            {
                css->classCapacity *= 2;
                css->classCapacity++;
                css->classes = realloc(css->classes, sizeof(*css->classes) * css->classCapacity);
            }
            cssClass = css->classes + i;
            cssClass->name = JGUtil_Createstrcpy(classBuf, classLen);
            cssClass->properties = NULL;
            cssClass->propertyCount = 0;
            cssClass->propertyCapacity = 0;
            printf("%s\n", cssClass->name);
            classLen = 0;
            run = 1;
            while(run)
            {
                ch = fgetc(file);
                while(ch != ';' && ch != EOF)
                {
                    if(ch == '}')
                    {
                        run = 0;
                        break;
                    }
                    if(ch == '\"')
                        quote = !quote;
                    if(quote || !isspace(ch))
                        classBuf[classLen++] = ch;
                    ch = fgetc(file);
                }
                if(ch == EOF)
                    goto error;
                classBuf[classLen] = 0;
                classLen = 0;
                str = classBuf;
                while((str = JGToken_Next(str, &tok)) && ((tok.type == JGTOKEN_CHAR && tok.val == '-') || tok.type == JGTOKEN_IDENTIFIER));
                if(!str)
                    break;
                if(tok.type != JGTOKEN_CHAR || tok.val != ':')
                    goto error;
                cssProp.name = JGUtil_Createstrcpy(classBuf, tok.tokenStart - classBuf);
                printf("\"%s\" -> ", cssProp.name);
                if(!(str = JGToken_Next(str, &tok)))
                    goto error;
                cssProp.type = tok.type;
                switch(tok.type)
                {
                case JGTOKEN_IDENTIFIER:
                    for(i = 0; i < sizeof(JGVARTABLE) / sizeof(*JGVARTABLE); i++)
                    {
                        if(!JGUtil_Strcmp0(tok.tokenStart, tok.tokenLength, JGVARTABLE[i].name))
                        {
                            cssProp.type = JGVARTABLE[i].type;
                            cssProp.strValue = JGVARTABLE[i].val;
                            printf("0x%x\n", cssProp.intValue);
                            break;
                        }
                    }
                    if(i == sizeof(JGVARTABLE) / sizeof(*JGVARTABLE))
                        goto error;
                    break;
                case JGTOKEN_STRING: cssProp.strValue = JGUtil_Createstrcpy(tok.tokenStart, tok.tokenLength);
                    printf("%s\n", cssProp.strValue);
                    break;
                case JGTOKEN_NUMBER:
                case JGTOKEN_FLOAT:
                    cssProp.intValue = tok.val;
                    printf("0x%x\n", cssProp.intValue);
                    break;
                default:
                    goto error;
                }
                i = cssClass->propertyCount++;
                if(cssClass->propertyCount > cssClass->propertyCapacity)
                {
                    cssClass->propertyCapacity *= 2;
                    cssClass->propertyCapacity++;
                    cssClass->properties = realloc(cssClass->properties, sizeof(*cssClass->properties) * cssClass->propertyCapacity);
                }
                *(cssClass->properties + i) = cssProp;
            }

        }
    }
    return css;
    error:
        JGCSS_Destroy(css);
        return NULL;
}

void JGCSS_Destroy(JGCSS *css)
{
    int propCnt;
    JGCSSPROPERTY *props;
    int cnt;
    JGCSSCLASS *children;
    cnt = css->classCount;
    children = css->classes;
    while(cnt--)
    {
        free(children->name);
        propCnt = children->propertyCount;
        props = children->properties;
        while(propCnt--)
        {
            free(props->name);
            if(props->type == JGTOKEN_STRING)
                free(props->strValue);
            props++;
        }
        free(children->properties);
        children++;
    }
    free(css->classes);
    free(css);
}

void JGCSS_Parse(JGCSS *css)
{
    JGCONTROLSTYLE style;
    JGCONTROLSTYLE defStyle;
    char *pFont;
    char fontBuf[260];
    int fontBufSize;
    int fontSize;
    JGTOKEN tok;
    int cnt = css->classCount;
    JGCSSCLASS *cssClass = css->classes;
    while(cnt--)
    {
        if(*cssClass->name != '.')
            defStyle = JGGetClass(cssClass->name)->style;
        else
            defStyle = (JGCONTROLSTYLE) {
                    .name = "Default",
                    .font = NULL,
                    .colorText = JGWHITE,
                    .colorHover = JGGRAY,
                    .colorBackground = JGDKGRAY,
                    .colorForeground = JGWHITE,
                };
        if(!JGCSS_GetProperty(cssClass, "background-color", &style.colorBackground))
            style.colorBackground = defStyle.colorBackground;
        if(!JGCSS_GetProperty(cssClass, "foreground-color", &style.colorForeground))
            style.colorForeground = defStyle.colorForeground;
        if(!JGCSS_GetProperty(cssClass, "text-color", &style.colorText))
            style.colorText = defStyle.colorText;
        if(!JGCSS_GetProperty(cssClass, "hover-color", &style.colorHover))
            style.colorHover = defStyle.colorHover;
        if(JGCSS_GetProperty(cssClass, "text-font", &pFont))
        {
            printf("GOT FONT: %s\n", pFont);
            fontBufSize = 0;
            while((pFont = JGToken_Next(pFont, &tok)) && tok.type == JGTOKEN_IDENTIFIER)
            {
                memcpy(fontBuf + fontBufSize, tok.tokenStart, tok.tokenLength);
                fontBufSize += tok.tokenLength;
                fontBuf[fontBufSize] = ' ';
                fontBufSize++;
            }
            if(fontBufSize)
            {
                if(tok.type == JGTOKEN_NUMBER)
                {
                    fontSize = tok.val;
                    if((pFont = JGToken_Next(pFont, &tok)) && tok.type == JGTOKEN_IDENTIFIER)
                    {
                        if(!JGToken_Next(pFont, &tok))
                        {
                            fontBuf[--fontBufSize] = 0;
                            style.font = JGCreateFont(fontBuf, fontSize);
                        }
                    }
                }
            }
        }
        else
            style.font = defStyle.font;
        if(*cssClass->name == '.')
            JGAddStyle(cssClass->name + 1, &style);
        else
            JGGetClass(cssClass->name)->style = style;
        cssClass++;
    }
}

JGCSSCLASS *JGCSS_GetClass(JGCSS *css, const char *name, JGCSSCLASS *dest)
{
    int cnt;
    JGCSSCLASS *children;
    cnt = css->classCount;
    children = css->classes;
    while(cnt--)
    {
        if(!strcmp(name, children->name))
        {
            if(dest)
                *dest = *children;
            return children;
        }
        children++;
    }
    return NULL;
}

JGCSSPROPERTY *JGCSS_GetProperty(JGCSSCLASS *cssClass, const char *name, void *dest)
{
    int cnt = cssClass->propertyCount;
    JGCSSPROPERTY *props = cssClass->properties;
    //printf("%d, %I64x\n", cnt, props);
    while(cnt--)
    {
        //printf("%I64x\n", props->name);
        if(!strcmp(name, props->name))
        {
            if(dest)
            {
                switch(props->type)
                {
                case JGTOKEN_STRING: *((char**) dest) = JGUtil_Createstrcpy0(props->strValue); break;
                case JGTOKEN_FLOAT: *((float*) dest) = props->floatValue; break;
                case JGTOKEN_NUMBER: *((int*) dest) = props->intValue; break;
                }
            }
            return props;
        }
        props++;
    }
    return NULL;
}
