#ifndef __JGTOKEN_H__
#define __JGTOKEN_H__

#include <ctype.h>
#include "../jgutil.h"

typedef enum {
    JGTOKEN_NULL,
    JGTOKEN_NUMBER,
    JGTOKEN_FLOAT,
    JGTOKEN_STRING,
    JGTOKEN_IDENTIFIER,
    JGTOKEN_SEPARATOR,
    JGTOKEN_CHAR,
    JGTOKEN_GROUP
} token_t;

typedef struct {
    char *tokenStart;
    int tokenLength;
    int val;
    int lines;
    token_t type;
} JGTOKEN;

char *JGToken_Next(char*, JGTOKEN*);
char *JGToken_ReadType(char*, token_t, JGTOKEN*);

#define JGToken_ReadIdentf(text, tok) JGToken_ReadType(text, TT_IDENTIFIER, tok)
#define JGToken_ReadString(text, tok) JGToken_ReadType(text, TT_STRING, tok)
#define JGToken_ReadNumber(text, tok) JGToken_ReadType(text, TT_INT, tok)

#endif // __JGTOKEN_H__

