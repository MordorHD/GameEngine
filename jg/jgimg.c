#include "jg.h"
#include "stb_image.h"
#include <stdio.h>

#define BMP_FILE_HEADER_TYPE 0x424D

typedef struct {
    uint16_t bfType;       /* 2 bytes */
    uint32_t bfSize;       /* 4 bytes */
    uint16_t bfReserved1;  /* 2 bytes */
    uint16_t bfReserved2;  /* 2 bytes */
    uint32_t bfOffBits;    /* 4 bytes */
} __BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;          /* 4 bytes */
    int32_t  biWidth;         /* 4 bytes */
    int32_t  biHeight;        /* 4 bytes */
    uint16_t biPlanes;        /* 2 bytes */
    uint16_t biBitCount;      /* 2 bytes */
    uint32_t biCompression;   /* 4 bytes */
    uint32_t biSizeImage;     /* 4 bytes */
    int32_t  biXPixPerMeter;  /* 4 bytes */
    int32_t  biYPixPerMeter;  /* 4 bytes */
    uint32_t biClrUsed;       /* 4 bytes */
    uint32_t biClrImportant;  /* 4 bytes */
} __BITMAPINFOHEADER;

#define PNG_FILE_HEADER_TYPE 0x89504E47

#define PNG_CHK_IHDR 0x49484452
#define PNG_CHK_PLTE 0x504C5445
#define PNG_CHK_IDAT 0x49444154
#define PNG_CHK_IEND 0x49454E44

#define PNG_CHK_SRGB 0x73524742

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t bitDepth;
    uint8_t colorType;
    uint8_t compressionMethod;
    uint8_t filterMethod;
    uint8_t interlaceMethod;
} __PNGHEADER;

static inline JGIMAGE JGLoadBitmap(FILE *fp)
{
    __BITMAPFILEHEADER bmpFileHeader;
    __BITMAPINFOHEADER bmpInfoHeader;
    fread(&bmpFileHeader, 14, 1, fp);
    fread(&bmpInfoHeader, 40, 1, fp);

    color_t *pixels, *hFlippedPixels, *pPixels;
    int height;
    uint16_t byteCount;
    int totalSize;

    byteCount = bmpInfoHeader.biBitCount / 8;
    height = bmpInfoHeader.biHeight;
    totalSize = bmpInfoHeader.biWidth * bmpInfoHeader.biHeight;
    pixels = malloc(sizeof(color_t) * totalSize);
    hFlippedPixels = malloc(sizeof(color_t) * totalSize);

    while(totalSize--)
    {
        fread(hFlippedPixels, byteCount, 1, fp);
        hFlippedPixels++;
    }
    pPixels = pixels;
    while(height--)
    {
        hFlippedPixels -= bmpInfoHeader.biWidth;
        memcpy(pPixels, hFlippedPixels, sizeof(color_t) * bmpInfoHeader.biWidth);
        pPixels += bmpInfoHeader.biWidth;
    }
    free(hFlippedPixels);
    return JGCreateImage(bmpInfoHeader.biWidth, bmpInfoHeader.biHeight, pixels);
}

static int FlipIntBytes(int i)
{
    int j;
    int t = 0;
    for(j = 0; j < 4; j++)
    {
        t <<= 8;
        t |= i & 0xFF;
        i >>= 8;
    }
    return t;
}

static inline JGIMAGE JGLoadPNG(FILE *fp)
{
    uint32_t chunkSize;
    uint32_t chunkType;
    uint8_t *colorPalette = NULL;
    uint8_t *pixels = NULL;
    _Bool isCapital;
    char chunkName[5];
    int i;
    char tmp;
    __PNGHEADER header;
    printf("READING PNG FILE\n");
    fseek(fp, 8, SEEK_SET);
    while(1)
    {
        fread(&chunkSize, 1, 4, fp);
        chunkSize = FlipIntBytes(chunkSize);
        fread(chunkName, 1, 4, fp);
        chunkName[4] = 0;
        printf("\tCHUNK: SIZE[%d], NAME[%s]\n", chunkSize, chunkName);
        isCapital = isupper(chunkName[0]);
        for(i = 0; i < 4 / 2; i++)
        {
            tmp = chunkName[4 - 1 - i];
            chunkName[4 - 1 - i] = toupper(chunkName[i]);
            chunkName[i] = toupper(tmp);
        }
        chunkType = *(int*) chunkName;
        switch(chunkType)
        {
        case PNG_CHK_IHDR:
            fread(&header, chunkSize, 1, fp);
            header.width = FlipIntBytes(header.width);
            header.height = FlipIntBytes(header.height);
            printf("\t\tHEAD[%d, %d, %d, %d, %d, %d, %d]\n", header.width, header.height, header.bitDepth, header.colorType, header.compressionMethod, header.filterMethod, header.interlaceMethod);
            break;
        case PNG_CHK_IDAT:
            pixels = malloc(chunkSize);
            fread(pixels, 1, chunkSize, fp);
            break;
        case PNG_CHK_PLTE:
            colorPalette = malloc(chunkSize);
            fread(colorPalette, 1, chunkSize, fp);
            break;
        case PNG_CHK_IEND:
            return JGCreateImage(header.width, header.height, (color_t*) pixels);
        default:
            if(isCapital)
                goto error;
            fseek(fp, chunkSize, SEEK_CUR);
        }
        // skip CRC
        fseek(fp, 4, SEEK_CUR);
    }
    error:
        free(pixels);
        free(colorPalette);
        return NULL;
}

JGIMAGE JGLoadImage(const char *filePath)
{
    // FILE *fp = fopen(filePath, "rb");
    // if(!fp)
    //    return NULL;
    JGIMAGE image;
    // uint8_t header[10];
    int w, h;
    int c;
    uint8_t *pixelsIn, *basePixelsIn;
    color_t *pixelsOut, *basePixelsOut;
    color_t rgb;
    int comp;
    unsigned char b;
    // fread(header, 1, 10, fp);
    // fseek(fp, 0, SEEK_SET);
    // header[6] = 0;
    if((pixelsIn = stbi_load(filePath, &w, &h, &comp, 0)))
    {
        if(comp != 4)
            pixelsOut = basePixelsOut = malloc(4 * w * h);
        else
            pixelsOut = basePixelsOut = (color_t*) pixelsIn;
        c = w * h;
        basePixelsIn = pixelsIn;
        while(c--)
        {
            switch(comp)
            {
            case STBI_grey: rgb = 0xFF000000 | (*pixelsIn << 16) | (*pixelsIn << 8) | *pixelsIn;  break;
            case STBI_grey_alpha: rgb = (*(pixelsIn + 1) << 24) | (*pixelsIn << 16) | (*pixelsIn << 8) | *pixelsIn;  break;
            case STBI_rgb: rgb = 0xFF000000 | *((color_t*) pixelsIn); break;
            case STBI_rgb_alpha: rgb = *((color_t*) pixelsIn); break;
            }
            b = rgb & 0xFF;
            rgb = (rgb & 0xFFFFFF00) | ((rgb & 0x00FF0000) >> 16);
            rgb = (rgb & 0xFF00FFFF) | (b << 16);
            *(pixelsOut++) = rgb;
            pixelsIn += comp;
        }
        if(comp != 4)
            free(basePixelsIn);
        return JGCreateImage(w, h, basePixelsOut);
    }
    // fclose(fp);
    return NULL;
}
