#include "compiler.h"
#include "pltobj.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    const char *className;
    const char **classAttributes;
    int classAttributeCount;
} CLASS;

static const char *ObjectAttributes[] = { "Iside", "Fweight", "[Icombat", "Ffriction", "[Irect", "[Irect", "[Ianimation", "Iproperties" };

static const CLASS ClassMap[] = {
    { "Object", ObjectAttributes, ARRLEN(ObjectAttributes) }
};

//   2      n     n+len     1         4        [VARVALUECNT]
// [SIZE] [NAME]   [0]    [TYPE] [VARINTVALUE]    |...|
// [SIZE] [NAME]   [0]    [TYPE] [VARVALUECNT]   [STRING]
static char Variables[2048];
static int VariableOffset = 1;
static int VariableCount = 0;

void *AllocateVariable(const char *name, int nameLen, vartype_t type, int val, void *ext)
{
    short structSize;
    char *base;
    char *var;
    WipeVariable(name, nameLen);
    base = Variables + VariableOffset;
    var = base + 2;
    memcpy(var, name, nameLen);
    var += nameLen;
    *(var++) = 0;
    *(var++) = type;
    memcpy(var, &val, sizeof(val));
    var += sizeof(val);
    switch(type)
    {
    case VTM_INT:
    case VTM_FLOAT: break;
    case VTM_ARRAY4: memcpy(var, ext, 4 * val); var += 4 * val; break;
    case VTM_ARRAY8: memcpy(var, ext, 8 * val); var += 8 * val; break;
    case VTM_STRING: memcpy(var, ext, val); var += val; break;
    case VTM_REFERENCE: memcpy(var, &ext, sizeof(ext)); var += sizeof(ext); break;
    default:
        return NULL;
    }
    structSize = var - base;
    VariableOffset += structSize;
    *(short*) base = structSize;
    VariableCount++;
    return base;
}

void WipeVariable(const char *name, int nameLen)
{
    void *var;
    short structSize;
    if((var = GetVariableOffset(name, nameLen)))
    {
        structSize = *(short*) var;
        VariableOffset -= structSize;
        memcpy(var, var + structSize, VariableOffset - (var - (void*) Variables));
    }
}

void *AllocateVariableFreely(const char *name, int nameLen, vartype_t type, int val)
{
    char *base;
    char *var;
    base = Variables + VariableOffset;
    var = base + 2;
    memcpy(var, name, nameLen);
    var += nameLen;
    *(var++) = 0;
    *(var++) = type;
    memcpy(var, &val, sizeof(val));
    var += sizeof(val);
    VariableCount++;
    return base;
}

void *GetFirstVariable(void)
{
    return Variables + 1;
}

void MemoryVariableSeek(int offset)
{
    VariableOffset = offset;
}

void MemoryVariableSeekP(void *ptr)
{
    VariableOffset = ptr - (void*) Variables;
}

void *GetVariableOffset(const char *name, int nameLen)
{
    char *base, *variables;
    short structSize;
    int cnt;
    int len;
    variables = Variables + 1;
    cnt = VariableCount;
    while(cnt--)
    {
        base = variables;
        structSize = *(short*) variables;
        variables += 2;
        len = strlen(variables);
        if(len == nameLen && !memcmp(name, variables, nameLen))
            return base;
        variables += structSize - 2;
    }
    return 0;
}

_Bool GetVariable(const char *name, int nameLen, char *type, void *val, void **ext)
{
    char *variables;
    short structSize;
    int cnt;
    int len;
    variables = Variables + 1;
    cnt = VariableCount;
    while(cnt--)
    {
        structSize = *(short*) variables;
        variables += 2;
        len = strlen(variables);
        if(len == nameLen && !memcmp(name, variables, nameLen))
        {
            variables += len + 1;
            *type = *(variables++);
            memcpy(val, variables, 4);
            *ext = variables + 4;
            return 1;
        }
        variables += structSize - 2;
    }
    return 0;
}

char *DecodeAndAllocVarValue(char *nameStart, int nameLen, char *text, TOKEN *tok, void **varDest)
{
    int varVal;
    int arrElemSize;
    vartype_t varType;
    int lineCnt;
    char *reset;
    char *varBase, *var, *varOff;
    switch(tok->type)
    {
    case TT_STRING: AllocateVariable(nameStart, nameLen, VTM_STRING, tok->tokenLength, tok->tokenStart); return text;
    case TT_CHAR:
        if(tok->val != '[')
        {
            printf("error at line %d: invalid token after equals sign\n", tok->lines);
            return NULL;
        }
        reset = text;
        text = NextToken(text, tok);
        if(tok->type == TT_CHAR && tok->val == ']')
            break;
        if(tok->type == TT_CHAR || tok->type == TT_SEPARATOR)
        {
            printf("error at line %d: invalid token at start of array\n", tok->lines);
            return NULL;
        }
        varVal = 1;
        if(tok->type == TT_INT)
        {
            arrElemSize = 4;
            varType = VTM_ARRAY4;
        }
        else
        {
            arrElemSize = 8;
            varType = VTM_ARRAY8;
        }
        while((text = NextToken(text, tok)) && tok->type == TT_SEPARATOR)
        {
            if(!(text = NextToken(text, tok)) || tok->type == TT_SEPARATOR || tok->type == TT_CHAR)
            {
                printf("error at line %d: invalid token after separator\n", tok->lines);
                return NULL;
            }
            if(tok->type == TT_IDENTIFIER)
            {
                arrElemSize = 8;
                varType = VTM_ARRAY8;
            }
            varVal++;
        }
        if(tok->type != TT_CHAR || tok->val != ']')
        {
            printf("error at line %d: invalid token, expected array end (']')\n", tok->lines);
            return NULL;
        }
        lineCnt = tok->lines;
        var = AllocateVariableFreely(nameStart, nameLen, varType, varVal);
        varBase = var;
        var += 2 + nameLen + 1 + 1 + sizeof(int);
        while(varVal--)
        {
            reset = NextToken(reset, tok);
            switch(tok->type)
            {
            case TT_INT: memcpy(var, &tok->val, sizeof(int)); break;
            case TT_STRING: memcpy(var, tok->tokenStart, tok->tokenLength); break;
            case TT_IDENTIFIER:
                varOff = GetVariableOffset(tok->tokenStart, tok->tokenLength);
                memcpy(var, &varOff, 8);
                break;
            default:
                // shouldn't happen
                return NULL;
            }
            var += arrElemSize;
            reset = NextToken(reset, tok);
        }
        *(short*) varBase = var - varBase;
        tok->lines = lineCnt;
        MemoryVariableSeekP(var);
        return reset;
    default:
        AllocateVariable(nameStart, nameLen, tok->type == TT_FLOAT ? VTM_FLOAT : VTM_INT, tok->val, NULL);
        return text;
    }
    return NULL;
}

_Bool CompileFile(const char *fileName)
{
    FILE *fp;
    char *textBase, *text, *reset;
    long len;
    if(!(fp = fopen(fileName, "rb")))
        return 0;
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    textBase = text = malloc(len + 1);
    fread(text, 1, len, fp);
    *(text + len) = 0;
    fclose(fp);

    TOKEN tok;
    char *defStart;
    int defLen;
    char *nameStart;
    int nameLen;
    const char *attr;
    void *var;
    void *ext;
    char type;
    int iVal;
    float fVal;
    OBJECT object;
    int i, j;

    printf("TEXT: %s\n", text);

    tok.lines = 1;
    while((text = NextToken(text, &tok)))
    {
        if(tok.type != TT_IDENTIFIER || strcasecmp0(tok.tokenStart, tok.tokenLength, "def"))
        {
            printf("error at line %d: expected def keyword\n", tok.lines);
            goto error;
        }
        if(!(text = ReadIdentifier(text, &tok)))
        {
            printf("error at line %d: expected identifier\n", tok.lines);
            goto error;
        }
        defStart = tok.tokenStart;
        defLen = tok.tokenLength;
        if(!(text = ReadIdentifier(text, &tok)))
        {
            printf("error at line %d: expected name (identifier) after identifier\n", tok.lines);
            goto error;
        }
        nameStart = tok.tokenStart;
        nameLen = tok.tokenLength;
        if(!(text = NextToken(text, &tok)) || tok.type != TT_CHAR || tok.val != '=')
        {
            printf("error at line %d: expected equals sign after name (identifier)\n", tok.lines);
            goto error;
        }
        if(!(reset = NextToken(text, &tok)))
        {
            printf("error at line %d: expected value after equals sign\n", tok.lines);
            goto error;
        }
        if(!strcasecmp0(defStart, defLen, "Var"))
        {
            DecodeAndAllocVarValue(nameStart, nameLen, text, &tok, &var);
        }
        else
        {
            if(tok.type != TT_CHAR || tok.val != '{')
            {
                printf("error at line %d: invalid token after equals sign (expected '{')\n", tok.lines);
                goto error;
            }
            text = reset;
            for(i = 0; i < ARRLEN(ClassMap); i++)
                if(!strcasecmp0(defStart, defLen, ClassMap[i].className))
                {
                    while((text = NextToken(text, &tok)))
                    {
                        if(tok.type == TT_CHAR && (tok.val == '}' || tok.val == '.'))
                        {
                            if(tok.val == '}')
                                break;
                            if(!(text = NextToken(text, &tok)) || tok.type != TT_IDENTIFIER)
                            {
                                printf("error at line %d: invalid token, expected attribute name after '.'\n", tok.lines);
                                goto error;
                            }
                            defStart = tok.tokenStart;
                            defLen = tok.tokenLength;
                            if(!(text = NextToken(text, &tok)) || tok.type != TT_CHAR || tok.val != '=')
                            {
                                printf("error at line %d: invalid token, expected '=' after attribute\n", tok.lines);
                                goto error;
                            }
                            if(!(text = NextToken(text, &tok)))
                            {
                                printf("error at line %d: expected token after '='\n", tok.lines);
                                goto error;
                            }
                            for(j = 0; j < ClassMap[i].classAttributeCount; j++)
                            {
                                attr = ClassMap[i].classAttributes[j];
                                if(*attr == '[')
                                    attr += 2;
                                else
                                    attr++;
                                if(!strcasecmp0(defStart, defLen, attr))
                                {
                                    attr = ClassMap[i].classAttributes[j];
                                    if((tok.type == TT_FLOAT || tok.type == TT_INT) && *attr != 'F' && *attr != 'I')
                                    {
                                        printf("error at line %d: invalid value type (expected type '%c')\n", tok.lines, *attr);
                                        goto error;
                                    }
                                    if(tok.type == TT_STRING && *attr != 'S')
                                    {
                                        printf("error at line %d: invalid value type (expected type '%c')\n", tok.lines, *attr);
                                        goto error;
                                    }
                                    if(tok.type == TT_CHAR && tok.val == '[' && *attr != '[')
                                    {
                                        printf("error at line %d: invalid value type (expected type '%c')\n", tok.lines, *attr);
                                        goto error;
                                    }
                                    text = DecodeAndAllocVarValue(defStart, defLen, text, &tok, &var);
                                    break;
                                }
                            }
                            if(j == ClassMap[i].classAttributeCount)
                            {
                                printf("error at line %d: invalid attribute name\n", tok.lines);
                                goto error;
                            }
                            if(!(text = NextToken(text, &tok)) || (tok.type != TT_SEPARATOR && tok.type != TT_CHAR))
                            {
                                printf("error at line %d: invalid token after value (expected ',' or '}')\n", tok.lines);
                                goto error;
                            }
                            if(tok.val == '}')
                                break;
                        }
                        else
                        {
                            printf("error at line %d: invalid token (expected '.' or '}')\n", tok.lines);
                            goto error;
                        }
                    }
                    break;
                }
            if(i == ARRLEN(ClassMap))
            {
                printf("error at line %d: invalid class name\n", tok.lines);
                goto error;
            }
            if(!i)
            {
                memset(&object, 0, sizeof(object));
                if(GetVariable("side", 4, &type, &iVal, &ext))
                    object.side = iVal;
                if(GetVariable("weight", 6, &type, &fVal, &ext))
                    object.weight = type == VTM_FLOAT ? fVal : (float) *(int*) &fVal;
                if(GetVariable("friction", 8, &type, &fVal, &ext))
                    object.friction = type == VTM_FLOAT ? fVal : (float) *(int*) &fVal;
                if(GetVariable("rect", 4, &type, &iVal, &ext))
                    object.rect = (JGRECT2D) {
                        .left   = (double) *((int*) ext),
                        .top    = (double) *((int*) ext + 1),
                        .right  = (double) *((int*) ext + 2),
                        .bottom = (double) *((int*) ext + 3)
                    };
                if(GetVariable("combat", 6, &type, &iVal, &ext))
                    object.combat = (OBJCOMBAT) {
                        .damage          = (double) *((int*) ext),
                        .health          = (double) *((int*) ext + 1),
                        .maxHealth       = (double) *((int*) ext + 2),
                        .currentCooldown = (double) *((int*) ext + 3),
                        .cooldown        = (double) *((int*) ext + 4)
                    };
                GetVariable("animation", 9, &type, &iVal, &ext);
                printf("SIDE[%d]\n", object.side);
                printf("WEIGHT[%lf]\n", object.weight);
                printf("FRICTION[%lf]\n", object.friction);
                printf("RECT[%lf, %lf, %lf, %lf]\n", object.rect.left, object.rect.top, object.rect.right, object.rect.bottom);
                printf("COMBAT[%lf, %lf, %lf, %d, %d]\n", object.combat.damage, object.combat.health, object.combat.maxHealth, object.combat.currentCooldown, object.combat.cooldown);
                Object_AddClass(nameStart, nameLen, &object);
            }
        }
    }
    free(textBase);
    return 1;
    error:
        free(textBase);
        return 0;
}

