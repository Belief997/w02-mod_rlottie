/*
 * Lottie Renderer - Standalone Lottie Animation Rendering Module
 * 
 * Core rendering functionality extracted from rlottie:
 * - Parse Lottie JSON files
 * - Render frames to ARGB32 pixel data
 * - Cross-platform support (Windows, ARM)
 */

#ifndef LOTTIE_RENDERER_H
#define LOTTIE_RENDERER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
#define LOTTIE_OK           0   /* Success */
#define LOTTIE_ERR_NULL    -1   /* Null pointer argument */
#define LOTTIE_ERR_INVALID -2   /* Invalid argument */
#define LOTTIE_ERR_IO      -3   /* IO error */
#define LOTTIE_ERR_PARSE   -4   /* Parse error */

/* Opaque handle type */
typedef struct LottieAnimation* LottieAnimationHandle;

/* Surface structure - render target */
typedef struct {
    uint32_t* buffer;       /* ARGB32 pixel buffer (premultiplied) */
    size_t    width;        /* Width in pixels */
    size_t    height;       /* Height in pixels */
    size_t    bytesPerLine; /* Bytes per scanline */
} LottieSurface;

/* Animation info structure */
typedef struct {
    double frameRate;       /* Frame rate (fps) */
    size_t totalFrames;     /* Total frame count */
    double duration;        /* Duration in seconds */
    size_t width;           /* Default width */
    size_t height;          /* Default height */
} LottieAnimationInfo;

/* ========== Loading Functions ========== */

/**
 * Load animation from file path
 * @param path Lottie JSON file path
 * @return Animation handle, NULL on failure
 */
LottieAnimationHandle lottie_animation_from_file(const char* path);

/**
 * Load animation from JSON string
 * @param jsonData JSON string data
 * @param dataSize Data length
 * @param resourcePath Resource path for external images, can be NULL
 * @return Animation handle, NULL on failure
 */
LottieAnimationHandle lottie_animation_from_data(
    const char* jsonData,
    size_t dataSize,
    const char* resourcePath
);

/* ========== Info Query ========== */

/**
 * Get animation info
 * @param handle Animation handle
 * @param info Output animation info structure
 * @return LOTTIE_OK on success, error code otherwise
 */
int lottie_animation_get_info(
    LottieAnimationHandle handle,
    LottieAnimationInfo* info
);

/**
 * Get frame rate
 * @param handle Animation handle
 * @return Frame rate (fps), 0 on failure
 */
double lottie_animation_get_framerate(LottieAnimationHandle handle);

/**
 * Get total frame count
 * @param handle Animation handle
 * @return Total frames, 0 on failure
 */
size_t lottie_animation_get_totalframe(LottieAnimationHandle handle);

/**
 * Get animation duration
 * @param handle Animation handle
 * @return Duration in seconds, 0 on failure
 */
double lottie_animation_get_duration(LottieAnimationHandle handle);

/**
 * Get animation size
 * @param handle Animation handle
 * @param width Output width
 * @param height Output height
 * @return LOTTIE_OK on success, error code otherwise
 */
int lottie_animation_get_size(
    LottieAnimationHandle handle,
    size_t* width,
    size_t* height
);

/* ========== Rendering Functions ========== */

/**
 * Render frame synchronously
 * @param handle Animation handle
 * @param frameNo Frame index (0-based)
 * @param surface Render target surface
 * @param keepAspectRatio 0 = stretch fill, 1 = keep aspect ratio
 * @return LOTTIE_OK on success, error code otherwise
 */
int lottie_animation_render(
    LottieAnimationHandle handle,
    size_t frameNo,
    LottieSurface* surface,
    int keepAspectRatio
);

/**
 * Get frame number at position
 * @param handle Animation handle
 * @param pos Normalized position (0.0 ~ 1.0)
 * @return Corresponding frame number
 */
size_t lottie_animation_frame_at_pos(
    LottieAnimationHandle handle,
    double pos
);

/* ========== Serialization Functions ========== */

/**
 * Serialize animation data to JSON string
 * @param handle Animation handle
 * @return JSON string, call lottie_free_string to free, NULL on failure
 * @note Current version returns basic animation info as JSON
 */
char* lottie_animation_to_json(LottieAnimationHandle handle);

/**
 * Free serialized string
 * @param str String to free
 */
void lottie_free_string(char* str);

/* ========== Resource Management ========== */

/**
 * Destroy animation and free resources
 * @param handle Animation handle
 */
void lottie_animation_destroy(LottieAnimationHandle handle);

/**
 * Configure model cache size
 * @param cacheSize Cache size, 0 = disable cache
 */
void lottie_configure_cache_size(size_t cacheSize);

/* ========== Pixel Format Conversion ========== */

/**
 * Convert premultiplied ARGB32 to straight (non-premultiplied) ARGB32
 * 
 * The render output is premultiplied alpha format where RGB values are
 * pre-multiplied by alpha. This function converts to straight alpha format.
 * 
 * @param buffer Pixel buffer to convert (in-place)
 * @param width Image width
 * @param height Image height
 * 
 * Pixel format (little-endian memory layout):
 *   Input:  [B*A, G*A, R*A, A] (premultiplied)
 *   Output: [B, G, R, A] (straight)
 */
void lottie_convert_to_straight_alpha(uint32_t* buffer, size_t width, size_t height);

/**
 * Convert ARGB32 to RGBA32 format
 * 
 * @param buffer Pixel buffer to convert (in-place)
 * @param width Image width
 * @param height Image height
 * 
 * Pixel format conversion:
 *   Input:  0xAARRGGBB (ARGB)
 *   Output: 0xAABBGGRR (RGBA, for OpenGL/PNG)
 */
void lottie_convert_argb_to_rgba(uint32_t* buffer, size_t width, size_t height);

/**
 * Convert premultiplied ARGB32 to straight RGBA32
 * 
 * Combines unpremultiply and ARGB->RGBA conversion in one pass.
 * 
 * @param buffer Pixel buffer to convert (in-place)
 * @param width Image width
 * @param height Image height
 */
void lottie_convert_to_straight_rgba(uint32_t* buffer, size_t width, size_t height);

#ifdef __cplusplus
}
#endif

#endif /* LOTTIE_RENDERER_H */
