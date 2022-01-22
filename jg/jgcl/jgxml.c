#include "jgxml.h"
#include "../jg.h"
#include "../jgutil.h"
#include <stdlib.h>

static inline _Bool DecodeNode(char *str, JGXMLNODE *node)
{
    printf("Decoding node: %s\n", str);
    JGTOKEN tok;
    JGXMLATTRIBUTE *attr;
    if(!(str = JGToken_Next(str, &tok)) || tok.type != JGTOKEN_IDENTIFIER)
        return 0;
    node->name = JGUtil_Createstrcpy(tok.tokenStart, tok.tokenLength);
    node->attributes = NULL;
    node->attributeCount = 0;
    node->attributeCapacity = 0;
    node->nodes = NULL;
    node->nodeCount = 0;
    node->nodeCapacity = 0;
    printf("\tNode name: %s\n", node->name);
    if(!(str = JGToken_Next(str, &tok)) || tok.type != JGTOKEN_IDENTIFIER)
        return 1;
    while(tok.type == JGTOKEN_IDENTIFIER)
    {
        node->attributeCount++;
        if(node->attributeCount > node->attributeCapacity)
        {
            node->attributeCapacity *= 2;
            node->attributeCapacity++;
            node->attributes = realloc(node->attributes, sizeof(*node->attributes) * node->attributeCapacity);
        }
        attr = node->attributes + node->attributeCount - 1;
        attr->name = JGUtil_Createstrcpy(tok.tokenStart, tok.tokenLength);
        printf("\tNode attribute name: %s\n", attr->name);
        if(!(str = JGToken_Next(str, &tok)) || tok.type != JGTOKEN_CHAR || tok.val != '=' || !(str = JGToken_Next(str, &tok)))
        {
            free(attr->name);
            return 0;
        }
        attr->type = tok.type;
        printf("\tNode attribute type: %d\n", attr->type);
        switch(tok.type)
        {
        case JGTOKEN_STRING: attr->strValue = JGUtil_Createstrcpy(tok.tokenStart, tok.tokenLength);
            printf("\tNode attribute value: %s\n", attr->strValue);
            break;
        case JGTOKEN_NUMBER: attr->intValue = tok.val;
            printf("\tNode attribute value: %d\n", attr->intValue);
            break;
        case JGTOKEN_FLOAT: attr->floatValue = *(float*) &tok.val;
            printf("\tNode attribute value: %f\n", attr->floatValue);
            break;
        case JGTOKEN_CHAR:
            if(tok.val == '-')
            {
                if((str = JGToken_Next(str, &tok)))
                {
                    attr->intValue = -tok.val;
                    printf("\tNode attribute value: %d\n", attr->intValue);
                    break;
                }
            }
        default:
            free(node->attributes);
            return 0;
        }
        str = JGToken_Next(str, &tok);
    }
    return 1;
}

static inline JGXMLNODE *ReadNode(FILE *fp)
{
    char ch;

    char nodeBuf[1000];
    int nodeLen;

    JGXMLNODE *source;
    JGXMLNODE *parent, *child;

    JGXMLNODE **path = NULL;
    int pathCnt = 0;
    int pathCap = 0;

    while((ch = fgetc(fp)) != EOF)
    {
        switch(ch)
        {
        case '<':
            nodeLen = 0;
            while((ch = getc(fp)) != EOF && isspace(ch));
            if(ch == EOF)
                goto error;
            if(ch == '/')
            {
                while((ch = getc(fp)) != EOF && isspace(ch));
                if(ch == EOF)
                    goto error;
                while((ch = getc(fp)) != EOF && isalpha(ch))
                    nodeBuf[nodeLen++] = ch;
                if(ch == EOF)
                    goto error;
                if(ch != '>')
                {
                    while((ch = getc(fp)) != EOF && isspace(ch));
                    if(ch != '>')
                        goto error;
                }
                nodeBuf[nodeLen] = 0;
                pathCnt--;
                break;
            }
            nodeBuf[nodeLen++] = ch;
            while((ch = getc(fp)) != EOF && ch != '>')
                nodeBuf[nodeLen++] = ch;
            nodeBuf[nodeLen] = 0;
            if(pathCnt)
            {
                parent = *(path + (pathCnt - 1));
                parent->nodeCount++;
                if(parent->nodeCount > parent->nodeCapacity)
                {
                    parent->nodeCapacity *= 2;
                    parent->nodeCapacity++;
                    parent->nodes = realloc(parent->nodes, sizeof(*parent->nodes) * parent->nodeCapacity);
                }
                child = parent->nodes + parent->nodeCount - 1;
            }
            else
                child = source = malloc(sizeof(*child));
            pathCnt++;
            if(pathCnt > pathCap)
            {
                pathCap *= 2;
                pathCap++;
                path = realloc(path, sizeof(*path) * pathCap);
            }
            *(path + (pathCnt - 1)) = child;
            if(!DecodeNode(nodeBuf, child))
                return 0;
            break;
        }
    }
    free(path);
    return source;
    error:

        return NULL;
}

JGXMLDOCUMENT *JGXML_Read(FILE *fp)
{
    return !fp ? NULL : ReadNode(fp);
}

void DestroyNode(JGXMLNODE *node)
{
    int cnt;
    JGXMLNODE *children;
    JGXMLATTRIBUTE *attrs;
    cnt = node->nodeCount;
    children = node->nodes;
    while(cnt--)
    {
        DestroyNode(children);
        children++;
    }
    free(node->name);
    free(node->nodes);
    cnt = node->attributeCount;
    attrs = node->attributes;
    while(cnt--)
    {
        free(attrs->name);
        if(attrs->type == JGTOKEN_STRING)
            free(attrs->strValue);
        attrs++;
    }
    free(node->attributes);
}

void JGXML_DestroyNode(JGXMLNODE *node)
{
    DestroyNode(node);
    free(node);
}

JGXMLNODE *JGXML_GetChild(JGXMLNODE *node, const char *name, JGXMLNODE *dest)
{
    int cnt;
    JGXMLNODE *children;
    cnt = node->nodeCount;
    children = node->nodes;
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

JGXMLATTRIBUTE *JGXML_GetAttribute(JGXMLNODE *node, const char *name, void *dest)
{
    int cnt = node->attributeCount;
    JGXMLATTRIBUTE *attrs = node->attributes;
    while(cnt--)
    {
        if(!strcmp(name, attrs->name))
        {
            if(dest)
            {
                switch(attrs->type)
                {
                case JGTOKEN_STRING: *((char**) dest) = JGUtil_Createstrcpy0(attrs->strValue); break;
                case JGTOKEN_FLOAT: *((float*) dest) = attrs->floatValue; break;
                case JGTOKEN_NUMBER: *((int*) dest) = attrs->intValue; break;
                }
            }
            return attrs;
        }
        attrs++;
    }
    return NULL;
}

static struct {
    const char *name;
    int bit;
} STRINGTOSTATE[] = {
    { "transparent", JGSTATE_TRANSPARENT },
    { "not_draw_bg", JGSTATE_NOTDRAWBG },
    { "fixed_width", JGSTATE_FIXEDWIDTH },
    { "fixed_height", JGSTATE_FIXEDHEIGHT },
    { "fixed_size", JGSTATE_FIXEDSIZE },
    { "cornered", JGSTATE_CORNERED },
    { "invisible", JGSTATE_INVISIBLE },
    { "borderless", JGSTATE_BORDERLESS },
    { "xbutton", JGSTATE_XBUTTON },
};

int StringToState(const char *str, int strLen)
{
    for(int i = 0; i < sizeof(STRINGTOSTATE) / sizeof(*STRINGTOSTATE); i++)
    {
        if(!JGUtil_Strcmp0(str, strLen, STRINGTOSTATE[i].name))
            return STRINGTOSTATE[i].bit;
    }
    return 0;
}

static void ParseNodes(JGCONTROL cParent, JGXMLNODE *parent)
{
    JGCONTROL control;
    JGLAYOUT *layout;
    JGGRIDBC gbc;
    JGTOKEN tok;
    int type, state;
    char *text;
    char *id;
    int x, y, w, h;
    int cnt;
    JGXMLNODE *node;
    cnt = parent->nodeCount;
    node = parent->nodes;
    while(cnt--)
    {
        state = 0;
        if(!strcmp(node->name, "Button"))
            type = JGTYPE_BUTTON;
        else if(!strcmp(node->name, "Label"))
            type = JGTYPE_STATIC;
        else if(!strcmp(node->name, "Slider"))
            type = JGTYPE_SLIDER;
        else if(!strcmp(node->name, "Window"))
            type = JGTYPE_WINDOW;
        else if(!strcmp(node->name, "Group"))
            type = JGTYPE_GROUP;
        else if(!strcmp(node->name, "Image"))
            type = JGTYPE_IMAGEVIEW;
        else
        {
            node++;
            continue;
        }
        state = 0;
        if(JGXML_GetAttribute(node, "state", &text))
        {
            while((text = JGToken_Next(text, &tok)))
            {
                if(tok.type == JGTOKEN_IDENTIFIER)
                {
                    state |= StringToState(tok.tokenStart, tok.tokenLength);
                }
            }
        }
        if(!JGXML_GetAttribute(node, "text", &text))
            text = NULL;
        if(!JGXML_GetAttribute(node, "left", &x))
            x = 0;
        if(!JGXML_GetAttribute(node, "top", &y))
            y = 0;
        if(!JGXML_GetAttribute(node, "right", &w))
            w = x;
        if(!JGXML_GetAttribute(node, "bottom", &h))
            h = y;
        w -= x;
        h -= y;
        if(!JGXML_GetAttribute(node, "id", &id))
            id = NULL;
        control = JGCreateControl(type, id, NULL, state, text, x, y, w, h);
        if(cParent)
        {
            layout = &(((JGGROUP) cParent)->layout);
            if(layout->type == JGLAYOUT_GRIDBAG)
            {
                if(!JGXML_GetAttribute(node, "gridX", &gbc.gridX))
                    gbc.gridX = -1;
                if(!JGXML_GetAttribute(node, "gridY", &gbc.gridY))
                    gbc.gridY = -1;
                if(!JGXML_GetAttribute(node, "gridWidth", &gbc.gridWidth))
                    gbc.gridWidth = 1;
                if(!JGXML_GetAttribute(node, "gridHeight", &gbc.gridHeight))
                    gbc.gridHeight = 1;
                printf("Adding layout comp: %d, %d, %d, %d\n", gbc.gridX, gbc.gridY, gbc.gridWidth, gbc.gridHeight);
                JGGridBagLayout_AddLayoutControl(cParent, control, &gbc);
            }
        }
        if(JGXML_GetAttribute(node, "layout", &text))
        {
            if(!strcasecmp(text, "Flow"))
                state = JGLAYOUT_FLOW;
            else if(!strcasecmp(text, "Stack"))
                state = JGLAYOUT_STACK;
            else if(!strcasecmp(text, "Grid"))
                state = JGLAYOUT_GRIDBAG;
            JGSetLayout(control, state);
        }
        if(type == JGTYPE_GROUP || type == JGTYPE_WINDOW)
        {
            ParseNodes(control, node);
            JGRelayout(control);
        }
        if(cParent)
            JGAddChildControl(cParent, control);
        else
            JGAddControl(control);
        node++;
    }
}

void JGXML_ParseNodes(JGXMLNODE *parent)
{
    ParseNodes(NULL, parent);
}
