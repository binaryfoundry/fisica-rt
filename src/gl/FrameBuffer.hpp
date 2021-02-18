#pragma once

#include "OpenGL.hpp"

namespace GL
{
    template <typename T>
    class FrameBuffer
    {
    private:
        bool created = false;

        uint32_t width = 0;
        uint32_t height = 0;

        GLuint gl_internal_format = GL_RGBA;
        GLuint gl_format = GL_RGBA;
        GLuint gl_type = GL_UNSIGNED_BYTE;

        void SetFormat();

    public:
        virtual ~FrameBuffer();

        GLuint gl_frame_handle = 0;
        GLuint gl_texture_handle = 0;
        GLuint gl_depth_renderbuffer_handle = 0;

        void Create(
            const uint32_t width,
            const uint32_t height,
            const bool mipmaps);

        void Delete();

        uint32_t Width() const
        {
            return width;
        }

        uint32_t Height() const
        {
            return height;
        }
    };
}
