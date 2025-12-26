/*
 * BMP File Writer Utility
 * 
 * Simple BMP file writer with no external dependencies
 * Supports 24-bit RGB format
 */

#ifndef BMP_WRITER_H
#define BMP_WRITER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

/* BMP file header */
typedef struct {
    uint16_t bfType;      /* File type "BM" */
    uint32_t bfSize;      /* File size */
    uint16_t bfReserved1; /* Reserved */
    uint16_t bfReserved2; /* Reserved */
    uint32_t bfOffBits;   /* Pixel data offset */
} BmpFileHeader;

/* BMP info header */
typedef struct {
    uint32_t biSize;          /* Info header size */
    int32_t  biWidth;         /* Image width */
    int32_t  biHeight;        /* Image height (positive=bottom-up) */
    uint16_t biPlanes;        /* Color planes */
    uint16_t biBitCount;      /* Bits per pixel */
    uint32_t biCompression;   /* Compression type */
    uint32_t biSizeImage;     /* Image data size */
    int32_t  biXPelsPerMeter; /* Horizontal resolution */
    int32_t  biYPelsPerMeter; /* Vertical resolution */
    uint32_t biClrUsed;       /* Colors used */
    uint32_t biClrImportant;  /* Important colors */
} BmpInfoHeader;

#pragma pack(pop)

/**
 * Save ARGB32 pixel data as BMP file
 * 
 * @param filename Output file path
 * @param buffer ARGB32 pixel data (premultiplied)
 * @param width Image width
 * @param height Image height
 * @return 0 on success, -1 on failure
 */
static inline int bmp_write(
    const char* filename,
    const uint32_t* buffer,
    int width,
    int height)
{
    if (!filename || !buffer || width <= 0 || height <= 0) {
        return -1;
    }
    
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        return -1;
    }
    
    /* Calculate row bytes (4-byte aligned) */
    int rowBytes = ((width * 3 + 3) / 4) * 4;
    int imageSize = rowBytes * height;
    int fileSize = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader) + imageSize;
    
    /* Fill file header */
    BmpFileHeader fileHeader;
    memset(&fileHeader, 0, sizeof(fileHeader));
    fileHeader.bfType = 0x4D42; /* "BM" */
    fileHeader.bfSize = fileSize;
    fileHeader.bfOffBits = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
    
    /* Fill info header */
    BmpInfoHeader infoHeader;
    memset(&infoHeader, 0, sizeof(infoHeader));
    infoHeader.biSize = sizeof(BmpInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height; /* Positive = bottom-up */
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0; /* BI_RGB */
    infoHeader.biSizeImage = imageSize;
    
    /* Write headers */
    fwrite(&fileHeader, sizeof(fileHeader), 1, fp);
    fwrite(&infoHeader, sizeof(infoHeader), 1, fp);
    
    /* Allocate row buffer */
    uint8_t* rowBuffer = (uint8_t*)malloc(rowBytes);
    if (!rowBuffer) {
        fclose(fp);
        return -1;
    }
    
    /* Write pixel data (bottom-up) */
    for (int y = height - 1; y >= 0; y--) {
        const uint32_t* srcRow = buffer + y * width;
        memset(rowBuffer, 0, rowBytes);
        
        for (int x = 0; x < width; x++) {
            uint32_t pixel = srcRow[x];
            
            /* ARGB32 -> BGR24 */
            /* Handle premultiplied alpha: unpremultiply if alpha > 0 */
            uint8_t a = (pixel >> 24) & 0xFF;
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;
            
            if (a > 0 && a < 255) {
                /* Unpremultiply */
                r = (uint8_t)((r * 255) / a);
                g = (uint8_t)((g * 255) / a);
                b = (uint8_t)((b * 255) / a);
            }
            
            rowBuffer[x * 3 + 0] = b;
            rowBuffer[x * 3 + 1] = g;
            rowBuffer[x * 3 + 2] = r;
        }
        
        fwrite(rowBuffer, rowBytes, 1, fp);
    }
    
    free(rowBuffer);
    fclose(fp);
    
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* BMP_WRITER_H */
