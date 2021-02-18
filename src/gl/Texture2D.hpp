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

        std::unique_ptr<std::vector<T>> data;

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

        std::unique_ptr<std::vector<T>>& Data()
        {
            return data;
        };

        void Load(const std::string file);

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