#include "../jg/jg.h"
#include "pltobj.h"
#include "pltgrid.h"
#include "pltcommon.h"
#include "../jg/jgcl/jgxml.h"

char KeyStates[255];

OBJECT *Player;

static inline void PrintNode(JGXMLNODE *node, int depth)
{
    for(int i = 0; i < depth; i++)
        printf("    ");
    printf("- %s[", node->name);
    if(node->attributeCount)
    {
        printf("%s", node->attributes->name);
        printf("=");
        switch(node->attributes->type)
        {
        case JGTOKEN_FLOAT: printf("%f", node->attributes->floatValue); break;
        case JGTOKEN_NUMBER: printf("%d", node->attributes->intValue); break;
        case JGTOKEN_STRING: printf("\"%s\"", node->attributes->strValue); break;
        }
    }
    for(int i = 1; i < node->attributeCount; i++)
    {
        printf(", %s", (node->attributes + i)->name);
        printf("=");
        switch((node->attributes + i)->type)
        {
        case JGTOKEN_FLOAT: printf("%f", (node->attributes + i)->floatValue); break;
        case JGTOKEN_NUMBER: printf("%d", (node->attributes + i)->intValue); break;
        case JGTOKEN_STRING: printf("\"%s\"", (node->attributes + i)->strValue); break;
        }
    }
    printf("]\n");
    for(int i = 0; i < node->nodeCount; i++)
        PrintNode(node->nodes + i, depth + 1);
}

void ButtonHandle(JGCONTROL button, uint32_t index, const JGEVENT *event)
{
    printf("Pressed :)\n");
}

JGIMAGE PngImage;

void JGStart(void)
{
    FILE *fp = fopen("F:\\.Programming Languages\\C\\GameEngine\\platformer\\cmps.css", "r");
    JGCSS *css = JGCSS_Read(fp);
    fclose(fp);
    JGCSS_Parse(css);
    JGCSS_Destroy(css);

    fp = fopen("F:\\.Programming Languages\\C\\GameEngine\\platformer\\cmps.xml", "r");
    JGXMLDOCUMENT *doc = JGXML_Read(fp);
    if(!doc)
    {
        JGStop();
        return;
    }
    fclose(fp);
    JGXML_ParseNodes(doc);
    JGXML_DestroyNode(doc);
    JGCONTROL control = JGGetControl("Button");
    JGHideControl(control);
    JGSetHandle(control, ButtonHandle);

    control = JGGetControl("Window");
    JGHideControl(control);
    JGShowControl(control);

    //PrintNode(doc, 0);
    //JGStop();
    Sprites_Load();
    Grid_Init();
    Objects_Init();

    PngImage = JGLoadImage("F:\\.Programming Languages\\C\\GameEngine\\platformer\\windows-xp-bliss-4k-lu-1920x1080.jpg", 0);
    printf("%I64d\n", PngImage);

    JGIMAGEVIEW imageControl = (JGIMAGEVIEW) JGGetControl("Image1");
    imageControl->image = Sprites_Get();
    imageControl->cut = (JGRECT) {
        .left = 128,
        .top = 240,
        .right = 144,
        .bottom = 256
    };
    imageControl = (JGIMAGEVIEW) JGGetControl("Image2");
    imageControl->image = Sprites_Get();
    imageControl->cut = (JGRECT) {
        .left = 128,
        .top = 256,
        .right = 144,
        .bottom = 272
    };
    imageControl = (JGIMAGEVIEW) JGGetControl("Image3");
    imageControl->image = Sprites_Get();
    imageControl->cut = (JGRECT) {
        .left = 144,
        .top = 256,
        .right = 160,
        .bottom = 272
    };

    Player = Object_CreateFromID(0);
    OBJECT *bow = Object_Create("Bow");
    bow->rect = (JGRECT2D) {
        .left = 30.0d,
        .top = 30.0d,
        .right = 96.0d,
        .bottom = 96.0d
    };
    OBJECT *coin;
    for(int i = 0; i < 20; i++)
    {
        coin = Object_Create("Coin");
        coin->rect = (JGRECT2D) {
            .left = i * 50.0d + 300.0d,
            .top = 500.0d,
            .right = i * 50.0d + 332.0d,
            .bottom = 532.0d,
        };
    }

    OBJECT *cEnemy;
    for(int i = 0; i < 5; i++)
    {
        cEnemy = Object_Create("CactusEnemy");
        cEnemy->rect = (JGRECT2D) {
            .left = i * 50.0d + 300.0d,
            .top = 500.0d,
            .right = i * 50.0d + 332.0d,
            .bottom = 532.0d,
        };
    }

    JGCAMERA cam;
    cam.x = 0;
    cam.y = 0;
    cam.target = &Player->rect.point1;
    cam.cstr_l = 0;
    cam.cstr_t = 0;
    cam.cstr_r = GRID_WIDTH * GRID_SPSIZE;
    cam.cstr_b = GRID_HEIGHT * GRID_SPSIZE;
    cam.cFlags = JGCA_HCENTER | JGCA_VCENTER;
    JGSetCamera(&cam);
}

void JGDraw(void)
{
    JGCAMERA cam;
    JGSIZE size;
    int x, y;
    int i;
    double health;
    int iMaxHealth;
    int cnt;
    int row;
    short *grid;
    JGIMAGE sprites = Sprites_Get();

    int x1, y1, x2, y2;
    int dx, dy, sx, sy;
    int err, err2;
    int yInc;

    double velX, velY;

    _Bool moved;

    JGGetWindowSize(&size);
    JGGradient(0, 0, 0xFF869CEB, size.width, size.height, 0xFF86EBD5, 0);
    JGDrawImage(sprites, 200, 200, 400, 200);
    //JGFill(JGRED);
    //JGRect(700, 10, 828, 138);
    JGDrawImage(PngImage, 0, 0, size.width, size.height);

    JGGetCamera(&cam);

    JGDrawImageSection(sprites, 0, 196, 120, 48, 100  - (int) (cam.x / 2.6d), 60 - (int) (cam.y / 2.6d), 240, 96);
    JGDrawImageSection(sprites, 0, 196, 120, 48, 800  - (int) (cam.x / 2.6d), 90 - (int) (cam.y / 2.6d), 240, 96);
    JGDrawImageSection(sprites, 0, 196, 120, 48, 300  - (int) (cam.x / 2.6d), 10 - (int) (cam.y / 2.6d), 240, 96);
    JGDrawImageSection(sprites, 0, 196, 120, 48, 1400 - (int) (cam.x / 2.6d), 70 - (int) (cam.y / 2.6d), 240, 96);

    grid = Grid_Get();
    cnt = GRID_LENGTH;
    x = -(int) cam.x;
    y = -(int) cam.y;
    row = GRID_WIDTH;
    while(cnt--)
    {
        if(!row)
        {
            x = -(int) cam.x;
            y += GRID_SPSIZE;
            row = GRID_WIDTH;
        }
        switch(*grid)
        {
        case GRID_DIRTGRASS: JGDrawImageSection(sprites, 0, 0, 16, 16, x, y, GRID_SPSIZE, GRID_SPSIZE); break;
        case GRID_DIRT: JGDrawImageSection(sprites, 16, 0, 16, 16, x, y, GRID_SPSIZE, GRID_SPSIZE); break;
        case GRID_SAND: JGDrawImageSection(sprites, 32, 0, 16, 16, x, y, GRID_SPSIZE, GRID_SPSIZE); break;
        }
        x += GRID_SPSIZE;
        grid++;
        row--;
    }
    Objects_Draw(&cam);

    health = Player->combat.health;
    iMaxHealth = (int) Player->combat.maxHealth;
    for(i = 0; i < iMaxHealth * 2; i++)
    {
        health -= 0.5d;
        if(health < 0)
            JGDrawImageSection(sprites, 32 + (i % 2) * 8, 48, 8, 16, 10 + i * 16, 8, 16, 32);
        else
            JGDrawImageSection(sprites, (i % 2) * 8, 48, 8, 16, 10 + i * 16, 8, 16, 32);
    }

    velX = Player->velocity.x;
    velY = Player->velocity.y;

    if(velX >= 0)
    {
        x1 = Player->rect.right / GRID_SPSIZE;
        x2 = (Player->rect.right + velX) / GRID_SPSIZE;
        dx = x2 - x1;
        sx = 1;
    }
    else
    {
        x1 = Player->rect.left / GRID_SPSIZE;
        x2 = (Player->rect.left + velX) / GRID_SPSIZE;
        dx = x1 - x2;
        sx = -1;
    }
    if(velY >= 0)
    {
        y1 = Player->rect.bottom / GRID_SPSIZE - 1;
        y2 = (Player->rect.bottom + velY) / GRID_SPSIZE - 1;
        dy = y1 - y2;
        sy = 1;
    }
    else
    {
        y1 = Player->rect.top / GRID_SPSIZE;
        y2 = (Player->rect.top + velY) / GRID_SPSIZE;
        dy = y2 - y1;
        sy = -1;
    }

    //printf("[%d, %d]->[%d, %d]\n", x1, y1, x2, y2);

    grid = Grid_Get() + x1 + y1 * GRID_WIDTH;
    err = dx + dy;
    yInc = GRID_WIDTH * sy;

    JGNoFill();
    JGStroke(JGBLUE);
    JGLine((Player->rect.right  + Player->rect.left) * 0.5d - cam.x,
           (Player->rect.bottom + Player->rect.top)  * 0.5d - cam.y,
           (Player->rect.right  + Player->rect.left) * 0.5d - cam.x + velX,
           (Player->rect.bottom + Player->rect.top)  * 0.5d - cam.y + velY);
    if(0)while(x1 != x2 || y1 != y2)
    {
        //if(*grid)
        //    return 0;
        err2 = 2 * err;
        if(err2 >= dy) /* e_xy + e_x > 0 */
        {
            JGRect(x1 * GRID_SPSIZE - cam.x,
                 y1 * GRID_SPSIZE - cam.y,
                 (x1 + 1) * GRID_SPSIZE - cam.x,
                 (y1 + 1) * GRID_SPSIZE - cam.y);
            err += dy;
            x1 += sx;
            grid += sx;
        }
        if(err2 <= dx) /* e_xy + e_y < 0 */
        {
            JGRect(x1 * GRID_SPSIZE - cam.x,
                 y1 * GRID_SPSIZE - cam.y,
                 (x1 + 1) * GRID_SPSIZE - cam.x,
                 (y1 + 1) * GRID_SPSIZE - cam.y);
            err += dx;
            y1 += sy;
            grid += yInc;
        }
    }

    JGStroke(JGRED);
    Object_DrawHurtBoxes();

    switch(Game_GetState())
    {
    case GS_RUNNING: break;
    case GS_PAUSED: break;
    case GS_GAMEOVER: break;
    }
}

static time_t nextTime = 1200;

static _Bool isMouseDown;
static time_t mouseDownTime;

_Bool IsMouseDown(void)
{
    return isMouseDown;
}

time_t JGGetMouseDownTime(void)
{
    return mouseDownTime;
}

char *Game_GetKeys(void)
{
    return KeyStates;
}

void JGStep(time_t runtime, time_t delta)
{
    OBJECT *sand;

    if(isMouseDown)
        mouseDownTime += delta;

    switch(Game_GetState())
    {
    case GS_RUNNING:
        nextTime -= delta;
        if(nextTime < 0)
        {
            sand = Object_Create("FallingSand");
            sand->rect.left  = 100.0d;
            sand->rect.right = 132.0d;
            nextTime = 1200 + nextTime;
        }
        Objects_Update(delta);
        break;
    case GS_PAUSED:
    case GS_GAMEOVER:
        break;
    }
}

static inline void ForwardEvent(const JGEVENT *event)
{
    OBJECT **objects, *object;
    int cnt, cnt2;
    int index;
    objects = Objects_Base();
    cnt = cnt2 = Objects_Count();
    index = 0;
    //printf("%I64d, %d\n", objects, cnt);
    while(cnt--)
    {
        object = *objects;
        if(!(object->properties & OBJPROP_DEACTIVATED))
            object->ObjectHandle(object, NULL, event->id, (uintptr_t) event);
        index++;
        if(cnt2 != Objects_Count())
        {
            cnt = cnt2 - index;
            objects = Objects_Base() + index;
            continue;
        }
        objects++;
    }
}

void JGEvent(const JGEVENT *event)
{
    switch(event->id)
    {
    case JGEVENT_SETCURSOR:
        JGSetCursor(JGCURSOR_DEFAULT);
        break;
    case JGEVENT_MOUSEPRESSED:
        isMouseDown = 1;
        mouseDownTime = 0;
        break;
    case JGEVENT_MOUSERELEASED:
        isMouseDown = 0;
        break;
    case JGEVENT_KEYPRESSED:  KeyStates[event->vkCode] = 1; break;
    case JGEVENT_KEYRELEASED:
        if(event->vkCode == 'W' && (Player->properties & OBJPROP_JUMP) && Player->velocity.y < 0.0d)
            Player->velocity.y *= 0.45d;
        KeyStates[event->vkCode] = 0;
        break;
    }
    ForwardEvent(event);
}

int main(int argc, char **argv)
{
    JGInit(argc, argv);
    JGSetFPSLimit(30.0d);
    JGRun();
}
