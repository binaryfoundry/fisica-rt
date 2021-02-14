#pragma once

#include "OpenGL.hpp"

#include <vector>

namespace GL
{
    template <typename T>
    class Texture2D
    {
    private:
        bool created = false;

        GLuint gl_internal_format = GL_RGBA;
        GLuint gl_format = GL_RGBA;
        GLuint gl_type = GL_UNSIGNED_BYTE;

        void SetFormat();

    public:
        GLuint gl_texture_handle;

        uint32_t width;
        uint32_t height;

        Texture2D();
        virtual ~Texture2D();

        void Create(
            const uint32_t width,
            const uint32_t height,
            const std::vector<T>& data);

        void Delete();
    };
}