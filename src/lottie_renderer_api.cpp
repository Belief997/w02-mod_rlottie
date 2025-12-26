/*
 * Lottie Renderer C API 实现
 * 
 * 包装 rlottie::Animation 类为 C 语言接口
 */

#include "lottie_renderer.h"
#include "lottie/rlottie.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <memory>

/* 内部结构定义 */
struct LottieAnimation {
    std::unique_ptr<rlottie::Animation> animation;
};

/* ========== 加载函数 ========== */

LottieAnimationHandle lottie_animation_from_file(const char* path)
{
    if (!path || path[0] == '\0') {
        return nullptr;
    }
    
    auto animation = rlottie::Animation::loadFromFile(std::string(path), true);
    if (!animation) {
        return nullptr;
    }
    
    LottieAnimation* handle = new (std::nothrow) LottieAnimation();
    if (!handle) {
        return nullptr;
    }
    
    handle->animation = std::move(animation);
    return handle;
}

LottieAnimationHandle lottie_animation_from_data(
    const char* jsonData,
    size_t dataSize,
    const char* resourcePath)
{
    if (!jsonData || dataSize == 0) {
        return nullptr;
    }
    
    std::string data(jsonData, dataSize);
    std::string resPath = resourcePath ? resourcePath : "";
    std::string key = "data_" + std::to_string(reinterpret_cast<uintptr_t>(jsonData));
    
    auto animation = rlottie::Animation::loadFromData(
        std::move(data), key, resPath, false);
    
    if (!animation) {
        return nullptr;
    }
    
    LottieAnimation* handle = new (std::nothrow) LottieAnimation();
    if (!handle) {
        return nullptr;
    }
    
    handle->animation = std::move(animation);
    return handle;
}

/* ========== 信息查询 ========== */

int lottie_animation_get_info(
    LottieAnimationHandle handle,
    LottieAnimationInfo* info)
{
    if (!handle || !handle->animation || !info) {
        return LOTTIE_ERR_NULL;
    }
    
    info->frameRate = handle->animation->frameRate();
    info->totalFrames = handle->animation->totalFrame();
    info->duration = handle->animation->duration();
    
    size_t w = 0, h = 0;
    handle->animation->size(w, h);
    info->width = w;
    info->height = h;
    
    return LOTTIE_OK;
}

double lottie_animation_get_framerate(LottieAnimationHandle handle)
{
    if (!handle || !handle->animation) {
        return 0.0;
    }
    return handle->animation->frameRate();
}

size_t lottie_animation_get_totalframe(LottieAnimationHandle handle)
{
    if (!handle || !handle->animation) {
        return 0;
    }
    return handle->animation->totalFrame();
}

double lottie_animation_get_duration(LottieAnimationHandle handle)
{
    if (!handle || !handle->animation) {
        return 0.0;
    }
    return handle->animation->duration();
}

int lottie_animation_get_size(
    LottieAnimationHandle handle,
    size_t* width,
    size_t* height)
{
    if (!handle || !handle->animation) {
        return LOTTIE_ERR_NULL;
    }
    
    size_t w = 0, h = 0;
    handle->animation->size(w, h);
    
    if (width) *width = w;
    if (height) *height = h;
    
    return LOTTIE_OK;
}

/* ========== 渲染函数 ========== */

int lottie_animation_render(
    LottieAnimationHandle handle,
    size_t frameNo,
    LottieSurface* surface,
    int keepAspectRatio)
{
    if (!handle || !handle->animation) {
        return LOTTIE_ERR_NULL;
    }
    
    if (!surface || !surface->buffer) {
        return LOTTIE_ERR_NULL;
    }
    
    if (surface->width == 0 || surface->height == 0) {
        return LOTTIE_ERR_INVALID;
    }
    
    // 创建 rlottie Surface
    rlottie::Surface rlottieSurface(
        surface->buffer,
        surface->width,
        surface->height,
        surface->bytesPerLine
    );
    
    // 同步渲染
    handle->animation->renderSync(
        frameNo,
        rlottieSurface,
        keepAspectRatio != 0
    );
    
    return LOTTIE_OK;
}

size_t lottie_animation_frame_at_pos(
    LottieAnimationHandle handle,
    double pos)
{
    if (!handle || !handle->animation) {
        return 0;
    }
    return handle->animation->frameAtPos(pos);
}

/* ========== 序列化函数 ========== */

char* lottie_animation_to_json(LottieAnimationHandle handle)
{
    if (!handle || !handle->animation) {
        return nullptr;
    }
    
    // 获取动画信息
    LottieAnimationInfo info;
    if (lottie_animation_get_info(handle, &info) != LOTTIE_OK) {
        return nullptr;
    }
    
    // 构建简单的 JSON 表示
    char buffer[512];
    int len = snprintf(buffer, sizeof(buffer),
        "{\n"
        "  \"frameRate\": %.2f,\n"
        "  \"totalFrames\": %zu,\n"
        "  \"duration\": %.3f,\n"
        "  \"width\": %zu,\n"
        "  \"height\": %zu\n"
        "}",
        info.frameRate,
        info.totalFrames,
        info.duration,
        info.width,
        info.height
    );
    
    if (len < 0 || len >= (int)sizeof(buffer)) {
        return nullptr;
    }
    
    char* result = (char*)malloc(len + 1);
    if (!result) {
        return nullptr;
    }
    
    memcpy(result, buffer, len + 1);
    return result;
}

void lottie_free_string(char* str)
{
    if (str) {
        free(str);
    }
}

/* ========== 资源管理 ========== */

void lottie_animation_destroy(LottieAnimationHandle handle)
{
    if (handle) {
        delete handle;
    }
}

void lottie_configure_cache_size(size_t cacheSize)
{
    rlottie::configureModelCacheSize(cacheSize);
}
