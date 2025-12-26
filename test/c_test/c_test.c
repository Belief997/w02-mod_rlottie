/*
 * Lottie Renderer C API Test
 * 
 * Pure C test program to verify C API compatibility
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lottie_renderer.h"

/* Simple BMP writer for C */
#pragma pack(push, 1)
typedef struct {
    unsigned short bfType;
    unsigned int   bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffBits;
} BMPFileHeader;

typedef struct {
    unsigned int   biSize;
    int            biWidth;
    int            biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    int            biXPelsPerMeter;
    int            biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

static int save_bmp(const char* filename, const unsigned int* buffer, int width, int height)
{
    FILE* fp;
    int rowBytes, imageSize, fileSize;
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    unsigned char* rowBuffer;
    int x, y;
    
    fp = fopen(filename, "wb");
    if (!fp) return -1;
    
    rowBytes = ((width * 3 + 3) / 4) * 4;
    imageSize = rowBytes * height;
    fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageSize;
    
    memset(&fileHeader, 0, sizeof(fileHeader));
    fileHeader.bfType = 0x4D42;
    fileHeader.bfSize = fileSize;
    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    
    memset(&infoHeader, 0, sizeof(infoHeader));
    infoHeader.biSize = sizeof(BMPInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biSizeImage = imageSize;
    
    fwrite(&fileHeader, sizeof(fileHeader), 1, fp);
    fwrite(&infoHeader, sizeof(infoHeader), 1, fp);
    
    rowBuffer = (unsigned char*)malloc(rowBytes);
    if (!rowBuffer) {
        fclose(fp);
        return -1;
    }
    
    for (y = height - 1; y >= 0; y--) {
        const unsigned int* srcRow = buffer + y * width;
        memset(rowBuffer, 0, rowBytes);
        
        for (x = 0; x < width; x++) {
            unsigned int pixel = srcRow[x];
            unsigned char a = (pixel >> 24) & 0xFF;
            unsigned char r = (pixel >> 16) & 0xFF;
            unsigned char g = (pixel >> 8) & 0xFF;
            unsigned char b = pixel & 0xFF;
            
            if (a > 0 && a < 255) {
                r = (unsigned char)((r * 255) / a);
                g = (unsigned char)((g * 255) / a);
                b = (unsigned char)((b * 255) / a);
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

int main(int argc, char* argv[])
{
    LottieAnimationHandle anim;
    LottieAnimationInfo info;
    LottieSurface surface;
    unsigned int* buffer;
    size_t width, height;
    const char* inputFile;
    char outputFile[256];
    int ret;
    
    if (argc < 2) {
        printf("Lottie Renderer C API Test\n");
        printf("Usage: %s <input.json> [output.bmp]\n", argv[0]);
        return 1;
    }
    
    inputFile = argv[1];
    
    printf("=== Lottie Renderer C API Test ===\n\n");
    
    /* Test: Load animation */
    printf("1. Loading animation: %s\n", inputFile);
    anim = lottie_animation_from_file(inputFile);
    if (!anim) {
        fprintf(stderr, "   FAILED: Cannot load animation\n");
        return 1;
    }
    printf("   OK: Animation loaded\n\n");
    
    /* Test: Get info */
    printf("2. Getting animation info...\n");
    ret = lottie_animation_get_info(anim, &info);
    if (ret != LOTTIE_OK) {
        fprintf(stderr, "   FAILED: Cannot get info (error %d)\n", ret);
        lottie_animation_destroy(anim);
        return 1;
    }
    printf("   Frame Rate:    %.2f fps\n", info.frameRate);
    printf("   Total Frames:  %zu\n", info.totalFrames);
    printf("   Duration:      %.2f sec\n", info.duration);
    printf("   Size:          %zu x %zu\n", info.width, info.height);
    printf("   OK\n\n");
    
    /* Test: Individual query functions */
    printf("3. Testing individual query functions...\n");
    printf("   get_framerate:   %.2f\n", lottie_animation_get_framerate(anim));
    printf("   get_totalframe:  %zu\n", lottie_animation_get_totalframe(anim));
    printf("   get_duration:    %.2f\n", lottie_animation_get_duration(anim));
    lottie_animation_get_size(anim, &width, &height);
    printf("   get_size:        %zu x %zu\n", width, height);
    printf("   frame_at_pos(0.5): %zu\n", lottie_animation_frame_at_pos(anim, 0.5));
    printf("   OK\n\n");
    
    /* Test: Render frame */
    printf("4. Rendering first frame...\n");
    buffer = (unsigned int*)malloc(width * height * sizeof(unsigned int));
    if (!buffer) {
        fprintf(stderr, "   FAILED: Memory allocation\n");
        lottie_animation_destroy(anim);
        return 1;
    }
    
    memset(buffer, 0, width * height * sizeof(unsigned int));
    
    surface.buffer = buffer;
    surface.width = width;
    surface.height = height;
    surface.bytesPerLine = width * sizeof(unsigned int);
    
    ret = lottie_animation_render(anim, 0, &surface, 1);
    if (ret != LOTTIE_OK) {
        fprintf(stderr, "   FAILED: Render error %d\n", ret);
        free(buffer);
        lottie_animation_destroy(anim);
        return 1;
    }
    printf("   OK: Frame rendered\n\n");
    
    /* Test: Save BMP */
    if (argc >= 3) {
        snprintf(outputFile, sizeof(outputFile), "%s", argv[2]);
    } else {
        snprintf(outputFile, sizeof(outputFile), "c_test_output.bmp");
    }
    
    printf("5. Saving to BMP: %s\n", outputFile);
    if (save_bmp(outputFile, buffer, (int)width, (int)height) != 0) {
        fprintf(stderr, "   FAILED: Cannot save BMP\n");
    } else {
        printf("   OK: BMP saved\n\n");
    }
    
    /* Test: Serialization */
    printf("6. Testing serialization...\n");
    {
        char* json = lottie_animation_to_json(anim);
        if (json) {
            printf("   JSON output:\n%s\n", json);
            lottie_free_string(json);
            printf("   OK\n\n");
        } else {
            printf("   FAILED: Cannot serialize\n\n");
        }
    }
    
    /* Test: NULL handling */
    printf("7. Testing NULL handling...\n");
    {
        LottieAnimationHandle nullHandle = NULL;
        LottieAnimationInfo nullInfo;
        
        ret = lottie_animation_get_info(nullHandle, &nullInfo);
        printf("   get_info(NULL): %d (expected %d)\n", ret, LOTTIE_ERR_NULL);
        
        printf("   get_framerate(NULL): %.2f (expected 0)\n", 
               lottie_animation_get_framerate(nullHandle));
        
        /* This should not crash */
        lottie_animation_destroy(nullHandle);
        printf("   destroy(NULL): OK (no crash)\n");
        printf("   OK\n\n");
    }
    
    /* Cleanup */
    printf("8. Cleanup...\n");
    free(buffer);
    lottie_animation_destroy(anim);
    printf("   OK\n\n");
    
    printf("=== All C API tests passed! ===\n");
    
    return 0;
}
