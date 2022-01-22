#include "jgutil.h"
#include <stdarg.h>
#include <string.h>

int JGUtil_Strcasecmp0(const char *str, int len, const char *str0)
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

int JGUtil_Strcmp0(const char *str, int len, const char *str0)
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

char *JGUtil_Createstrcpy(const char *str, int len)
{
    char *ret = malloc(len + 1);
    memcpy(ret, str, len);
    *(ret + len) = 0;
    return ret;
}

char *JGUtil_Createstrcpy0(const char *str0)
{
    return JGUtil_Createstrcpy(str0, strlen(str0));
}

void JGList_Init(JGLIST *list, int initialCapacity, uint32_t elemSize)
{
    list->elems = initialCapacity ? malloc(initialCapacity * elemSize) : NULL;
    list->elemCount = 0;
    list->capacity = initialCapacity;
    list->elemSize = elemSize;
}

bool JGList_SetCapacity(JGLIST *list, int capacity)
{
    const int elemSize = list->elemSize;
    const int cnt = list->elemCount;
    list->elems = realloc(list->elems, capacity * elemSize);
    if(cnt > capacity)
    {
        list->elemCount = capacity;
        return 1;
    }
    return 0;
}

static void CheckCapacity(JGLIST *list, int addCnt, int newCnt, uint32_t elemSize)
{
    register size_t capacity = list->capacity;
    if(newCnt > capacity)
    {
        capacity *= 2;
        capacity += addCnt;
        list->capacity = capacity;
        capacity *= elemSize;
        list->elems = realloc(list->elems, capacity * elemSize);
    }
}

void *JGList_AddElem(JGLIST * restrict list, const void * restrict elem)
{
    const uint32_t elemSize = list->elemSize;
    const int cnt = list->elemCount;
    const int newCnt = list->elemCount = cnt + 1;
    CheckCapacity(list, 1, newCnt, elemSize);
    return memcpy(list->elems + cnt * elemSize, elem, elemSize);
}

void *JGList_AddElems(JGLIST * restrict list, int elemCnt, const void * restrict elems)
{
    const uint32_t elemSize = list->elemSize;
    const int cnt = list->elemCount;
    const int newCnt = list->elemCount = cnt + elemCnt;
    CheckCapacity(list, elemCnt, newCnt, elemSize);
    return memcpy(list->elems + cnt * elemSize, elems, elemCnt * elemSize);
}

void *JGList_InsertElem(JGLIST *list, int index, const void *elem)
{
    const uint32_t elemSize = list->elemSize;
    const int cnt = list->elemCount;
    const int newCnt = list->elemCount = cnt + 1;
    CheckCapacity(list, 1, newCnt, elemSize);
    void *elems = list->elems;
    elems += index * elemSize;
    memmove(elems + elemSize, elems, (cnt - index) * elemSize);
    return memcpy(elems, elem, elemSize);
}

void *JGList_InsertElems(JGLIST * restrict list, int index, int elemCnt, const void * restrict elems)
{
    const uint32_t elemSize = list->elemSize;
    const int cnt = list->elemCount;
    const int newCnt = list->elemCount = cnt + elemCnt;
    CheckCapacity(list, elemCnt, newCnt, elemSize);
    void *newElems = list->elems;
    newElems += cnt * elemSize;
    memmove(newElems + elemCnt * elemSize, newElems, (newCnt - index - elemCnt) * elemSize);
    return memcpy(newElems, elems, elemCnt * elemSize);
}

inline void *JGList_ElemAt(JGLIST *list, int index)
{
    return list->elems + index * list->elemSize;
}

int JGList_IndexOf(JGLIST *list, const void *elem)
{
    const uint32_t elemSize = list->elemSize;
    register int cnt = list->elemCount;
    register void *elems = list->elems;
    while(cnt--)
    {
        if(!memcmp(elems, elem, elemSize))
            return list->elemCount - cnt;
        elems += elemSize;
    }
    return -1;
}

inline bool JGList_RemoveElem(JGLIST *list, const void *elem)
{
    return JGList_RemoveElems(list, elem, 1);
}

bool JGList_RemoveElems(JGLIST *list, const void *elem, int removeCnt)
{
    int index = JGList_IndexOf(list, elem);
    if(index < 0)
        return 0;
    return JGList_RemoveElemsAtIndex(list, index, removeCnt);
}

inline bool JGList_RemoveElemAtIndex(JGLIST *list, int index)
{
    return JGList_RemoveElemsAtIndex(list, index, 1);
}

bool JGList_RemoveElemsAtIndex(JGLIST *list, int index, int removeCnt)
{
    const uint32_t elemSize = list->elemSize;
    const int cnt = list->elemCount;
    const int newCnt = cnt - removeCnt;
    if(newCnt < 0)
    {
        free(list->elems);
        list->elems = NULL;
        list->elemCount = 0;
        return 0;
    }
    void *elems = list->elems + index * elemSize;
    memmove(elems, elems + removeCnt * elemSize, (newCnt - index) * elemSize);
    list->elemCount = newCnt;
    return 1;
}

void JGList_Clear(JGLIST *list)
{
    free(list->elems);
    list->elemCount = 0;
}

bool JGList_EnumElems(JGLIST *list, JGLISTENUMERATOR er)
{
    const uint32_t elemSize = list->elemSize;
    register int cnt = list->elemCount;
    register void *elems = list->elems;
    while(cnt--)
    {
        if(!er(elems))
            return 0;
        elems += elemSize;
    }
    return 1;
}

void JGTable_AddEntry(JGTABLE *table, char *entry, const void *elem)
{
    const uint32_t elemSize = table->elemSize;
    char e[elemSize];
    memcpy(e, &entry, sizeof(char*));
    memcpy(e + sizeof(char*), elem, elemSize - sizeof(char*));
    JGList_AddElem(table, &e);
}

void JGTable_InsertEntry(JGTABLE *table, int index, char *entry, const void *elem)
{
    const uint32_t elemSize = table->elemSize;
    char e[elemSize];
    memcpy(e, &entry, sizeof(char*));
    memcpy(e + sizeof(char*), elem, elemSize - sizeof(char*));
    JGList_InsertElem(table, index, &e);
}

int JGTable_IndexOf(const JGTABLE *table, const char *entry)
{
    const uint32_t elemSize = table->elemSize;
    register int cnt = table->elemCount;
    register void *entries = table->elems;
    while(cnt)
    {
        if(!strcmp(*(char**) entries, entry))
            return table->elemCount - cnt;
        entries += elemSize;
        cnt--;
    }
    return -1;
}

bool JGTable_RemoveEntry(JGTABLE *table, const char *str)
{
    int index = JGTable_IndexOf(table, str);
    if(index < 0)
        return 0;
    return JGList_RemoveElemAtIndex(table, index);
}

void *JGTable_ElemOf(const JGTABLE *table, const char *str)
{
    int index = JGTable_IndexOf(table, str);
    if(index < 0)
        return NULL;
    return table->elems + index * table->elemSize + sizeof(char*);
}

// TREE
void JGTree_Init(JGTREE *tree, uint32_t elemSize)
{
    memset(&tree->base, 0, sizeof(JGTREENODE));
    tree->elemSize = elemSize;
}

void *JGTree_AddNode(JGTREE *tree, JGTREENODE *node, void *elem)
{
    const uint32_t elemSize = tree->elemSize;
    JGTREENODE *nodes = node->nodes;
    const int oldCnt = node->nodeCnt;
    const int newCnt = node->nodeCnt = oldCnt + 1;
    nodes = node->nodes = realloc(nodes, sizeof(*nodes) * newCnt);

    nodes += oldCnt;
    nodes->elem = malloc(elemSize);
    memcpy(nodes->elem, elem, elemSize);
    nodes->nodeCnt = 0;
    nodes->nodes = NULL;
    return nodes;
}

JGTREENODE *JGTree_GetNode(JGTREE *tree, JGTREENODE *node, int index)
{
    return node->nodes + index;
}

void JGTreeEnumerator_Init(JGTREEENUMERATOR *enumerator, JGTREENODE *start)
{
    enumerator->selected = start;
    enumerator->route = malloc(sizeof(JGTREENODE));
    enumerator->routeCount = 0;
    enumerator->route = NULL;
}

bool JGTreeEnumerator_MoveDown(JGTREEENUMERATOR *enumerator, int index)
{
    JGTREENODE *sel = enumerator->selected;
    if(index < 0 || index >= sel->nodeCnt)
        return 0;
    const int routeCnt = enumerator->routeCount;
    const int newRouteCnt = enumerator->routeCount = routeCnt + 1;
    JGTREENODE **route = enumerator->route;
    route = enumerator->route = realloc(route, sizeof(*route) * newRouteCnt);
    *(route + routeCnt) = sel;
    enumerator->selected = sel->nodes + index;
    enumerator->selectedIndex = index;
    return 1;
}

bool JGTreeEnumerator_MoveUp(JGTREEENUMERATOR *enumerator)
{
    int routeCnt = enumerator->routeCount;
    if(!routeCnt)
        return 0;
    int newRouteCnt = enumerator->routeCount = routeCnt - 1;
    enumerator->selected = *(enumerator->route + newRouteCnt);
    return 1;
}

bool JGTreeEnumerator_MoveRight(JGTREEENUMERATOR *enumerator)
{
    int index = enumerator->selectedIndex;
    JGTREENODE **route = enumerator->route;
    int routeCnt = enumerator->routeCount;
    JGTREENODE *parent = *(route + (routeCnt - 1));
    if(index >= parent->nodeCnt)
        return 0;
    int newIndex = enumerator->selectedIndex = index + 1;
    enumerator->selected = parent->nodes + newIndex;
    return 1;
}

bool JGTreeEnumerator_MoveLeft(JGTREEENUMERATOR *enumerator)
{
    int index = enumerator->selectedIndex;
    if(!index)
        return 0;
    JGTREENODE **route = enumerator->route;
    int routeCnt = enumerator->routeCount;
    JGTREENODE *parent = *(route + (routeCnt - 1));
    int newIndex = enumerator->selectedIndex = index - 1;
    enumerator->selected = parent->nodes + newIndex;
    return 1;
}



