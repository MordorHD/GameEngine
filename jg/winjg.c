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

static volatile JGCAMERA *Camera;

static JGIMAGE ScreenImage;

JGIMAGE JGGetScreenImage(void)
{ return ScreenImage; }

void JGGetWindowSize(JGSIZE *size)
{
    RECT r;
    GetClientRect(Window, &r);
    size->width = r.right;
    size->height = r.bottom;
}

void JGGetScreenSize(JGSIZE *size)
{
    size->width = ScreenImage->bmp.bmWidth;
    size->height = ScreenImage->bmp.bmHeight;
}

static LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
static void *Run(void*);

void JGInit(int argc, char **argv)
{
    WNDCLASS wc = {0};
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_DBLCLKS;
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
    wc.lpszClassName = "WC_MAIN_WINDOW";
    wc.lpfnWndProc = MainWndProc;
    RegisterClass(&wc);

    Window = CreateWindow("WC_MAIN_WINDOW", "Title", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);

    WindowDc = GetDC(Window);
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    HMONITOR hmon = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
    GetMonitorInfo(hmon, &mi);
    BufferDc = CreateCompatibleDC(WindowDc);
    HBITMAP buffer = CreateCompatibleBitmap(WindowDc, mi.rcMonitor.right, mi.rcMonitor.bottom);
    SelectObject(BufferDc, buffer);
    SelectObject(BufferDc, GetStockObject(DC_BRUSH));
    SelectObject(BufferDc, GetStockObject(DC_PEN));
    ScreenImage = malloc(sizeof(*ScreenImage));
    ScreenImage->imageDc = BufferDc;
    ScreenImage->hbmp = buffer;
    ScreenImage->bmp.bmWidth  = mi.rcMonitor.right;
    ScreenImage->bmp.bmHeight = mi.rcMonitor.bottom;
    SetBkMode(BufferDc, TRANSPARENT);
    SetTextAlign(BufferDc, TA_CENTER | TA_BASELINE);

    memset((void*) (Camera = malloc(sizeof(*Camera))), 0, sizeof(*Camera));

    SetGraphicsMode(WindowDc, GM_ADVANCED);
    SetGraphicsMode(BufferDc, GM_ADVANCED);

    ShowWindow(Window, SW_NORMAL);
    UpdateWindow(Window);
}

void JGCameraTranslate(float x, float y)
{
    Camera->x += x;
    Camera->y += y;
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
    Camera->cFlags &= JGCA_BOTTOM | JGCA_TOP | JGCA_RIGHT | JGCA_VCENTER | JGCA_HCENTER | JGCA_LEFT;
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

JGIMAGE JGCreateImage(int width, int height, int *pixels)
{
    JGIMAGE img = malloc(sizeof(*img));
    BITMAPINFO bmi;
    HDC hdc = GetDC(Window);

    BITMAP bmp;
    bmp.bmType = 0;
    bmp.bmWidth = width;
    bmp.bmHeight = height;
    bmp.bmWidthBytes = width * 4;
    bmp.bmPlanes = 1;
    bmp.bmBitsPixel = 32;
    bmp.bmBits = pixels;

    img->imageDc = CreateCompatibleDC(hdc);
    img->hbmp = CreateBitmapIndirect(&bmp);
    img->bmp = bmp;
    SelectObject(img->imageDc, img->hbmp);

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

void JGClipRect(const RECT *rect)
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

void JGBkColor(color_t bkColor)
{
    SetBkColor(BufferDc, bkColor);
}

void JGBkMode(int mode)
{
    SetBkMode(BufferDc, mode);
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

XFORM Transform;

void JGResetTransform(void)
{
    Transform.eM11 = 1.0f;
    Transform.eM12 = 0.0f;
    Transform.eM21 = 0.0f;
    Transform.eM22 = 1.0f;
    Transform.eDx  = 0.0f;
    Transform.eDy  = 0.0f;
    ModifyWorldTransform(BufferDc, NULL, MWT_IDENTITY);
}

void JGTranslate(float x, float y)
{
    Transform.eDx += x * Transform.eM11 + y * Transform.eM12;
    Transform.eDy += x * Transform.eM21 + y * Transform.eM22;
    SetWorldTransform(BufferDc, &Transform);
}

void JGScale(float x, float y)
{
    Transform.eM11 *= x;
    Transform.eM12 *= x;
    Transform.eM21 *= y;
    Transform.eM22 *= y;
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
    JGPOINT2D *target;
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
            tx = target->x;
            ty = target->y;
            if(fabs(x - tx) >= 0.1d || fabs(y - ty) >= 0.1d)
            {
                x_align = (cFlags & JGCA_HCENTER) ? (wr.right  >> 1) : (cFlags & JGCA_RIGHT)  ? wr.right  : 0;
                y_align = (cFlags & JGCA_VCENTER) ? (wr.bottom >> 1) : (cFlags & JGCA_BOTTOM) ? wr.bottom : 0;
                l = Camera->cstr_l;
                r = Camera->cstr_r;
                t = Camera->cstr_t;
                b = Camera->cstr_b;
                drl = r - l;
                dbt = b - t;
                tx = drl <= wr.right ? 0
                        : -tx - x_align >= l - wr.right ? -l
                        : x_align + tx > r ? r - wr.right
                        : tx - x_align;
                ty = dbt <= wr.bottom ? 0
                        : -ty - y_align >= t - wr.bottom ? -t
                        : y_align + ty > b ? b - wr.bottom
                        : ty - y_align;
                Camera->x += (tx - x) * 0.1f;
                Camera->y += (ty - y) * 0.1f;
            }
        }
        // user draw
        JGDraw();
        // draw controls
        JGDispatchEvent(JGEVENT_REDRAW, NULL);
        // copy buffer to window
        BitBlt(WindowDc, 0, 0, wr.right, wr.bottom, BufferDc, 0, 0, SRCCOPY);
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
    switch(msg)
    {
    case WM_CREATE: JGSetCursor(JGCURSOR_DEFAULT); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_ERASEBKGND: return 1;
    case WM_SIZE:
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
        break;
    case WM_KEYUP:
        event.id = JGEVENT_KEYRELEASED;
        event.vkCode = wParam;
        event.keyChar = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
        event.flags = lParam;
        break;
    case WM_CHAR:
        event.id = JGEVENT_KEYTYPED;
        event.vkCode = wParam;
        event.keyChar = wParam;
        event.flags = lParam;
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
        SetCapture(hWnd);
        event.id = JGEVENT_MOUSEPRESSED;
        event.x = GET_X_LPARAM(lParam);
        event.y = GET_Y_LPARAM(lParam);
        event.flags = LOWORD(wParam);
        event.pressedButton = msg;
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        ReleaseCapture();
        event.id = JGEVENT_MOUSERELEASED;
        event.x = GET_X_LPARAM(lParam);
        event.y = GET_Y_LPARAM(lParam);
        event.flags = LOWORD(wParam);
        event.pressedButton = msg;
        break;
    case WM_MOUSEWHEEL:
        event.id = JGEVENT_MOUSEWHEEL;
        event.x = GET_X_LPARAM(lParam);
        event.y = GET_Y_LPARAM(lParam);
        event.flags = LOWORD(wParam);
        event.deltaWheel = GET_WHEEL_DELTA_WPARAM(wParam);
        break;
    case WM_MOUSEMOVE:
        event.id = JGEVENT_MOUSEMOVED;
        event.x = GET_X_LPARAM(lParam);
        event.y = GET_Y_LPARAM(lParam);
        event.dx = event.x - prevX;
        event.dy = event.y - prevY;
        prevX = event.x;
        prevY = event.y;
        event.flags = LOWORD(wParam);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    if(JGDispatchEvent(event.id, &event) != JGCONSUME && !JGGetMouseOverControl())
        JGEvent(&event);
    return 0;
}
