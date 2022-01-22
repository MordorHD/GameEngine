#ifndef __JGXML_H__
#define __JGXML_H__

#include <stdio.h>
#include "jgtoken.h"

struct _xmlnode;

typedef struct {
    char *name;
    int type;
    union {
        int intValue;
        float floatValue;
        char *strValue;
    };
} JGXMLATTRIBUTE;

typedef struct _xmlnode {
    char *name;
    // attributes
    JGXMLATTRIBUTE *attributes;
    int attributeCount;
    int attributeCapacity;
    // child nodes
    struct _xmlnode *nodes;
    int nodeCount;
    int nodeCapacity;
} JGXMLDOCUMENT, JGXMLNODE;

JGXMLDOCUMENT *JGXML_Read(FILE*);
void JGXML_DestroyNode(JGXMLNODE*);
void JGXML_ParseNodes(JGXMLNODE*);
JGXMLNODE *JGXML_GetChild(JGXMLNODE*, const char*, JGXMLNODE*);
JGXMLATTRIBUTE *JGXML_GetAttribute(JGXMLNODE*, const char*, void*);

typedef struct {
    char *name;
    int type;
    union {
        int intValue;
        float floatValue;
        char *strValue;
    };
} JGCSSPROPERTY;

typedef struct {
    char *name;
    JGCSSPROPERTY *properties;
    int propertyCount;
    int propertyCapacity;
} JGCSSCLASS;

typedef struct {
    JGCSSCLASS *classes;
    int classCount;
    int classCapacity;
} JGCSS;

JGCSS *JGCSS_Read(FILE*);
void JGCSS_Destroy(JGCSS*);
void JGCSS_Parse(JGCSS*);
JGCSSCLASS *JGCSS_GetClass(JGCSS*, const char*, JGCSSCLASS*);
JGCSSPROPERTY *JGCSS_GetProperty(JGCSSCLASS*, const char*, void*);

#endif // __JGXML_H__
