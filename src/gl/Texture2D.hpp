#pragma once

#include "OpenGL.hpp"

#include <vector>
#include <array>

// TODO generic location
enum class Filter
{
    NEAREST,
    LINEAR,
    REPEAT,
    CLAMP_TO_EDGE
};

namespace GL
{
    template <typename T, size_t E = 1>
    class Texture2D : public GLTextureResource
    {
    private:
        bool created = false;

        uint32_t width;
        uint32_t height;

        GLuint gl_internal_format = GL_RGBA;
        GLuint gl_format = GL_RGBA;
        GLuint gl_type = GL_UNSIGNED_BYTE;

        std::array<std::unique_ptr<std::vector<T>>, E> data;

        void SetFormat()
        {
            if      constexpr (std::is_same_v<T, TexDataByteRGBA>)
            {
                gl_type = GL_UNSIGNED_BYTE;
                gl_format = GL_RGBA;
                gl_internal_format = GL_RGBA8;
            }
            else if constexpr (std::is_same_v<T, TexDataFloatRGBA>)
            {
                gl_type = GL_FLOAT;
                gl_format = GL_RGBA;
                gl_internal_format = GL_RGBA32F;
            }
            else
            {
                assert(false);
            }
        };

    public:
        Texture2D(
            const std::string& file_path)
        {
            Load(file_path);
            Update();
        }

        Texture2D(
            const uint32_t width,
            const uint32_t height)
        {
            Create(width, height);
        }

        Texture2D(const Texture2D&) = delete;

        uint32_t Width() const
        {
            return width;
        }

        uint32_t Height() const
        {
            return height;
        }

        virtual ~Texture2D()
        {
            assert(!created);
        }

        std::unique_ptr<std::vector<T>>& Data(
            size_t index = 0)
        {
            return data[index];
        };

        void Create(
            const uint32_t width_,
            const uint32_t height_)
        {
            created = true;

            for (size_t i = 0; i < E; i++)
            {
                if (data[i] == nullptr)
                {
                    data[i] = std::make_unique<std::vector<T>>(
                        width_ * height_);
                }
            }

            width = width_;
            height = height_;

            SetFormat();

            glActiveTexture(
                GL_TEXTURE0);

            glGenTextures(
                1, &gl_texture_handle);
        };

        void Update()
        {
            glActiveTexture(
                GL_TEXTURE0);

            if (E == 1)
            {
                glBindTexture(
                    GL_TEXTURE_2D,
                    gl_texture_handle);

                glTexImage2D(
                    GL_TEXTURE_2D,
                    0, // TODO mipmaps parameter
                    gl_internal_format,
                    width,
                    height,
                    0,
                    gl_format,
                    gl_type,
                    (GLvoid*)&((*data[0])[0]));

                glGenerateMipmap(
                    GL_TEXTURE_2D);

                glBindTexture(
                    GL_TEXTURE_2D,
                    NULL);
            }
            else
            {
                glBindTexture(
                    GL_TEXTURE_2D_ARRAY,
                    gl_texture_handle);

                glTexStorage3D(
                    GL_TEXTURE_2D_ARRAY,
                    1, // TODO mipmaps parameter
                    gl_internal_format,
                    width,
                    height,
                    E);

                for (uint32_t i = 0; i < E; i++)
                {
                    glTexSubImage3D(
                        GL_TEXTURE_2D_ARRAY,
                        0,
                        0,
                        0,
                        i,
                        width,
                        height,
                        1,
                        gl_format,
                        gl_type,
                        (GLvoid*)&((*data[i])[0]));
                }

                GL::CheckError();

                glGenerateMipmap(
                    GL_TEXTURE_2D_ARRAY);

                glBindTexture(
                    GL_TEXTURE_2D_ARRAY,
                    NULL);
            }
        }

        void Load(
            const std::string file,
            const size_t index = 0)
        {
            if      constexpr (std::is_same_v<T, TexDataByteRGBA>)
            {
                int t_width, t_height, t_channels;

                const auto* loaded = stbi_load(
                    file.c_str(),
                    &t_width,
                    &t_height,
                    &t_channels,
                    STBI_rgb_alpha);

                const auto* raw_data = reinterpret_cast<TexDataByteRGBA*>(
                    loaded);

                const size_t raw_data_size =
                    t_width * t_height;

                data[index] = std::make_unique<std::vector<TexDataByteRGBA>>(
                    raw_data_size);

                data[index]->assign(
                    raw_data,
                    raw_data + raw_data_size);

                stbi_image_free(
                    raw_data);

                Create(
                    t_width,
                    t_height);
            }
            else if constexpr (std::is_same_v<T, TexDataFloatRGBA>)
            {
                int t_width, t_height, t_channels;

                const auto* loaded = stbi_loadf(
                    file.c_str(),
                    &t_width,
                    &t_height,
                    &t_channels,
                    STBI_rgb_alpha);

                const auto* raw_data = reinterpret_cast<TexDataFloatRGBA*>(
                    loaded);

                const size_t raw_data_size =
                    t_width * t_height;

                data[index] = std::make_unique<std::vector<TexDataFloatRGBA>>(
                    raw_data_size);

                data[index]->assign(
                    raw_data,
                    raw_data + raw_data_size);

                stbi_image_free(
                    raw_data);

                Create(
                    t_width,
                    t_height);
            }
            else
            {
                assert(false);
            }
        }

        void Delete()
        {
            if (created)
            {
                glDeleteTextures(
                    1, &gl_texture_handle);
            }

            created = false;
        }
    };
}