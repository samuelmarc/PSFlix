#pragma once

#include <pngdec/pngdec.h>
#include <sysmodule/sysmodule.h>
#include <sysutil/sysutil.h>
#include <tiny3d.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace cellflix {

class BackgroundRenderer final {
public:
    BackgroundRenderer() = default;
    BackgroundRenderer(const BackgroundRenderer&) = delete;
    BackgroundRenderer& operator=(const BackgroundRenderer&) = delete;

    ~BackgroundRenderer() {
        if (initialized_) {
            // Wait for pending RSX work before terminating the graphics context.
            tiny3d_Exit();
            sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT3);
        }
    }

    bool initialize() {
        if (tiny3d_Init(1024 * 1024) != TINY3D_OK) {
            return false;
        }
        initialized_ = true;
        tiny3d_UserViewport(1, 0.0f, 0.0f,
            static_cast<float>(Video_Resolution.width) / canvasWidth,
            static_cast<float>(Video_Resolution.height) / canvasHeight,
            static_cast<float>(Video_Resolution.width) / 1920.0f,
            static_cast<float>(Video_Resolution.height) / 1080.0f);
        return true;
    }

    bool loadBackground(const char* path) {
        if (!initialized_ || textureMemory_) {
            return false;
        }

        const s32 moduleResult = sysModuleLoad(SYSMODULE_PNGDEC);
        if (moduleResult != SYSMODULE_OK &&
            static_cast<u32>(moduleResult) != SYSMODULE_ERR_DUPLICATE) {
            std::fprintf(stderr, "Could not load the PNG decoder.\n");
            return false;
        }

        pngData image{};
        const s32 decodeResult = pngLoadFromFile(path, &image);
        if (moduleResult == SYSMODULE_OK) {
            sysModuleUnload(SYSMODULE_PNGDEC);
        }

        // RSX textures support dimensions up to 4096. Bound allocation size too.
        const u64 byteCount = static_cast<u64>(image.pitch) * image.height;
        if (decodeResult != PNGDEC_ERROR_OK || !image.bmp_out ||
            image.width == 0 || image.height == 0 ||
            image.width > 4096 || image.height > 4096 ||
            image.pitch < image.width * 4 || byteCount > 64 * 1024 * 1024) {
            std::free(image.bmp_out);
            std::fprintf(stderr, "Could not load background PNG: %s\n", path);
            return false;
        }

        textureMemory_ = tiny3d_AllocTexture(static_cast<u32>(byteCount));
        if (!textureMemory_) {
            std::free(image.bmp_out);
            std::fprintf(stderr, "Could not allocate the background texture.\n");
            return false;
        }

        std::memcpy(textureMemory_, image.bmp_out, static_cast<std::size_t>(byteCount));
        textureOffset_ = tiny3d_TextureOffset(textureMemory_);
        width_ = image.width;
        height_ = image.height;
        pitch_ = image.pitch;
        std::free(image.bmp_out);
        // Tiny3D uses a linear RSX arena. Its texture pointers cannot be freed
        // individually; the process releases the arena on termination.
        return true;
    }

    void render() const {
        tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);
        tiny3d_Project2D();

        if (textureMemory_) {
            tiny3d_SetTexture(0, textureOffset_, width_, height_, pitch_,
                TINY3D_TEX_FORMAT_A8R8G8B8, 1);
            tiny3d_SetPolygon(TINY3D_QUADS);
            vertex(0.0f, 0.0f, 0.0f, 0.0f);
            vertex(canvasWidth, 0.0f, 1.0f, 0.0f);
            vertex(canvasWidth, canvasHeight, 1.0f, 1.0f);
            vertex(0.0f, canvasHeight, 0.0f, 1.0f);
            tiny3d_End();
        }

        // Continue flipping while the PS menu is open; Tiny3D manages its overlay.
        tiny3d_Flip();
    }

private:
    static void vertex(float x, float y, float u, float v) {
        tiny3d_VertexPos(x, y, 1.0f);
        tiny3d_VertexTexture(u, v);
    }

    static constexpr float canvasWidth = 848.0f;
    static constexpr float canvasHeight = 512.0f;
    bool initialized_ = false;
    void* textureMemory_ = nullptr;
    u32 textureOffset_ = 0;
    u32 width_ = 0;
    u32 height_ = 0;
    u32 pitch_ = 0;
};

} // namespace cellflix
