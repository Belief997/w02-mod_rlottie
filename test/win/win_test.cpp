/*
 * Lottie Renderer Windows Test Program
 * 
 * Features:
 * 1. Load Lottie JSON file
 * 2. Render all frames and save as BMP files
 * 3. Optional: Play animation using GDI
 * 
 * Usage:
 *   lottie_test <input.json> [output_dir] [--play]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>

#include "lottie_renderer.h"
#include "bmp_writer.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)

/* High precision timer */
static double get_time_ms() {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000.0 / (double)freq.QuadPart;
}

#else
#include <sys/stat.h>
#include <sys/time.h>

static double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}
#endif

/* Print usage */
static void print_usage(const char* program) {
    printf("Lottie Renderer Test Program\n\n");
    printf("Usage: %s <input.json> [output_dir] [--play]\n\n", program);
    printf("Arguments:\n");
    printf("  input.json   Lottie JSON animation file\n");
    printf("  output_dir   Output directory (default: ./output)\n");
    printf("  --play       Play animation using GDI (Windows only)\n");
    printf("\nExamples:\n");
    printf("  %s animation.json\n", program);
    printf("  %s animation.json ./frames\n", program);
    printf("  %s animation.json --play\n", program);
}

/* Render all frames and save as BMP */
static int render_to_files(
    LottieAnimationHandle anim,
    const char* outputDir,
    size_t width,
    size_t height)
{
    LottieAnimationInfo info;
    if (lottie_animation_get_info(anim, &info) != LOTTIE_OK) {
        fprintf(stderr, "Error: Cannot get animation info\n");
        return -1;
    }
    
    printf("Animation Info:\n");
    printf("  Frame Rate: %.2f fps\n", info.frameRate);
    printf("  Total Frames: %zu\n", info.totalFrames);
    printf("  Duration: %.2f sec\n", info.duration);
    printf("  Original Size: %zu x %zu\n", info.width, info.height);
    printf("  Render Size: %zu x %zu\n", width, height);
    printf("\n");
    
    /* Create output directory */
    mkdir(outputDir, 0755);
    
    /* Allocate pixel buffer */
    size_t bufferSize = width * height * sizeof(uint32_t);
    uint32_t* buffer = (uint32_t*)malloc(bufferSize);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return -1;
    }
    
    /* Create Surface */
    LottieSurface surface;
    surface.buffer = buffer;
    surface.width = width;
    surface.height = height;
    surface.bytesPerLine = width * sizeof(uint32_t);
    
    printf("Rendering...\n");
    double startTime = get_time_ms();
    
    /* Render each frame */
    for (size_t frame = 0; frame < info.totalFrames; frame++) {
        /* Clear buffer */
        memset(buffer, 0, bufferSize);
        
        /* Render frame */
        if (lottie_animation_render(anim, frame, &surface, 1) != LOTTIE_OK) {
            fprintf(stderr, "Warning: Failed to render frame %zu\n", frame);
            continue;
        }
        
        /* Generate filename */
        char filename[512];
        snprintf(filename, sizeof(filename), "%s/frame_%04zu.bmp", outputDir, frame);
        
        /* Save as BMP */
        if (bmp_write(filename, buffer, (int)width, (int)height) != 0) {
            fprintf(stderr, "Warning: Failed to save frame %zu\n", frame);
            continue;
        }
        
        /* Progress display */
        if ((frame + 1) % 10 == 0 || frame == info.totalFrames - 1) {
            printf("\r  Progress: %zu / %zu (%.1f%%)", 
                   frame + 1, info.totalFrames,
                   (frame + 1) * 100.0 / info.totalFrames);
            fflush(stdout);
        }
    }
    
    double endTime = get_time_ms();
    double elapsed = endTime - startTime;
    
    printf("\n\nRendering Complete!\n");
    printf("  Total Frames: %zu\n", info.totalFrames);
    printf("  Time: %.2f sec\n", elapsed / 1000.0);
    printf("  Average: %.2f ms/frame\n", elapsed / info.totalFrames);
    printf("  Output Dir: %s\n", outputDir);
    
    free(buffer);
    return 0;
}

#ifdef _WIN32
/* GDI animation playback */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static struct {
    LottieAnimationHandle anim;
    uint32_t* buffer;
    uint32_t* displayBuffer;  /* Buffer with checkerboard background */
    size_t width;
    size_t height;
    size_t currentFrame;
    size_t totalFrames;
    double frameRate;
    BITMAPINFO bmi;
} g_player;

/* Generate checkerboard pattern for transparency display */
static uint32_t get_checker_color(int x, int y, int cellSize) {
    int cx = x / cellSize;
    int cy = y / cellSize;
    if ((cx + cy) % 2 == 0) {
        return 0xFFCCCCCC;  /* Light gray */
    } else {
        return 0xFF999999;  /* Dark gray */
    }
}

/* Blend premultiplied ARGB over checkerboard background */
static void blend_with_checkerboard(
    uint32_t* dst,
    const uint32_t* src,
    size_t width,
    size_t height,
    int cellSize)
{
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t idx = y * width + x;
            uint32_t pixel = src[idx];
            
            uint8_t a = (pixel >> 24) & 0xFF;
            
            if (a == 255) {
                /* Fully opaque - use pixel directly */
                dst[idx] = pixel;
            } else if (a == 0) {
                /* Fully transparent - show checkerboard */
                dst[idx] = get_checker_color((int)x, (int)y, cellSize);
            } else {
                /* Semi-transparent - blend with checkerboard */
                uint32_t bg = get_checker_color((int)x, (int)y, cellSize);
                
                /* Source is premultiplied, so: out = src + bg * (1 - srcA) */
                uint8_t srcR = (pixel >> 16) & 0xFF;
                uint8_t srcG = (pixel >> 8) & 0xFF;
                uint8_t srcB = pixel & 0xFF;
                
                uint8_t bgR = (bg >> 16) & 0xFF;
                uint8_t bgG = (bg >> 8) & 0xFF;
                uint8_t bgB = bg & 0xFF;
                
                uint8_t invA = 255 - a;
                uint8_t outR = srcR + (bgR * invA) / 255;
                uint8_t outG = srcG + (bgG * invA) / 255;
                uint8_t outB = srcB + (bgB * invA) / 255;
                
                dst[idx] = 0xFF000000 | (outR << 16) | (outG << 8) | outB;
            }
        }
    }
}

static int play_animation(LottieAnimationHandle anim, size_t width, size_t height) {
    LottieAnimationInfo info;
    if (lottie_animation_get_info(anim, &info) != LOTTIE_OK) {
        return -1;
    }
    
    g_player.anim = anim;
    g_player.width = width;
    g_player.height = height;
    g_player.currentFrame = 0;
    g_player.totalFrames = info.totalFrames;
    g_player.frameRate = info.frameRate;
    
    /* Allocate buffers */
    size_t bufSize = width * height * sizeof(uint32_t);
    g_player.buffer = (uint32_t*)malloc(bufSize);
    g_player.displayBuffer = (uint32_t*)malloc(bufSize);
    if (!g_player.buffer || !g_player.displayBuffer) {
        free(g_player.buffer);
        free(g_player.displayBuffer);
        return -1;
    }
    
    /* Setup BITMAPINFO */
    memset(&g_player.bmi, 0, sizeof(g_player.bmi));
    g_player.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    g_player.bmi.bmiHeader.biWidth = (LONG)width;
    g_player.bmi.bmiHeader.biHeight = -(LONG)height; /* Top-down */
    g_player.bmi.bmiHeader.biPlanes = 1;
    g_player.bmi.bmiHeader.biBitCount = 32;
    g_player.bmi.bmiHeader.biCompression = BI_RGB;
    
    /* Register window class */
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "LottiePlayer";
    RegisterClassEx(&wc);
    
    /* Calculate window size */
    RECT rect = {0, 0, (LONG)width, (LONG)height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    /* Create window */
    HWND hwnd = CreateWindowEx(
        0, "LottiePlayer", "Lottie Player",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!hwnd) {
        free(g_player.buffer);
        return -1;
    }
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    /* Set timer */
    UINT timerInterval = (UINT)(1000.0 / g_player.frameRate);
    SetTimer(hwnd, 1, timerInterval, NULL);
    
    /* Message loop */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    free(g_player.buffer);
    free(g_player.displayBuffer);
    return 0;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER: {
        /* Render current frame */
        LottieSurface surface;
        surface.buffer = g_player.buffer;
        surface.width = g_player.width;
        surface.height = g_player.height;
        surface.bytesPerLine = g_player.width * sizeof(uint32_t);
        
        /* Clear buffer to transparent */
        memset(g_player.buffer, 0, g_player.width * g_player.height * sizeof(uint32_t));
        lottie_animation_render(g_player.anim, g_player.currentFrame, &surface, 1);
        
        /* Blend with checkerboard background (8x8 pixel cells) */
        blend_with_checkerboard(
            g_player.displayBuffer,
            g_player.buffer,
            g_player.width,
            g_player.height,
            8
        );
        
        /* Update frame number */
        g_player.currentFrame = (g_player.currentFrame + 1) % g_player.totalFrames;
        
        /* Repaint */
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }
    
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        SetDIBitsToDevice(
            hdc, 0, 0,
            (DWORD)g_player.width, (DWORD)g_player.height,
            0, 0, 0, (UINT)g_player.height,
            g_player.displayBuffer, &g_player.bmi, DIB_RGB_COLORS
        );
        
        EndPaint(hwnd, &ps);
        break;
    }
    
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        break;
    
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;
    
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
#endif /* _WIN32 */

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* inputFile = argv[1];
    const char* outputDir = "output";
    int playMode = 0;
    
    /* Parse arguments */
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--play") == 0) {
            playMode = 1;
        } else if (argv[i][0] != '-') {
            outputDir = argv[i];
        }
    }
    
    printf("Loading animation: %s\n", inputFile);
    
    /* Load animation */
    LottieAnimationHandle anim = lottie_animation_from_file(inputFile);
    if (!anim) {
        fprintf(stderr, "Error: Cannot load animation file '%s'\n", inputFile);
        return 1;
    }
    
    /* Get animation size */
    size_t width = 0, height = 0;
    lottie_animation_get_size(anim, &width, &height);
    
    /* Limit max size */
    if (width > 1920) {
        height = height * 1920 / width;
        width = 1920;
    }
    if (height > 1080) {
        width = width * 1080 / height;
        height = 1080;
    }
    
    int result = 0;
    
#ifdef _WIN32
    if (playMode) {
        printf("Play mode (Press ESC to exit)\n");
        result = play_animation(anim, width, height);
    } else {
        result = render_to_files(anim, outputDir, width, height);
    }
#else
    if (playMode) {
        printf("Warning: Play mode only supported on Windows\n");
    }
    result = render_to_files(anim, outputDir, width, height);
#endif
    
    /* Test serialization */
    char* json = lottie_animation_to_json(anim);
    if (json) {
        printf("\nAnimation JSON Info:\n%s\n", json);
        lottie_free_string(json);
    }
    
    /* Free resources */
    lottie_animation_destroy(anim);
    
    return result;
}
