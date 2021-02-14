#pragma once

#include "GL.hpp"

#include <vector>

namespace GL
{
    template <typename T>
    class Texture2D
    {
    private:
        GLuint gl_internal_format = GL_RGBA;
        GLuint gl_format = GL_RGBA;
        GLuint gl_type = GL_UNSIGNED_BYTE;

        void SetFormat();

    public:
        GLuint gl_texture_handle;

        uint32_t width;
        uint32_t height;
        std::vector<T> data;

        void Create(
            const uint32_t width,
            const uint32_t height);

        Texture2D();

        void Delete()
        {
            glDeleteTextures(1, &gl_texture_handle);
        }
    };
}