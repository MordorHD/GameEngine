#include "jg.h"
#include <math.h>

static color_t *Buffer; color_t *JGGetBuffer(void) { return Buffer; }
static int BufferWidth; int JGGetBufferWidth(void) { return BufferWidth; }
static int BufferHeight; int JGGetBufferHeight(void) { return BufferHeight; }
static int GlobalAlpha = 255;
static color_t Fill, Stroke;
static JGRECT Clip;

void JGFill(color_t fill)
{
    Fill = fill;
}

void JGStroke(color_t stroke)
{
    Stroke = stroke;
}

bool JGClipRect(const JGRECT *rect)
{
    JGRECT r = (JGRECT) {
        .left = 0,
        .top = 0,
        .right = BufferWidth,
        .bottom = BufferHeight
    };
    return JGRectIntersection(rect, &r, &Clip);
}

void __JGSetBuffer(color_t *buffer, int width, int height)
{
    BufferWidth = width;
    BufferHeight = height;
    Buffer = buffer;
    Clip = (JGRECT) {
        .left = 0,
        .top = 0,
        .right = width,
        .bottom = height
    };
}

JGIMAGE JGCreateImage(int width, int height, color_t *pixels)
{
    JGIMAGE img = malloc(sizeof(*img));
    img->width = width;
    img->height = height;
    img->pixels = pixels;
    return img;
}

JGIMAGE JGCreateImageSection(const JGIMAGE srcImg, int x, int y, int width, int height)
{
    JGIMAGE destImg;
    const color_t *src;
    color_t *dest, *base;
    int w, h, offset;

    src = srcImg->pixels + x + y * srcImg->width;
    offset = srcImg->width - width;
    base = dest = malloc(4 * width * height);
    h = height;
    while(h--)
    {
        w = width;
        while(w--)
            *(dest++) = *(src++);
        src += offset;
    }

    destImg = malloc(sizeof(*destImg));
    destImg->width = width;
    destImg->height = height;
    destImg->pixels = base;
    return destImg;
}

JGTEXTURE JGCreateColoredTexture(color_t color, float ratioX, float ratioY, int offsetX, int offsetY)
{
    JGTEXTURE texture = malloc(sizeof(*texture));
    texture->type = JGTEXTURE_COLOR;
    texture->color = color;
    texture->ratioX = ratioX;
    texture->ratioY = ratioY;
    texture->offsetX = offsetX;
    texture->offsetY = offsetY;
    return texture;
}

JGTEXTURE JGCreateImageTexture(JGIMAGE image, float ratioX, float ratioY, int offsetX, int offsetY, bool fill)
{
    JGTEXTURE texture = malloc(sizeof(*texture));
    texture->type = JGTEXTURE_IMAGE + fill;
    texture->image = image;
    texture->ratioX = ratioX;
    texture->ratioY = ratioY;
    texture->offsetX = offsetX;
    texture->offsetY = offsetY;
    return texture;
}

JGTEXTURE JGCreateImageTextureEx(string_t filePath, int width, int height, float ratioX, float ratioY, int offsetX, int offsetY, bool fill)
{
    JGTEXTURE texture;
    JGIMAGE image;
    color_t *dest;
    int w, h;
    int xr, yr;
    int x, y;

    image = JGLoadImage(filePath);
    dest = malloc(4 * width * height);
    xr = (image->width << 16) / width + 1;
    yr = (image->height << 16) / height + 1;
    dest += width * height;
    h = height;
    while(h--)
    {
        w = width;
        while(w--)
        {
            x = (w * xr) >> 16;
            y = (h * yr) >> 16;
            *(--dest) = *(image->pixels + x + y * image->width);
        }
    }

    free(image->pixels);

    texture = malloc(sizeof(*texture));
    texture->type = JGTEXTURE_IMAGE + fill;
    image->width = width;
    image->height = height;
    image->pixels = dest;
    texture->image = image;
    texture->ratioX = ratioX;
    texture->ratioY = ratioY;
    texture->offsetX = offsetX;
    texture->offsetY = offsetY;
    return texture;
}

struct __JGWrapper {
    JGIMAGE image;
    JGTEXTURE texture;
};

jgar_t __JGAnimatedTextureHandle(jgam_t msg, time_t delta, JGKEYFRAME *frame)
{
    struct __JGWrapper *wrapper;
    switch(msg)
    {
    case JGKFH_START:
        wrapper = frame->param;
        wrapper->texture->image = wrapper->image;
    default:
        break;
    }
    return JGKFH_OK;
}

JGTEXTURE JGCreateAnimatedTexture(JGIMAGE *frames, int frameCount, int frameTime, bool cycle, float ratioX, float ratioY, int offsetX, int offsetY)
{
    JGTEXTURE texture;
    JGANIMATION animation;
    JGKEYFRAME *keyFrames;
    struct __JGWrapper *wrapper;
    int i;
    texture = malloc(sizeof(*texture));
    keyFrames = malloc(sizeof(*keyFrames) * frameCount);
    texture->type = JGTEXTURE_ANIMATED;
    texture->image = *frames;
    for(i = 0; i < frameCount; i++)
    {
        keyFrames[i].runtime = 0;
        keyFrames[i].duration = frameTime;
        wrapper = malloc(sizeof(*wrapper));
        wrapper->image = *(frames + i);
        wrapper->texture = texture;
        keyFrames[i].param = wrapper;
        if(i + 1 == frameCount)
            keyFrames[i].nextFrame = cycle ? keyFrames : NULL;
        else
            keyFrames[i].nextFrame = keyFrames + i + 1;
        keyFrames[i].handle = __JGAnimatedTextureHandle;
    }
    texture->ratioX = ratioX;
    texture->ratioY = ratioY;
    texture->offsetX = offsetX;
    texture->offsetY = offsetY;
    JGAnimation_Init(&animation, keyFrames);
    JGAnimation_Play(&animation);
    return texture;
}

JGTEXTURE JGCreateAnimatedTextureEx(JGIMAGE source, int x, int y, int width, int height, int nx, int ny, int frameTime, bool cycle, float ratioX, float ratioY, int offsetX, int offsetY)
{
    JGTEXTURE texture;
    JGANIMATION animation;
    JGKEYFRAME *keyFrames;
    struct __JGWrapper *wrapper;
    int ix, iy, i;
    int frameCount;
    texture = malloc(sizeof(*texture));
    frameCount = nx * ny;
    keyFrames = malloc(sizeof(*keyFrames) * frameCount);
    texture->type = JGTEXTURE_ANIMATED;
    for(iy = 0; iy < ny; iy++)
    for(ix = 0; ix < nx; ix++)
    {
        i = ix + iy * nx;
        keyFrames[i].runtime = 0;
        keyFrames[i].duration = frameTime;
        wrapper = malloc(sizeof(*wrapper));
        wrapper->image = JGCreateImageSection(source, x + width * ix, y + height * iy, width, height);
        if(!i)
            texture->image = wrapper->image;
        wrapper->texture = texture;
        keyFrames[i].param = wrapper;
        if(i + 1 == frameCount)
            keyFrames[i].nextFrame = cycle ? keyFrames : NULL;
        else
            keyFrames[i].nextFrame = keyFrames + i + 1;
        keyFrames[i].handle = __JGAnimatedTextureHandle;
    }
    texture->ratioX = ratioX;
    texture->ratioY = ratioY;
    texture->offsetX = offsetX;
    texture->offsetY = offsetY;
    JGAnimation_Init(&animation, keyFrames);
    JGAnimation_Play(&animation);
    return texture;
}

// combines two pixels considering the global alpha and the sources alpha value
#define _JGCOMBINE(dest, src) { \
    const uint8_t *pSrc = (uint8_t*) (src); \
    uint8_t *pDest = (uint8_t*) (dest); \
    uint8_t pAlpha = (GlobalAlpha * *(pSrc + 3)) >> 8; \
    *pDest = ((int) *pDest * (0x100 - pAlpha) + (int) *pSrc * pAlpha) >> 8; \
    pDest++; pSrc++; \
    *pDest = ((int) *pDest * (0x100 - pAlpha) + (int) *pSrc * pAlpha) >> 8; \
    pDest++; pSrc++; \
    *pDest = ((int) *pDest * (0x100 - pAlpha) + (int) *pSrc * pAlpha) >> 8; }

#define _JGCOMBINEINC(dest, src) { \
    const uint8_t *pSrc = (uint8_t*) (src); \
    uint8_t pAlpha = (GlobalAlpha * *(pSrc + 3)) >> 8; \
    *((uint8_t*) dest) = (*((uint8_t*) dest) * (0x100 - pAlpha) + *pSrc * pAlpha) >> 8; \
    dest = (void*) dest + 1; pSrc++; \
    *((uint8_t*) dest) = (*((uint8_t*) dest) * (0x100 - pAlpha) + *pSrc * pAlpha) >> 8; \
    dest = (void*) dest + 1; pSrc++; \
    *((uint8_t*) dest) = (*((uint8_t*) dest) * (0x100 - pAlpha) + *pSrc * pAlpha) >> 8; \
    dest = (void*) dest + 2; }

static inline void __JGRect(int left, int top, int right, int bottom)
{
    int width;
    int offset;
    int w, h;
    color_t *dest;
    dest = Buffer + left + BufferWidth * top;
    width = right - left;
    offset = BufferWidth - width;
    h = bottom - top;
    while(h--)
    {
        w = width;
        while(w--)
            _JGCOMBINEINC(dest, &Fill);
        dest += offset;
        top++;
    }
}

bool JGRect(int left, int top, int right, int bottom)
{
    color_t *dest;
    int cnt;
    JGRECT r;
    r = (JGRECT) {
        .left = left,
        .top = top,
        .right = right,
        .bottom = bottom
    };
    if(!JGRectIntersection(&Clip, &r, &r))
        return 0;
    if(Fill)
        __JGRect(r.left, r.top, r.right, r.bottom);
    if(!Stroke)
        return 1;
    dest = Buffer + r.left + BufferWidth * r.top;
    cnt = r.bottom - r.top;
    if(left >= 0)
    {
        while(--cnt)
        {
            _JGCOMBINE(dest, &Stroke);
            dest += BufferWidth;
        }
    }
    else
    {
        dest += BufferWidth * (cnt - 1);
    }
    cnt = r.right - r.left;
    if(bottom < BufferHeight)
    {
        while(--cnt)
        {
            _JGCOMBINE(dest, &Stroke);
            dest++;
        }
    }
    else
    {
        dest += cnt - 1;
    }
    cnt = r.bottom - r.top;
    if(right < BufferWidth)
    {
        while(--cnt)
        {
            _JGCOMBINE(dest, &Stroke);
            dest -= BufferWidth;
        }
    }
    else
    {
        dest -= BufferWidth * (cnt - 1);
    }
    if(top >= 0)
    {
        cnt = r.right - r.left;
        while(--cnt)
        {
            _JGCOMBINE(dest, &Stroke);
            dest--;
        }
    }
    return 1;
}

bool JGOval(int left, int top, int right, int bottom)
{
    int cx, cy, rx, ry;
    int dx, dy, d, x, y, t, c;
    int rx2, ry2;
    int lc, rc;
    color_t *uDest, *lDest;
    rx = (right - left) / 2;
    ry = (bottom - top) / 2;
    cx = left;
    cy = top;
    left = __max(left, Clip.left);
    top = __max(top, Clip.top);
    right = __min(right, Clip.right);
    bottom = __min(bottom, Clip.bottom);
    if(left > right || top > bottom)
        return 0;
    cx += rx;
    cy += ry;
    rx2 = rx * rx;
    ry2 = ry * ry;
    x = 0;
    y = ry;
    d = ry2 - rx2 * ry + rx2 / 4;
    dx = 2 * ry2 * x;
    dy = 2 * rx2 * y;
    uDest = lDest = Buffer + cx + cy * BufferWidth;
    uDest -= ry * BufferWidth;
    lDest += ry * BufferWidth;
    while(dx < dy)
    {
        dx += 2 * ry2;
        if(d >= 0)
        {
            lc = cx - x;
            lc = lc < 0 ? lc + 1 : 0;
            rc = cx + x - Clip.right;
            rc = rc >= 0 ? rc + 1 : 0;
            c = 2 * x - rc;
            if(c > -lc)
            {
                uDest += c;
                lDest += c;
                c += lc;
                while(c--)
                {
                    if(cy - y >= Clip.top && cy - y < Clip.bottom)
                        _JGCOMBINE(uDest, &Fill);
                    if(cy + y >= Clip.top && cy + y < Clip.bottom)
                        _JGCOMBINE(lDest, &Fill);
                    uDest--;
                    lDest--;
                }
                uDest += BufferWidth + lc;
                lDest -= BufferWidth - lc;
            }
            else
            {
                uDest += BufferWidth;
                lDest -= BufferWidth;
            }
            y--;
            dy -= 2 * rx2;
            d -= dy;
        }
        uDest--;
        lDest--;
        x++;
        d += dx + ry2;
    }
    d = x * (x + 1);
    d *= ry2;
    t = y - 1;
    t *= t;
    d += rx2 * t;
    d -= rx2 * ry2;
    while(y >= 0)
    {
        lc = cx - x;
        lc = lc < 0 ? lc + 1 : 0;
        rc = cx + x - Clip.right + 1;
        rc = rc > 0 ? rc : 0;
        c = 2 * x - rc;
        if(c > -lc)
        {
            uDest += c;
            lDest += c;
            c += lc;
            while(c--)
            {
                if(!y)
                {
                    if(cy >= Clip.top && cy < Clip.bottom)
                        _JGCOMBINE(uDest, &Fill);
                }
                else
                {
                    if(cy - y >= Clip.top && cy - y < Clip.bottom)
                        _JGCOMBINE(uDest, &Fill);
                    if(cy + y >= Clip.top && cy + y < Clip.bottom)
                        _JGCOMBINE(lDest, &Fill);
                }
                uDest--;
                lDest--;
            }
            uDest += BufferWidth + lc;
            lDest -= BufferWidth - lc;
        }
        else
        {
            uDest += BufferWidth;
            lDest -= BufferWidth;
        }
        y--;
        dy -= 2 * rx2;
        if(d <= 0)
        {
            x++;
            uDest--;
            lDest--;
            dx += 2 * ry2;
            d += dx;
        }
        d += rx2 - dy;
    }
    return 1;
}

#define INSIDE 0x0
#define LEFT 0x1
#define RIGHT 0x2
#define TOP 0x4
#define BOTTOM 0x8

static char ComputeCode(int x, int y)
{
    char code = INSIDE;

    if(x < Clip.left)
        code |= LEFT;
    else if(x > Clip.right)
        code |= RIGHT;
    if(y < Clip.top)
        code |= BOTTOM;
    else if(y > Clip.bottom)
        code |= TOP;

    return code;
}

bool JGLine(int x1, int y1, int x2, int y2)
{
    color_t *dest;
    int err, err2;
    int dx, dy;
    int sx, sy;
    char code1 = ComputeCode(x1, y1);
    char code2 = ComputeCode(x2, y2);
    char code_out;
    int x = 0, y = 0;
    while(1)
    {
        if(!(code1 | code2))
            break;
        if(code1 & code2)
            return 0;

        code_out = code1 ? code1 : code2;
        if(code_out & TOP)
        {
            x = x1 + (x2 - x1) * (Clip.bottom - y1) / (y2 - y1);
            y = Clip.bottom;
        }
        else if(code_out & BOTTOM)
        {
            x = x1 + (x2 - x1) * (Clip.top - y1) / (y2 - y1);
            y = Clip.top;
        }
        else if(code_out & RIGHT)
        {
            y = y1 + (y2 - y1) * (Clip.right - x1) / (x2 - x1);
            x = Clip.right;
        }
        else if(code_out & LEFT)
        {
            y = y1 + (y2 - y1) * (Clip.left - x1) / (x2 - x1);
            x = Clip.left;
        }
        if(code1)
        {
            x1 = x;
            y1 = y;
            code1 = ComputeCode(x1, y1);
        }
        else
        {
            x2 = x;
            y2 = y;
            code2 = ComputeCode(x2, y2);
        }
    }
    if(x1 < x2)
    {
        dx = x2 - x1;
        sx = 1;
    }
    else
    {
        dx = x1 - x2;
        sx = -1;
    }
    if(y1 < y2)
    {
        dy = y1 - y2;
        sy = 1;
    }
    else
    {
        dy = y2 - y1;
        sy = -1;
    }

    err = dx + dy;

    dest = Buffer + x1 + y1 * BufferWidth;

    while(1)
    {
        _JGCOMBINE(dest, &Stroke);
        if(x1 == x2 && y1 == y2)
            break;
        err2 = 2 * err;
        if(err2 >= dy) /* e_xy + e_x > 0 */
        {
            err += dy;
            x1 += sx;
            dest += sx;
        }
        if(err2 <= dx) /* e_xy + e_y < 0 */
        {
            err += dx;
            y1 += sy;
            dest += BufferWidth * sy;
        }
    }
    return 1;
}

bool JGDrawImage(const JGIMAGE image, int x, int y, int width, int height)
{
    color_t *dest;
    int w, offset;
    int absWidth;
    int xr, yr;
    int x2, y2;
    JGRECT r;
    absWidth = abs(width);
    r = (JGRECT) {
        .left = x,
        .top = y,
        .right = x + absWidth,
        .bottom = y + height
    };
    if(!JGRectIntersection(&Clip, &r, &r))
        return 0;
    dest = Buffer + r.right + (r.bottom - 1) * BufferWidth;
    xr = (image->width << 16) / absWidth + 1;
    yr = (image->height << 16) / height + 1;
    absWidth = r.right - r.left;
    height = r.bottom - r.top;
    offset = BufferWidth - absWidth;
    while(height--)
    {
        w = absWidth;
        while(w--)
        {
            x2 = (((w + r.left - x) * xr) >> 16);
            y2 = ((height + r.top - y) * yr) >> 16;
            dest--;
            if(width < 0)
                x2 = image->width - 1 - x2;
            _JGCOMBINE(dest, image->pixels + x2 + y2 * image->width);
        }
        dest -= offset;
    }
    return 1;
}

bool JGDrawImageSection(const JGIMAGE image, int imgX, int imgY, int imgWidth, int imgHeight, int x, int y, int width, int height)
{
    color_t *src, *dest;
    int w, h;
    int absWidth;
    int xi;
    int xr, yr;
    int x2, y2;
    JGRECT r;
    absWidth = abs(width);
    r = (JGRECT) {
        .left = x,
        .top = y,
        .right = x + absWidth,
        .bottom = y + height
    };
    if(!JGRectIntersection(&Clip, &r, &r))
        return 0;
    dest = Buffer + r.left + (r.bottom - 1) * BufferWidth;
    h = r.bottom - r.top;
    w = r.right - r.left;
    xr = (imgWidth << 16) / absWidth + 1;
    yr = (imgHeight << 16) / height + 1;
    x2 = ((r.left - x) * xr) >> 16;
    y2 = ((r.top - y) * yr) >> 16;
    src = image->pixels;
    src += imgX + x2 + (imgY + y2) * image->width;
    while(h--)
    {
        xi = w;
        while(xi--)
        {
            x2 = (xi * xr) >> 16;
            y2 = (h * yr) >> 16;
            if(width < 0)
                x2 = image->width - 1 - x2;
            _JGCOMBINE(dest + xi, src + x2 + y2 * image->width);
        }
        dest -= BufferWidth;
    }
    return 1;
}

bool JGDrawTexture(const JGTEXTURE texture, int x, int y, int width, int height)
{
    JGRECT r;
    color_t *dest;
    int offset;
    int ox, oy;
    //x += texture->offsetX;
    //y -= height - 5.0f * texture->ratioY;
    //width *= texture->ratioX;
    //height *= texture->ratioY;
    switch(texture->type)
    {
    case JGTEXTURE_COLOR:
        Fill = texture->color;
        if(width < 0)
            width = -width;
        return JGRect(x, y, x + width, y + height);
    case JGTEXTURE_IMAGE:
        if(width < 0)
            width = -width;
        r = (JGRECT) {
            .left = x,
            .top = y,
            .right = x + width,
            .bottom = y + height
        };
        if(!JGRectIntersection(&Clip, &r, &r))
            return 0;
        dest = Buffer + r.left + r.top * BufferWidth;
        offset = BufferWidth - (r.right - r.left);
        ox = x < Clip.left ? ((Clip.left - x) % texture->image->width) : 0;
        oy = y < Clip.top ? ((Clip.top - y) % texture->image->height) : 0;
        ox -= r.left;
        oy -= r.top;
        for(y = r.top; y < r.bottom; y++, dest += offset)
        for(x = r.left; x < r.right; x++)
            _JGCOMBINEINC(dest, texture->image->pixels +
                          ((x + ox) % texture->image->width) +
                          ((y + oy) % texture->image->height) * texture->image->width);
        return 1;
    case JGTEXTURE_ANIMATED:
    case JGTEXTURE_FILLIMAGE:
        return JGDrawImage(texture->image, x, y, width, height);
    default:
        return 0;
    }
}

bool JGGradient(int x1, int y1, color_t color1, int x2, int y2, color_t color2, int type)
{
    color_t *dest;
    uint8_t color[4];
    float a, r, g, b;
    float as, rs, gs, bs;
    int dy;
    int h;
    int xi, w, offset;
    JGRECT rect;
    rect = (JGRECT) {
        .left = x1,
        .top = y1,
        .right = x2,
        .bottom = y2
    };
    if(!JGRectIntersection(&Clip, &rect, &rect))
        return 0;
    dy = y2 - y1;
    a = (color1 & 0xFF000000) >> 24;
    r = (color1 & 0xFF0000) >> 16;
    g = (color1 & 0xFF00) >> 8;
    b = (color1 & 0xFF);
    as = ((color2 & 0xFF000000) >> 24) - a;
    rs = ((color2 & 0xFF0000) >> 16) - r;
    gs = ((color2 & 0xFF00) >> 8) - g;
    bs = (color2 & 0xFF) - b;
    as /= dy;
    rs /= dy;
    gs /= dy;
    bs /= dy;
    dy = y1 - rect.top;
    r += rs * dy;
    g += gs * dy;
    b += bs * dy;
    dest = Buffer + rect.left + rect.top * BufferWidth;
    h = rect.bottom - rect.top;
    w = rect.right - rect.left;
    offset = BufferWidth - w;
    while(h--)
    {
        color[3] = a;
        color[2] = r;
        color[1] = g;
        color[0] = b;
        xi = w;
        while(xi--)
            _JGCOMBINEINC(dest, color);
        a += as;
        r += rs;
        g += gs;
        b += bs;
        dest += offset;
    }
    return 1;
}
