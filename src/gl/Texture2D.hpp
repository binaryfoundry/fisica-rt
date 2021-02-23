#pragma once

#include "OpenGL.hpp"

#include <vector>
#include <array>

namespace GL
{
    template <typename T, size_t E = 1>
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

        std::array<std::unique_ptr<std::vector<T>>, E> data;

    public:
        GLuint gl_texture_handle;

        Texture2D(
            const std::string& file_path);

        Texture2D(
            const uint32_t width,
            const uint32_t height);

        Texture2D(const Texture2D&) = delete;

        virtual ~Texture2D();

        void Create(
            const uint32_t width,
            const uint32_t height);

        void Update();

        std::unique_ptr<std::vector<T>>& Data(size_t index = 0)
        {
            return data[index];
        };

        void Load(
            const std::string file,
            const size_t index = 0);

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