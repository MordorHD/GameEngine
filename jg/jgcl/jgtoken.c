#include "jgtoken.h"
#include <string.h>
#include <stdlib.h>

char *JGToken_ReadType(char *text, token_t type, JGTOKEN *tok)
{
    if(!(text = JGToken_Next(text, tok)) || tok->type != type)
        return NULL;
    return text;
}

char *JGToken_Next(char *str, JGTOKEN *tok)
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
        tok->type = JGTOKEN_IDENTIFIER;
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
                    tok->type = JGTOKEN_FLOAT;
                    fVal += val;
                    tok->val = *(int*) &fVal;
                    tok->tokenLength = len + pEnd - str;
                    return pEnd;
                }
            }
        }
        tok->type = JGTOKEN_NUMBER;
        tok->val = val;
    }
    else switch(ch)
    {
    case '\'':
        tok->val = *(str++);
        if(*str != '\'')
            return NULL;
        str++;
        tok->type = JGTOKEN_NUMBER;
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
        tok->type = JGTOKEN_STRING;
        break;
    case ',': case ';':
        tok->type = JGTOKEN_SEPARATOR;
        tok->val = ch;
        break;
    default:
        tok->type = JGTOKEN_CHAR;
        tok->val = ch;
    }
    tok->tokenLength = len;
    return str;
}
