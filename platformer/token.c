#include "token.h"
#include <string.h>
#include <stdlib.h>

int strcasecmp0(const char *str, int len, const char *str0)
{
    while(*str0)
    {
        if(!len)
            return -1;
        if(tolower(*str) != tolower(*str0))
            return -1;
        len--;
        str++;
        str0++;
    }
    return len;
}

int strcmp0(const char *str, int len, const char *str0)
{
    while(*str0)
    {
        if(!len)
            return -1;
        if(*str != *str0)
            return -1;
        len--;
        str++;
        str0++;
    }
    return len;
}

char *CreateStringCopy(const char *str, int len)
{
    char *ret = malloc(len + 1);
    memcpy(ret, str, len);
    *(ret + len) = 0;
    return ret;
}

char *CreateStringCopy0(const char *str0)
{
    return CreateStringCopy(str0, strlen(str0));
}

char *ReadTokenType(char *text, token_t type, TOKEN *tok)
{
    if(!(text = NextToken(text, tok)) || tok->type != type)
        return NULL;
    return text;
}

char *NextToken(char *str, TOKEN *tok)
{
    int len;
    char ch;
    int val;
    float fVal;
    char *pEnd;
    _Bool meta;
    tok->type = 0;
    if(!str)
        return 0ll;
    ch = *(str++);
    while(isspace(ch))
    {
        if(ch == '\n')
            tok->lines++;
        ch = *(str++);
    }
    if(!ch)
        return 0ll;
    tok->tokenStart = str - 1;
    len = 1;
    if(isalpha(ch) || ch == '_')
    {
        tok->val = ch;
        while(isalnum(*str) || *str == '_')
        {
            str++;
            len++;
        }
        tok->type = TT_IDENTIFIER;
    }
    else if(isdigit(ch))
    {
        val = 0;
        switch(*str)
        {
        case 'X':
        case 'x':
            while(1)
            {
                ch = *(++str);
                if(isdigit(ch))
                    ch -= '0';
                else if(isalpha(ch))
                    ch = toupper(ch) - 'A' + 10;
                else
                    break;
                val <<= 4;
                val |= ch;
            }
            break;
        case 'B':
        case 'b':
            while(1)
            {
                ch = *(++str);
                val <<= 1;
                if(ch == '1')
                    val |= 1;
                else if(ch != '0')
                    break;
            }
            break;
        default:
            val = ch - '0';
            while(isdigit(ch = *str))
            {
                val *= 10;
                val += ch - '0';
                str++;
            }
            // float
            if(*str == '.')
            {
                fVal = strtof(str, &pEnd);
                if(pEnd > str)
                {
                    tok->type = TT_FLOAT;
                    fVal += val;
                    tok->val = *(int*) &fVal;
                    tok->tokenLength = len + pEnd - str;
                    return pEnd;
                }
            }
        }
        tok->type = TT_INT;
        tok->val = val;
    }
    else switch(ch)
    {
    case '\'':
        tok->val = *(str++);
        if(*str != '\'')
            return NULL;
        str++;
        tok->type = TT_INT;
        len = 3;
        break;
    case '\"':
        tok->val = ch;
        len = 0;
        tok->tokenStart = str;
        meta = 0;
        while(*str && (*str != '\"' || meta))
        {
            if(*str == '\\')
                meta = !meta;
            str++;
            len++;
        }
        if(*str)
            str++;
        tok->type = TT_STRING;
        break;
    case ',': case ';':
        tok->type = TT_SEPARATOR;
        tok->val = ch;
        break;
    default:
        tok->type = TT_CHAR;
        tok->val = ch;
    }
    tok->tokenLength = len;
    return str;
}
