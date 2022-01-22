#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <ctype.h>

int strcasecmp0(const char*, int, const char*);
int strcmp0(const char*, int, const char*);
char *CreateStringCopy(const char*, int);
char *CreateStringCopy0(const char*);

typedef enum {
    TT_NULL,
    TT_INT,
    TT_FLOAT,
    TT_STRING,
    TT_IDENTIFIER,
    TT_SEPARATOR,
    TT_CHAR,
    TT_GROUP
} token_t;

typedef struct {
    char *tokenStart;
    int tokenLength;
    int val;
    int lines;
    token_t type;
} TOKEN;

char *NextToken(char*, TOKEN*);
char *ReadTokenType(char*, token_t, TOKEN*);

#define ReadIdentifier(text, tok) ReadTokenType(text, TT_IDENTIFIER, tok)
#define ReadString(text, tok) ReadTokenType(text, TT_STRING, tok)
#define ReadNumber(text, tok) ReadTokenType(text, TT_INT, tok)

#endif // __TOKEN_H__

