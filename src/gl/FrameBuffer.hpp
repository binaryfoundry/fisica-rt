#pragma once

#include "GL.hpp"

namespace GL
{
    struct FrameBuffer
    {
    public:
        uint32_t width = 0;
        uint32_t height = 0;

        GLuint gl_frame_handle = 0;
        GLuint gl_texture_handle = 0;
        GLuint gl_depth_renderbuffer_handle = 0;

        void Delete()
        {
            glDeleteBuffers(
                1, &gl_frame_handle);

            glDeleteTextures(
                1, &gl_texture_handle);

            glDeleteRenderbuffers(
                1, &gl_depth_renderbuffer_handle);
        }
    };

    void GenFrameBuffer(
        const uint32_t width,
        const uint32_t height,
        const TextureFormat format,
        const bool mipmaps,
        FrameBuffer& fb);
}
