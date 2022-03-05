#include "jg.h"
#include <windows.h>

HCURSOR PredefinedCursors[9];
char LoadStates[9];

HCURSOR Cursor;

JGCURSOR JGGetCursor(void)
{
    return Cursor;
}

JGCURSOR JGCreateCursor(JGIMAGE image, int hotspotX, int hotspotY, int width, int height)
{
    ICONINFO ii;
    HBITMAP andOldHBmp, xorOldHBmp, imageOldHBmp;
    HBITMAP imageHBmp;
    BITMAP bmp;
    HBITMAP imageMask, andMask;
    int x, y;
    COLORREF pixel;
    HDC hdc, andDc, xorDc;
    HDC imageDc;
    hdc = GetDC(NULL);
    andDc = CreateCompatibleDC(hdc);
    andOldHBmp = SelectObject(andDc, andMask = CreateCompatibleBitmap(hdc, width, height));
    xorDc = CreateCompatibleDC(hdc);
    xorOldHBmp = SelectObject(xorDc, imageMask = CreateCompatibleBitmap(hdc, width, height));
    imageDc = CreateCompatibleDC(hdc);
    bmp.bmBits = image->pixels;
    bmp.bmWidth = width;
    bmp.bmHeight = height;
    bmp.bmPlanes = 1;
    bmp.bmBitsPixel = 32;
    imageHBmp = CreateBitmapIndirect(&bmp);
    imageOldHBmp = SelectObject(imageDc, imageHBmp);
    if(width != image->width || height != image->height)
        StretchBlt(xorDc, 0, 0, width, height, imageDc, 0, 0, image->width, image->height, SRCCOPY);
    else
        BitBlt(xorDc, 0, 0, width, height, imageDc, 0, 0, SRCCOPY);
    for(x = 0; x < width; x++)
    for(y = 0; y < height; y++)
    {
        pixel = GetPixel(xorDc, x, y);
        SetPixel(andDc, x, y, pixel ? 0 : 0xFFFFFF);
    }
    SelectObject(andDc, andOldHBmp);
    SelectObject(xorDc, xorOldHBmp);
    SelectObject(imageDc, imageOldHBmp);
    DeleteDC(andDc);
    DeleteDC(xorDc);
    DeleteDC(imageDc);
    DeleteObject(imageHBmp);
    ReleaseDC(NULL, hdc);
    ii.fIcon = FALSE;
    ii.xHotspot = hotspotX;
    ii.yHotspot = hotspotY;
    ii.hbmMask = andMask;
    ii.hbmColor = imageMask;
    return CreateIconIndirect(&ii);
}

JGCURSOR JGLoadCursor(cursor_t type)
{
    if(!LoadStates[type])
    {
        switch(type)
        {
        case JGCURSOR_DEFAULT:
        {
            BYTE andPlane[] = {
                0b00000000, 0b01111111,
                0b00000000, 0b00111111,
                0b00000000, 0b00111111,
                0b00000000, 0b00111111,
                0b00000000, 0b01111111,
                0b00000000, 0b01111111,
                0b00000000, 0b00111111,
                0b00000000, 0b00011111,
                0b00000000, 0b00001111,
                0b10001100, 0b00000111,
                0b11111110, 0b00000011,
                0b11111111, 0b00000001,
                0b11111111, 0b10000000,
                0b11111111, 0b11000000,
                0b11111111, 0b11100000,
                0b11111111, 0b11110000,
            };
            BYTE xorPlane[] = {
                0b11111111, 0b10000000,
                0b10000000, 0b01000000,
                0b10000000, 0b01000000,
                0b10000000, 0b01000000,
                0b10000000, 0b10000000,
                0b10000000, 0b10000000,
                0b10000000, 0b01000000,
                0b10000000, 0b00100000,
                0b10001100, 0b00010000,
                0b01110010, 0b00001000,
                0b00000001, 0b00000100,
                0b00000000, 0b10000010,
                0b00000000, 0b01000001,
                0b00000000, 0b00100001,
                0b00000000, 0b00010001,
                0b00000000, 0b00001111,
            };
            PredefinedCursors[type] = CreateCursor(NULL, 0, 0, 16, 16, andPlane, xorPlane);
            break;
        }
        case JGCURSOR_LR:
        {
            BYTE andPlane[] = {
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11110111, 0b11101111,
                0b11100011, 0b11000111,
                0b11000011, 0b11000011,
                0b10000000, 0b00000001,
                0b00000000, 0b00000000,
                0b00000000, 0b00000000,
                0b10000000, 0b00000001,
                0b11000011, 0b11000011,
                0b11100011, 0b11000111,
                0b11110111, 0b11101111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
            };
            BYTE xorPlane[] = {
                0b00000000, 0b00000000,
                0b00000000, 0b00000000,
                0b00000000, 0b00000000,
                0b00001000, 0b00010000,
                0b00010100, 0b00101000,
                0b00100100, 0b00100100,
                0b01000011, 0b11000010,
                0b10000000, 0b00000001,
                0b10000000, 0b00000001,
                0b01000011, 0b11000010,
                0b00100100, 0b00100100,
                0b00010100, 0b00101000,
                0b00001000, 0b00010000,
                0b00000000, 0b00000000,
                0b00000000, 0b00000000,
                0b00000000, 0b00000000,
            };
            PredefinedCursors[type] = CreateCursor(NULL, 8, 8, 16, 16, andPlane, xorPlane);
            break;
        }
        case JGCURSOR_UD:
        {
            BYTE andPlane[] = {
                0b11111110, 0b01111111,
                0b11111100, 0b00111111,
                0b11111000, 0b00011111,
                0b11110000, 0b00001111,
                0b11100000, 0b00000111,
                0b11110000, 0b00001111,
                0b11111100, 0b00111111,
                0b11111100, 0b00111111,
                0b11111100, 0b00111111,
                0b11111100, 0b00111111,
                0b11110000, 0b00001111,
                0b11100000, 0b00000111,
                0b11110000, 0b00001111,
                0b11111000, 0b00011111,
                0b11111100, 0b00111111,
                0b11111110, 0b01111111,
            };
            BYTE xorPlane[] = {
                0b00000001, 0b10000000,
                0b00000010, 0b01000000,
                0b00000100, 0b00100000,
                0b00001000, 0b00010000,
                0b00010000, 0b00001000,
                0b00001100, 0b00110000,
                0b00000010, 0b01000000,
                0b00000010, 0b01000000,
                0b00000010, 0b01000000,
                0b00000010, 0b01000000,
                0b00001100, 0b00110000,
                0b00010000, 0b00001000,
                0b00001000, 0b00010000,
                0b00000100, 0b00100000,
                0b00000010, 0b01000000,
                0b00000001, 0b10000000,
            };
            PredefinedCursors[type] = CreateCursor(NULL, 8, 8, 16, 16, andPlane, xorPlane);
            break;
        }
        case JGCURSOR_LU_RD:
        {
            BYTE andPlane[] = {
                0b00000011, 0b11111111,
                0b00000001, 0b11111111,
                0b00000001, 0b11111111,
                0b00000001, 0b11111111,
                0b00000000, 0b11111111,
                0b00000000, 0b01111111,
                0b10000000, 0b00111111,
                0b11110000, 0b00011111,
                0b11111000, 0b00001111,
                0b11111100, 0b00000001,
                0b11111110, 0b00000000,
                0b11111111, 0b00000000,
                0b11111111, 0b10000000,
                0b11111111, 0b10000000,
                0b11111111, 0b10000000,
                0b11111111, 0b11000000,
            };
            BYTE xorPlane[] = {
                0b11111100, 0b00000000,
                0b10000010, 0b00000000,
                0b10000010, 0b00000000,
                0b10000010, 0b00000000,
                0b10000001, 0b00000000,
                0b10000000, 0b10000000,
                0b01110000, 0b01000000,
                0b00001000, 0b00100000,
                0b00000100, 0b00010000,
                0b00000010, 0b00001110,
                0b00000001, 0b00000001,
                0b00000000, 0b10000001,
                0b00000000, 0b01000001,
                0b00000000, 0b01000001,
                0b00000000, 0b01000001,
                0b00000000, 0b00111111,
            };
            PredefinedCursors[type] = CreateCursor(NULL, 8, 8, 16, 16, andPlane, xorPlane);
            break;
        }
        case JGCURSOR_LD_RU:
        {
            BYTE andPlane[] = {
                0b11111111, 0b11000000,
                0b11111111, 0b10000000,
                0b11111111, 0b10000000,
                0b11111111, 0b10000000,
                0b11111111, 0b00000000,
                0b11111110, 0b00000000,
                0b11111100, 0b00000001,
                0b11111000, 0b00001111,
                0b11110000, 0b00011111,
                0b10000000, 0b00111111,
                0b00000000, 0b01111111,
                0b00000000, 0b11111111,
                0b00000001, 0b11111111,
                0b00000001, 0b11111111,
                0b00000001, 0b11111111,
                0b00000011, 0b11111111,
            };
            BYTE xorPlane[] = {
                0b00000000, 0b00111111,
                0b00000000, 0b01000001,
                0b00000000, 0b01000001,
                0b00000000, 0b01000001,
                0b00000000, 0b10000001,
                0b00000001, 0b00000001,
                0b00000010, 0b00001110,
                0b00000100, 0b00010000,
                0b00001000, 0b00100000,
                0b01110000, 0b01000000,
                0b10000000, 0b10000000,
                0b10000001, 0b00000000,
                0b10000010, 0b00000000,
                0b10000010, 0b00000000,
                0b10000010, 0b00000000,
                0b11111100, 0b00000000,
            };
            PredefinedCursors[type] = CreateCursor(NULL, 8, 8, 16, 16, andPlane, xorPlane);
            break;
        }
        case JGCURSOR_MOVE:
        {
            BYTE andPlane[] = {
                0b11111110, 0b01111111,
                0b11111100, 0b00111111,
                0b11111000, 0b00011111,
                0b11111100, 0b00111111,
                0b11111100, 0b00111111,
                0b11011100, 0b00011011,
                0b10000000, 0b00000001,
                0b00000000, 0b00000000,
                0b00000000, 0b00000000,
                0b10000000, 0b00000001,
                0b11011100, 0b00111011,
                0b11111100, 0b00111111,
                0b11111100, 0b00111111,
                0b11111000, 0b00011111,
                0b11111100, 0b00111111,
                0b11111110, 0b01111111,
            };
            BYTE xorPlane[] = {
                0b00000001, 0b10000000,
                0b00000010, 0b01000000,
                0b00000100, 0b00100000,
                0b00000010, 0b01000000,
                0b00000010, 0b01000000,
                0b00100010, 0b00100100,
                0b01011100, 0b00011010,
                0b10000000, 0b00000001,
                0b10000000, 0b00000001,
                0b01011100, 0b00111010,
                0b00100010, 0b01000100,
                0b00000010, 0b01000000,
                0b00000010, 0b01000000,
                0b00000100, 0b00100000,
                0b00000010, 0b01000000,
                0b00000001, 0b10000000,
            };
            PredefinedCursors[type] = CreateCursor(NULL, 8, 8, 16, 16, andPlane, xorPlane);
            break;
        }
        case JGCURSOR_HAND:
        {
            BYTE andPlane[] = {
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111110, 0b11111111,
                0b11111100, 0b01111111,
                0b11111000, 0b00111111,
                0b11100000, 0b00011111,
                0b11000000, 0b00011111,
                0b11000000, 0b00011111,
                0b11000000, 0b00011111,
                0b11000000, 0b00011001,
                0b10000000, 0b00010001,
                0b10000000, 0b00000001,
                0b11000000, 0b00000011,
                0b11000000, 0b00000011,
                0b11100000, 0b00000111,
                0b11110000, 0b00001111,
            };
            BYTE xorPlane[] = {
                0b00000000, 0b00000000,
                0b00000000, 0b00000000,
                0b00000001, 0b00000000,
                0b00000010, 0b10000000,
                0b00000110, 0b11000000,
                0b00011010, 0b10100000,
                0b00101010, 0b10100000,
                0b00101010, 0b10100000,
                0b00101010, 0b10100000,
                0b00101010, 0b10100110,
                0b01001000, 0b00101010,
                0b01000000, 0b00010010,
                0b00100000, 0b00000100,
                0b00100000, 0b00000100,
                0b00010000, 0b00001000,
                0b00001111, 0b11110000,
            };
            PredefinedCursors[type] = CreateCursor(NULL, 8, 8, 16, 16, andPlane, xorPlane);
            break;
        }
        case JGCURSOR_CARET:
        {
            BYTE andPlane[] = {
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
            };
            BYTE xorPlane[] = {
                0b00001110, 0b01110000,
                0b00011111, 0b11111000,
                0b00011001, 0b10011000,
                0b00000001, 0b10000000,
                0b00000001, 0b10000000,
                0b00000001, 0b10000000,
                0b00000001, 0b10000000,
                0b00000001, 0b10000000,
                0b00000001, 0b10000000,
                0b00000001, 0b10000000,
                0b00000001, 0b10000000,
                0b00000001, 0b10000000,
                0b00000001, 0b10000000,
                0b00011001, 0b10011000,
                0b00011111, 0b11111000,
                0b00001110, 0b01110000,
            };
            PredefinedCursors[type] = CreateCursor(NULL, 8, 8, 16, 16, andPlane, xorPlane);
            break;
        }
        case JGCURSOR_ROTATE:
        {
            BYTE andPlane[] = {
                0b11111000, 0b00011111,
                0b11100111, 0b11100111,
                0b11011111, 0b11111011,
                0b10111000, 0b01111011,
                0b10110111, 0b10011101,
                0b01101111, 0b11101101,
                0b01101111, 0b11101101,
                0b01011111, 0b11011110,
                0b01011111, 0b11101101,
                0b01101111, 0b11110011,
                0b01101111, 0b11111111,
                0b10110111, 0b11111111,
                0b11011011, 0b11111111,
                0b11100111, 0b11111111,
                0b11111111, 0b11111111,
                0b11111111, 0b11111111,
            };
            BYTE xorPlane[] = {
                0b00000111, 0b11100000,
                0b00011111, 0b11111000,
                0b00111111, 0b11111100,
                0b01111111, 0b11111100,
                0b01111000, 0b01111110,
                0b11110000, 0b00011110,
                0b11110000, 0b00011110,
                0b11100000, 0b00111111,
                0b11100000, 0b00011110,
                0b11110000, 0b00001100,
                0b11110000, 0b00000000,
                0b01111000, 0b00000000,
                0b00111100, 0b00000000,
                0b00011000, 0b00000000,
                0b00000000, 0b00000000,
                0b00000000, 0b00000000,
            };
            PredefinedCursors[type] = CreateCursor(NULL, 8, 8, 16, 16, andPlane, xorPlane);
            break;
        }
        }
        LoadStates[type] = 1;
    }
    return PredefinedCursors[type];
}

void JGSetCursor(JGCURSOR cursor)
{
    Cursor = cursor;
}
