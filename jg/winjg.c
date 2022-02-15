#include "jg.h"
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

static volatile bool running;
static volatile time_t uframeTime;
static pthread_t thread_id;

static HWND Window;
static HDC BufferDc;
static HDC WindowDc;

static int BufferWidth;
static int BufferHeight;
static float BufferStretchX;
static float BufferStretchY;

static volatile JGCAMERA *Camera;

static XFORM Transform;

void JGGetWindowSize(JGSIZE *size)
{
    RECT r;
    GetClientRect(Window, &r);
    size->width = r.right;
    size->height = r.bottom;
}

static LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
static void *Run(void*);

void JGInit(int argc, char **argv)
{
    __JGControlInit();

    WNDCLASS wc = {0};
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_DBLCLKS;
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
    wc.lpszClassName = "JGWC_MAIN_WINDOW";
    wc.lpfnWndProc = MainWndProc;
    RegisterClass(&wc);

    Window = CreateWindow("JGWC_MAIN_WINDOW", "Title", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);
    WindowDc = GetDC(Window);

    memset((void*) (Camera = malloc(sizeof(*Camera))), 0, sizeof(*Camera));

}

void JGSetBufferSize(int width, int height)
{
    HBITMAP buffer;
    BufferDc = CreateCompatibleDC(WindowDc);
    buffer = CreateCompatibleBitmap(WindowDc, BufferWidth = width, BufferHeight = height);
    SelectObject(BufferDc, buffer);
    SelectObject(BufferDc, GetStockObject(DC_BRUSH));
    SelectObject(BufferDc, GetStockObject(DC_PEN));
    SetBkMode(BufferDc, TRANSPARENT);
    SetTextAlign(BufferDc, TA_CENTER | TA_BASELINE);
    SetGraphicsMode(BufferDc, GM_ADVANCED);
    SetGraphicsMode(WindowDc, GM_ADVANCED);
}

void JGCameraTranslate(float x, float y)
{
    Camera->x += x;
    Camera->y += y;
}

void JGSetCameraPos(float x, float y)
{
    Camera->x = x;
    Camera->y = y;
}

void JGSetCamera(JGCAMERA *newCamera)
{
    Camera->x = newCamera->x;
    Camera->y = newCamera->y;
    Camera->cFlags = newCamera->cFlags;
    Camera->cstr_l = newCamera->cstr_l;
    Camera->cstr_t = newCamera->cstr_t;
    Camera->cstr_r = newCamera->cstr_r;
    Camera->cstr_b = newCamera->cstr_b;
    Camera->target = newCamera->target;
}

void JGGetCamera(JGCAMERA *camera)
{
    camera->x = Camera->x;
    camera->y = Camera->y;
    camera->cFlags = Camera->cFlags;
    camera->cstr_l = Camera->cstr_l;
    camera->cstr_t = Camera->cstr_t;
    camera->cstr_r = Camera->cstr_r;
    camera->cstr_b = Camera->cstr_b;
    camera->target = Camera->target;
}

void JGCameraSetTarget(JGPOINT2D *p)
{
    Camera->target = p;
}

void JGResetCamera(void)
{
    Camera->x = 0;
    Camera->y = 0;
    Camera->target = NULL;
    SetWorldTransform(BufferDc, &Transform);
}

static int SaveSate;

void JGSave(void)
{
    SaveSate = SaveDC(BufferDc);
}

void JGRestore(void)
{
    RestoreDC(BufferDc, SaveSate);
}

JGIMAGE JGCreateImage(int width, int height, color_t *pixels, bool flipped)
{
    JGIMAGE img = malloc(sizeof(*img));
    HDC hdc = GetDC(Window);

    BITMAP bmp;
    bmp.bmType = 0;
    bmp.bmWidth = width;
    bmp.bmHeight = height;
    bmp.bmWidthBytes = width * 4;
    bmp.bmPlanes = 1;
    bmp.bmBitsPixel = 32;
    bmp.bmBits = pixels;
    img->bmp = bmp;

    img->imageDc = CreateCompatibleDC(hdc);
    img->hbmp = CreateBitmapIndirect(&bmp);
    SelectObject(img->imageDc, img->hbmp);

    img->flippedDc = CreateCompatibleDC(hdc);
    img->flippedHbmp = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(img->flippedDc, img->flippedHbmp);
    StretchBlt(img->flippedDc, width, 0, -width, height, img->imageDc, 0, 0, width, height, SRCCOPY);

    ReleaseDC(Window, hdc);
    return img;
}

JGIMAGE JGCreateImageSection(JGIMAGE src, int x, int y, int width, int height)
{
    JGIMAGE img = malloc(sizeof(*img));
    HDC hdc = GetDC(Window);

    img->imageDc = CreateCompatibleDC(hdc);
    img->hbmp = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(img->imageDc, img->hbmp);

    GetObject(img->hbmp, sizeof(BITMAP), &img->bmp);

    BitBlt(img->imageDc, 0, 0, width, height, src->imageDc, x, y, SRCCOPY);

    ReleaseDC(Window, hdc);
    return img;
}

/*JGIMAGE JGLoadImage(const char *filePath, bool flip)
{
    HDC winDc;
    JGIMAGE img;
    HBITMAP hbmp = LoadImage(NULL, filePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    winDc = GetDC(NULL);
    img = malloc(sizeof(*img));
    img->hbmp = hbmp;
    GetObject(hbmp, sizeof(img->bmp), &img->bmp);
    SelectObject(img->imageDc = CreateCompatibleDC(winDc), hbmp);
    if(flip)
    {
        img->flippedHbmp = CreateCompatibleBitmap(winDc, img->bmp.bmWidth, img->bmp.bmHeight);
        SelectObject(img->flippedDc = CreateCompatibleDC(winDc), img->flippedHbmp);
        StretchBlt(img->flippedDc, 0, 0, img->bmp.bmWidth, img->bmp.bmHeight,
                   img->imageDc, img->bmp.bmWidth, 0, -img->bmp.bmWidth, img->bmp.bmHeight, SRCCOPY);
    }
    else
    {
        img->flippedDc = NULL;
        img->flippedHbmp = NULL;
    }
    ReleaseDC(NULL, winDc);
    return img;
}*/

void JGCopyImage(JGIMAGE destImg, int x1, int y1, int width1, int height1, JGIMAGE srcImg, int x2, int y2, int width2, int height2)
{
    TransparentBlt(destImg->imageDc, x1, y1, width1, height1, srcImg->imageDc, x2, y2, width2, height2, 0);
}

void JGGetImageSize(const JGIMAGE img, JGSIZE *size)
{
    size->width = img->bmp.bmWidth;
    size->height = img->bmp.bmHeight;
}

void JGClip(int left, int top, int right, int bottom)
{
    HRGN hRgn = CreateRectRgn(left, top, right, bottom);
    SelectClipRgn(BufferDc, hRgn);
}

void JGClipRect(const JGRECT *rect)
{
    HRGN hRgn = rect ? CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom) : NULL;
    SelectClipRgn(BufferDc, hRgn);
}

void JGDrawImage(JGIMAGE img, int x, int y, int width, int height)
{
    TransparentBlt(BufferDc, x, y, width, height, img->imageDc, 0, 0, img->bmp.bmWidth, img->bmp.bmHeight, 0);
}

void JGDrawImageFlipped(JGIMAGE img, int x, int y, int width, int height)
{
    TransparentBlt(BufferDc, x, y, width, height, img->flippedDc, 0, 0, img->bmp.bmWidth, img->bmp.bmHeight, 0);
}

void JGDrawImageSection(JGIMAGE img, int x1, int y1, int width1, int height1, int x, int y, int width, int height)
{
    TransparentBlt(BufferDc, x, y, width, height, img->imageDc, x1, y1, width1, height1, 0);
}

void JGDrawImageSectionFlipped(JGIMAGE img, int x1, int y1, int width1, int height1, int x, int y, int width, int height)
{
    x1 = img->bmp.bmWidth - x1 - width1;
    TransparentBlt(BufferDc, x, y, width, height, img->flippedDc, x1, y1, width1, height1, 0);
}

void JGFill(color_t fill)
{
    SelectObject(BufferDc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(BufferDc, fill);
}

void JGNoFill(void)
{
    SelectObject(BufferDc, GetStockObject(NULL_BRUSH));
}

void JGSetStroke(int style, int width, color_t color)
{
    static HPEN lastPen = NULL;
    DeleteObject(lastPen);
    lastPen = CreatePen(style, width, color);
    SelectObject(BufferDc, lastPen);
}

void JGStroke(color_t stroke)
{
    SelectObject(BufferDc, GetStockObject(DC_PEN));
    SetDCPenColor(BufferDc, stroke);
}

void JGNoStroke(void)
{
    SelectObject(BufferDc, GetStockObject(NULL_PEN));
}

void JGTextAlign(int textAlign)
{
    SetTextAlign(BufferDc, textAlign);
}

void JGTextColor(color_t textColor)
{
    SetTextColor(BufferDc, textColor);
}

void JGFont(const char *name, int size)
{
    static HFONT lastFont = NULL;
    DeleteObject(lastFont);
    lastFont = CreateFont(size, 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, name);
    SelectObject(BufferDc, lastFont);
}

void JGSetFont(JGFONT font)
{
    SelectObject(BufferDc, font);
}

JGFONT JGCreateFont(const char *name, int size)
{
    return CreateFont(size, 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, name);
}

int JGTextWidth(string_t text, int len)
{
    SIZE s;
    GetTextExtentPoint32(BufferDc, text, len, &s);
    return s.cx;
}

int JGTextHeight(void)
{
    TEXTMETRIC tm;
    GetTextMetrics(BufferDc, &tm);
    return tm.tmAscent + tm.tmDescent;
}

void JGText(string_t text, int len, int x, int y)
{
    TextOut(BufferDc, x, y, text, len);
}

void JGRect(int left, int top, int right, int bottom)
{
    Rectangle(BufferDc, left, top, right + 1, bottom + 1);
}

void JGOval(int left, int top, int right, int bottom)
{
    Ellipse(BufferDc, left, top, right, bottom);
}

void JGLine(int x1, int y1, int x2, int y2)
{
    MoveToEx(BufferDc, x1, y1, NULL);
    LineTo(BufferDc, x2, y2);
}

void JGGradient(int x1, int y1, color_t color1, int x2, int y2, color_t color2, int type)
{
    TRIVERTEX vert[2];
    vert[0] = (TRIVERTEX) {
        .x = x1,
        .y = y1,
        .Red = (color1 & 0xFF) << 8,
        .Green = color1 & 0xFF00,
        .Blue  = (color1 >> 8) & 0xFF00,
        .Alpha = 0
    };
    vert[1] = (TRIVERTEX) {
        .x = x2,
        .y = y2,
        .Red = (color2 & 0xFF) << 8,
        .Green = color2 & 0xFF00,
        .Blue  = (color2 >> 8) & 0xFF00,
        .Alpha = 0
    };
    GRADIENT_RECT gr = {
        .UpperLeft = 0,
        .LowerRight = 1
    };
    GradientFill(BufferDc, vert, 2, &gr, 1, GRADIENT_FILL_RECT_V);
}

void JGResetTransform(void)
{
    Transform.eM11 = 1.0f;
    Transform.eM12 = 0.0f;
    Transform.eM21 = 0.0f;
    Transform.eM22 = 1.0f;
    Transform.eDx  = 0.0f;
    Transform.eDy  = 0.0f;
    SetWorldTransform(BufferDc, &Transform);
}

void JGTranslate(float x, float y)
{
    Transform.eDx += x * Transform.eM11 + y * Transform.eM12;
    Transform.eDy += -x * Transform.eM21 + y * Transform.eM22;
    SetWorldTransform(BufferDc, &Transform);
}

void JGScale(float x, float y)
{
    Transform.eM11 *= x;
    Transform.eM12 *= x;
    Transform.eM21 *= y;
    Transform.eM22 *= y;
    SetWorldTransform(BufferDc, &Transform);
}

void JGShear(float x, float y)
{
    double t0, t1;
    t0 = Transform.eM11;
    t1 = Transform.eM12;
    Transform.eM11 = t0 + t1 * y;
    Transform.eM12 = t0 * x + t1;

    t0 = Transform.eM21;
    t1 = Transform.eM22;
    Transform.eM21 = t0 + t1 * y;
    Transform.eM22 = t0 * x + t1;
    SetWorldTransform(BufferDc, &Transform);
}

void JGRotate(float r)
{
    double sin, cos, t0, t1;
    sincos(r, &sin, &cos);
    t0 = Transform.eM11;
    t1 = Transform.eM12;
    Transform.eM11 =  cos * t0 + sin * t1;
    Transform.eM12 = -sin * t0 + cos * t1;
    t0 = Transform.eM21;
    t1 = Transform.eM22;
    Transform.eM21 =  cos * t0 + sin * t1;
    Transform.eM22 = -sin * t0 + cos * t1;
    SetWorldTransform(BufferDc, &Transform);
}

void JGRect2D(const JGRECT2D *r)
{
    JGRect((int) (r->left - Camera->x), (int) (r->top - Camera->y), (int) (r->right - Camera->x), (int) (r->bottom - Camera->y));
}

void JGOval2D(const JGELLIPSE2D *e)
{
    JGOval((int) (e->left - Camera->x), (int) (e->top - Camera->y), (int) (e->right - Camera->x), (int) (e->bottom - Camera->y));
}

void JGLine2D(const JGLINE2D *l)
{
    JGLine((int) (l->x1 - Camera->x), (int) (l->y1 - Camera->y), (int) (l->x2 - Camera->x), (int) (l->y2 - Camera->y));
}

void JGSetFPSLimit(double limit)
{
    assert(limit >= 1e-3 && "SetFPSLimit(limit), limit must be greater than 0.001");
    uframeTime = (time_t) round(limit > 1e3 ? 1 : 1e6 / limit);
}

void JGRun(void)
{
    if(running)
        return;
    ShowWindow(Window, SW_NORMAL);
    UpdateWindow(Window);
    running = 1;
    pthread_create(&thread_id, NULL, Run, NULL);
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void JGStop(void)
{ running = 0; SendMessage(Window, WM_CLOSE, 0, 0); }

#define SEC_TO_MICRO 1000000

static void *Run(void *arg)
{
    struct timespec start, end;
    time_t mdiff;
    time_t uruntime = 0, udiff = 0;
    time_t noverflow = 0;
    RECT wr;
    int cFlags;
    long x_align, y_align;
    double x, y, tx, ty;
    int l, t, r, b, drl, dbt;
    void *target;
    JGStart();
    while(running)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        mdiff = udiff / 1000;
        uruntime += udiff;
        JGStep(uruntime / 1000, mdiff);
        JGAnimation_Forward(mdiff);
        GetClientRect(Window, &wr);
        // update camera if it exists
        if(Camera && (target = Camera->target))
        {
            cFlags = Camera->cFlags;
            x = Camera->x;
            y = Camera->y;
            if(cFlags & JGCA_FTARGET)
            {
                tx = ((JGPOINT2D*) target)->x;
                ty = ((JGPOINT2D*) target)->y;
            }
            else
            {
                tx = (double) ((JGPOINT*) target)->x;
                ty = (double) ((JGPOINT*) target)->y;
            }
            if(fabs(x - tx) >= 0.1d || fabs(y - ty) >= 0.1d)
            {
                x_align = (cFlags & JGCA_HCENTER) ? (BufferWidth / 2) : (cFlags & JGCA_RIGHT)  ? BufferWidth  : 0;
                y_align = (cFlags & JGCA_VCENTER) ? (BufferHeight / 2) : (cFlags & JGCA_BOTTOM) ? BufferHeight : 0;
                l = Camera->cstr_l;
                r = Camera->cstr_r;
                t = Camera->cstr_t;
                b = Camera->cstr_b;
                drl = r - l;
                dbt = b - t;
                tx = drl <= BufferWidth ? 0
                        : -tx - x_align >= l - BufferWidth ? -l
                        : x_align + tx > r ? r - BufferWidth
                        : tx - x_align;
                ty = dbt <= BufferHeight ? 0
                        : -ty - y_align >= t - BufferHeight ? -t
                        : y_align + ty > b ? b - BufferHeight
                        : ty - y_align;
                Camera->x += (tx - x) * 0.1f;
                Camera->y += (ty - y) * 0.1f;
            }
        }
        // user draw
        JGDraw();
        // draw controls
        JGDispatchEvent(JGEVENT_REDRAW, NULL);
        // stretch buffer to window
        StretchBlt(WindowDc, 0, 0, wr.right, wr.bottom, BufferDc, 0, 0, BufferWidth, BufferHeight, SRCCOPY);
        //BitBlt(WindowDc, 0, 0, wr.right, wr.bottom, BufferDc, 0, 0, SRCCOPY);
        clock_gettime(CLOCK_MONOTONIC, &end);
        udiff = (end.tv_sec - start.tv_sec) * SEC_TO_MICRO + (noverflow + end.tv_nsec - start.tv_nsec) / 1000;
        noverflow = (end.tv_nsec - start.tv_nsec) % 1000;
        //printf("%d\n", uoverflow);
        if(uframeTime > udiff)
        {
            usleep(uframeTime - udiff);
            udiff = uframeTime;
        }
    }
    return NULL;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    JGEVENT event; event.id = 0;
    static int prevX, prevY;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT *rp;
    _Bool userDispatch = 1;
    switch(msg)
    {
    case WM_CREATE: JGSetCursor(JGCURSOR_DEFAULT); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_ERASEBKGND: return 1;
    case WM_SIZE:
        BufferStretchX = (float) BufferWidth / (float) LOWORD(lParam);
        BufferStretchY = (float) BufferHeight / (float) HIWORD(lParam);
        event.id = JGEVENT_SIZE;
        break;
    case WM_PAINT:
        rp = &ps.rcPaint;
        hdc = BeginPaint(hWnd, &ps);
        BitBlt(hdc, rp->left, rp->top, rp->right - rp->left, rp->bottom - rp->left, BufferDc, rp->left, rp->right, SRCCOPY);
        EndPaint(hWnd, &ps);
        return 0;
    case WM_SETCURSOR:
        event.id = JGEVENT_SETCURSOR;
        JGEvent(&event);
        JGDispatchEvent(JGEVENT_SETCURSOR, NULL);
        SetCursor(JGGetCursor());
        return 0;
    case WM_SETFOCUS:
        event.id = JGEVENT_FOCUSGAINED;
        break;
    case WM_KILLFOCUS:
        event.id = JGEVENT_FOCUSLOST;
        break;
    case WM_KEYDOWN:
        event.id = JGEVENT_KEYPRESSED;
        event.vkCode = wParam;
        event.keyChar = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
        event.flags = lParam;
        userDispatch = !JGGetKeyboardFocusControl();
        break;
    case WM_KEYUP:
        event.id = JGEVENT_KEYRELEASED;
        event.vkCode = wParam;
        event.keyChar = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
        event.flags = lParam;
        userDispatch = !JGGetKeyboardFocusControl();
        break;
    case WM_CHAR:
        event.id = JGEVENT_KEYTYPED;
        event.vkCode = wParam;
        event.keyChar = wParam;
        event.flags = lParam;
        userDispatch = !JGGetKeyboardFocusControl();
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
        SetCapture(hWnd);
        event.id = JGEVENT_MOUSEPRESSED;
        event.x = GET_X_LPARAM(lParam) * BufferStretchX + Camera->x;
        event.y = GET_Y_LPARAM(lParam) * BufferStretchY + Camera->y;
        event.flags = LOWORD(wParam);
        event.pressedButton = msg;
        userDispatch = !JGGetMouseHoverControl();
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        ReleaseCapture();
        event.id = JGEVENT_MOUSERELEASED;
        event.x = GET_X_LPARAM(lParam) * BufferStretchX + Camera->x;
        event.y = GET_Y_LPARAM(lParam) * BufferStretchY + Camera->y;
        event.flags = LOWORD(wParam);
        event.pressedButton = msg;
        userDispatch = !JGGetMouseFocusControl();
        break;
    case WM_MOUSEWHEEL:
        event.id = JGEVENT_MOUSEWHEEL;
        event.x = GET_X_LPARAM(lParam) * BufferStretchX + Camera->x;
        event.y = GET_Y_LPARAM(lParam) * BufferStretchY + Camera->y;
        event.flags = LOWORD(wParam);
        event.deltaWheel = GET_WHEEL_DELTA_WPARAM(wParam);
        userDispatch = !JGGetMouseHoverControl();
        break;
    case WM_MOUSEMOVE:
        event.id = JGEVENT_MOUSEMOVED;
        event.x = GET_X_LPARAM(lParam) * BufferStretchX + Camera->x;
        event.y = GET_Y_LPARAM(lParam) * BufferStretchY + Camera->y;
        event.dx = event.x - prevX;
        event.dy = event.y - prevY;
        prevX = event.x;
        prevY = event.y;
        event.flags = LOWORD(wParam);
        userDispatch = !JGGetMouseHoverControl();
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    if(JGDispatchEvent(event.id, &event) != JGCONSUME && userDispatch)
        JGEvent(&event);
    return 0;
}
