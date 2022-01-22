#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stdio.h>
#include "token.h"

_Bool CompileFile(const char*);

typedef enum {
    VTM_NULL,
    VTM_INT,
    VTM_FLOAT,
    VTM_STRING,
    VTM_ARRAY4,
    VTM_ARRAY8,
    VTM_REFERENCE
} vartype_t;

typedef struct {
    char structSize;
    const char *varName;
    vartype_t varType;
    union {
        int varIntValue;
        int varValueCount;
    };
    void *varBase;
} VARIABLE;

void *AllocateVariable(const char*, int, vartype_t, int, void*);
void *AllocateVariableFreely(const char*, int, vartype_t, int);
void WipeVariable(const char*, int);
void *GetFirstVariable(void);
void *GetVariableOffset(const char*, int);
void MemoryVariableSeek(int);
void MemoryVariableSeekP(void*);
_Bool GetVariable(const char*, int, char*, void*, void**);

#define ARRLEN(a) (sizeof(a)/sizeof(*(a)))
#define VoidToInt(a) ((int) (long long int) a)
#define IntToVoid(a) ((void*) (long long int) a)

#endif // __COMPILER_H__

