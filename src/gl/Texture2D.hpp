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

        uint32_t width;
        uint32_t height;

        GLuint gl_internal_format = GL_RGBA;
        GLuint gl_format = GL_RGBA;
        GLuint gl_type = GL_UNSIGNED_BYTE;

        void SetFormat();

        std::shared_ptr<std::vector<T>> data;

    public:
        GLuint gl_texture_handle;

        Texture2D();
        Texture2D(const Texture2D&) = delete;

        virtual ~Texture2D();

        void Create(
            const uint32_t width,
            const uint32_t height);

        void Update();

        uint32_t Width() { return width; }
        uint32_t Height() { return height; }

        void Load(const std::string file);

        void Delete();
    };
}