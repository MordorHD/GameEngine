#ifndef __JGUTIL_H__
#define __JGUTIL_H__

#include "jgdefs.h"
#include <stdlib.h>

int JGUtil_Strcasecmp0(const char*, int, const char*);
int JGUtil_Strcmp0(const char*, int, const char*);
char *JGUtil_Createstrcpy(const char*, int);
char *JGUtil_Createstrcpy0(const char*);

typedef struct ListTag {
    void *elems;
    int count;
    int capacity;
    uint32_t elemSize;
} JGLIST, JGTABLE, JGINTLIST, JGSTRINGLIST;

#define JGCheckBounds(index, cnt) ((index)>=0&&(index)<(cnt))
void JGList_Init(JGLIST*, int, uint32_t);
bool JGList_SetCapacity(JGLIST*, int);
void *JGList_AddElem(JGLIST*, const void*);
void *JGList_AddElems(JGLIST* restrict, int, const void* restrict);
void *JGList_InsertElem(JGLIST*, int, const void*);
void *JGList_InsertElems(JGLIST* restrict, int, int, const void* restrict);
void *JGList_ElemAt(JGLIST*, int);
int JGList_IndexOf(JGLIST*, const void*);
bool JGList_RemoveElem(JGLIST*, const void*);
bool JGList_RemoveElems(JGLIST*, const void*, int);
bool JGList_RemoveElemAtIndex(JGLIST*, int);
bool JGList_RemoveElemsAtIndex(JGLIST*, int, int);
void JGList_Clear(JGLIST*);
typedef bool (*JGLISTENUMERATOR)(void*);
bool JGList_EnumElems(JGLIST*, JGLISTENUMERATOR);
#define JGList_Inc(list, a) a += (list)->elemSize
#define JGList_Dec(list, a) a -= (list)->elemSize
#define JGList_Get(list, index) ((list)->elems + (list)->elemSize * index)

typedef struct {
    char *entry;
    void *elem;
} JGENTRY;

#define JGTable_Init(table, cap, elemSize) JGList_Init(table, cap, sizeof(char*) + elemSize)

void JGTable_AddEntry(JGTABLE*, char*, const void*);
#define JGTable_AddEntries JGList_AddElems
void JGTable_InsertEntry(JGLIST*, int, char*, const void*);
#define JGTable_InsertEntries JGList_InsertElems
int JGTable_IndexOf(const JGTABLE*, const char*);
void *JGTable_ElemOf(const JGTABLE*, const char*);
bool JGTable_RemoveEntry(JGTABLE*, const char*);
#define JGTable_RemoveEntryAtIndex JGList_RemoveElemAtIndex
#define JGTable_RemoveEntriesAtIndex JGList_RemoveElemAtIndex
#define JGTable_Clear JGList_Clear
typedef bool (*JGTABLEENUMERATOR)(JGENTRY*);
#define JGTable_EnumElems JGList_EnumElems

typedef struct node {
    int nodeCnt;
    struct node *nodes;
    void *elem;
} JGTREENODE;

// target: slow to create, but fast access times, also minimizing memory usage
typedef struct tree {
    JGTREENODE base;
    uint32_t elemSize;
} JGTREE;

void JGTree_Init(JGTREE*, uint32_t);
void *JGTree_AddNode(JGTREE*, JGTREENODE*, void*);
JGTREENODE *JGTree_GetNode(JGTREE*, JGTREENODE*, int);

typedef struct {
    JGTREENODE *selected;
    int selectedIndex;
    JGTREENODE **route;
    int routeCount;
} JGTREEENUMERATOR;

void JGTreeEnumerator_Init(JGTREEENUMERATOR*, JGTREENODE*);
bool JGTreeEnumerator_MoveDown(JGTREEENUMERATOR*, int);
bool JGTreeEnumerator_MoveUp(JGTREEENUMERATOR*);
bool JGTreeEnumerator_MoveRight(JGTREEENUMERATOR*);
bool JGTreeEnumerator_MoveLeft(JGTREEENUMERATOR*);

#define JGTreeEnumerator_GetElem(e, dest) memcpy(dest, (e)->selected->elem, sizeof(*dest))

#endif // __JGUTIL_H__

