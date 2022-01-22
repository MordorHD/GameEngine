#include "jg.h"
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <windows.h>
#include <windowsx.h>

volatile bool running;
volatile int frameTimeMillis;
volatile double ftmOverflow;
volatile int FPS;
pthread_t thread_id;

HWND Window;
HDC BufferDc;
HDC WindowDc;
RECT RepaintRect = {
            .left    = INT_MAX,
            .top     = INT_MAX,
            .right   = INT_MIN,
            .bottom  = INT_MIN,
        };

LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

void JGInit(int argc, char **argv)
{
    WNDCLASS wc = {0};
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

    ShowWindow(Window, SW_NORMAL);
    UpdateWindow(Window);
}

int JGGetWindowWidth(void)
{
    RECT r;
    GetClientRect(Window, &r);
    return r.right;
}

int JGGetWindowHeight(void)
{
    RECT r;
    GetClientRect(Window, &r);
    return r.bottom;
}

static void UpdatePaintRect(int minX, int minY, int maxX, int maxY)
{
    RepaintRect = (RECT) {
         .left   = min(minX, RepaintRect.left),
         .top    = min(minY, RepaintRect.top),
         .right  = max(maxX, RepaintRect.right),
         .bottom = max(maxY, RepaintRect.bottom),
    };
}

void JGFill(color_t fill)
{
    SetDCBrushColor(BufferDc, fill);
}

void JGStroke(color_t stroke)
{
    SetDCPenColor(BufferDc, stroke);
}

void JGRect(int left, int top, int right, int bottom)
{
    UpdatePaintRect(left, top, right, bottom);
    Rectangle(BufferDc, left, top, right, bottom);
}

void JGOval(int left, int top, int right, int bottom)
{
    UpdatePaintRect(left, top, right, bottom);
    Ellipse(BufferDc, left, top, right, bottom);
}

void JGLine(int x1, int y1, int x2, int y2)
{
    UpdatePaintRect(x1, y1, x2, y2);
    MoveToEx(BufferDc, x1, y1, NULL);
    LineTo(BufferDc, x2, y2);
}

void JGSetFPSLimit(double limit)
{
    assert(limit >= 1.0d && "SetFPSLimit(limit), limit must be greater than 1.0");
    if(limit > 1000.0d)
        limit = 1000.0d;
    double ftmd = 1000.0d / limit;
    int ftm = frameTimeMillis = (int) ftmd;
    ftmOverflow = ftmd - ftm;
    FPS = (int) limit;
}

static void *Run(void *arg)
{
    struct timespec start, end;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);

    time_t runtime = 0;
    time_t diff = 1;
    double overflow = 0;
    int f1000 = 0;
    int fpsCount = 0;
    while(running)
    {
        JGStep(runtime, diff);
        JGDraw();

        if(RepaintRect.left < RepaintRect.right && RepaintRect.top < RepaintRect.bottom)
        {
            BitBlt(WindowDc, RepaintRect.left, RepaintRect.top,
                   RepaintRect.right - RepaintRect.left, RepaintRect.bottom - RepaintRect.left,
                   BufferDc, RepaintRect.left, RepaintRect.top, SRCCOPY);
           //printf("0x%I64x, %ld, %ld, %ld, %ld, 0x%I64x, %ld, %ld, 0x%x\n", WindowDc, RepaintRect.left, RepaintRect.top, RepaintRect.right - RepaintRect.left, RepaintRect.bottom - RepaintRect.left, BufferDc, RepaintRect.left, RepaintRect.top, SRCCOPY);
        }

        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
        diff = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        start.tv_sec = end.tv_sec;
        start.tv_nsec = end.tv_nsec;
        if(diff < frameTimeMillis)
        {
            diff = frameTimeMillis;
            if((overflow += ftmOverflow) > 0.0d)
            {
                int io = (int) overflow;
                overflow -= io;
                diff += io;
            }
            _sleep(diff);
        }
        runtime += diff;
        // only fps tracking
        fpsCount++;
        if((f1000 += diff) >= 1000)
        {
            FPS = fpsCount;
            f1000 -= 1000;
            fpsCount = 0;
        }
    }
    return NULL;
}

void JGStart(void)
{
    if(running)
        return;
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
{ running = 0; }

static inline void WinTranslateProc(JGEVENT *event, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_SETFOCUS:
        event->id = JGEVENT_ID_FOCUSGAINED;
        break;
    case WM_KILLFOCUS:
        event->id = JGEVENT_ID_FOCUSLOST;
        break;
    case WM_KEYDOWN:
        event->id = JGEVENT_ID_KEYPRESSED;
        event->vkCode = wParam;
        event->keyChar = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
        event->flags = lParam;
        break;
    case WM_KEYUP:
        event->id = JGEVENT_ID_KEYRELEASED;
        event->vkCode = wParam;
        event->keyChar = MapVirtualKey(wParam, MAPVK_VK_TO_CHAR);
        event->flags = lParam;
        break;
    case WM_CHAR:
        event->id = JGEVENT_ID_KEYTYPED;
        event->vkCode = wParam;
        event->keyChar = wParam;
        event->flags = lParam;
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
        event->id = JGEVENT_ID_MOUSEPRESSED;
        event->mousePos.x = GET_X_LPARAM(lParam);
        event->mousePos.y = GET_Y_LPARAM(lParam);
        event->flags = LOWORD(wParam);
        event->pressedButton = msg;
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        event->id = JGEVENT_ID_MOUSERELEASED;
        event->mousePos.x = GET_X_LPARAM(lParam);
        event->mousePos.y = GET_Y_LPARAM(lParam);
        event->flags = LOWORD(wParam);
        event->pressedButton = msg;
        break;
    case WM_MOUSEWHEEL:
        event->id = JGEVENT_ID_MOUSEWHEEL;
        event->mousePos.x = GET_X_LPARAM(lParam);
        event->mousePos.y = GET_Y_LPARAM(lParam);
        event->flags = LOWORD(wParam);
        event->deltaWheel = GET_WHEEL_DELTA_WPARAM(wParam);
        break;
    case WM_MOUSEMOVE:
        event->id = JGEVENT_ID_MOUSEMOVED;
        event->mousePos.x = GET_X_LPARAM(lParam);
        event->mousePos.y = GET_Y_LPARAM(lParam);
        event->flags = LOWORD(wParam);
        break;
    default:
        event->id = 0;
    }
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static JGEVENT event;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT *rp;
    WinTranslateProc(&event, msg, wParam, lParam);
    JGEvent(&event);
    switch(msg)
    {
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT:
    {
        rp = &ps.rcPaint;
        hdc = BeginPaint(hWnd, &ps);
        BitBlt(hdc, rp->left, rp->top, rp->right - rp->left, rp->bottom - rp->left, BufferDc, rp->left, rp->right, SRCCOPY);
        EndPaint(hWnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
