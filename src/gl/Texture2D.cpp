#include "Texture2D.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace GL
{
    template<typename T, size_t E>
    Texture2D<T, E>::Texture2D(
        const std::string& file_path)
    {
        Load(file_path);
        Update();
    }

    template<typename T, size_t E>
    Texture2D<T, E>::Texture2D(
        const uint32_t width,
        const uint32_t height)
    {
        Create(width, height);
    }

    template<typename T, size_t E>
    Texture2D<T, E>::~Texture2D()
    {
        assert(!created);
    }

    template<typename T, size_t E>
    void Texture2D<T, E>::SetFormat()
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
            static_assert(false);
        }
    };

    template<typename T, size_t E>
    void Texture2D<T, E>::Create(
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

    template<typename T, size_t E>
    void Texture2D<T, E>::Update()
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

            glTexParameteri(
                GL_TEXTURE_2D_ARRAY,
                GL_TEXTURE_MIN_FILTER,
                GL_NEAREST);

            glTexParameteri(
                GL_TEXTURE_2D_ARRAY,
                GL_TEXTURE_MAG_FILTER,
                GL_NEAREST);

            glTexParameteri(
                GL_TEXTURE_2D_ARRAY,
                GL_TEXTURE_WRAP_S,
                GL_REPEAT);

            glTexParameteri(
                GL_TEXTURE_2D_ARRAY,
                GL_TEXTURE_WRAP_T,
                GL_REPEAT);

            glGenerateMipmap(
                GL_TEXTURE_2D_ARRAY);

            glBindTexture(
                GL_TEXTURE_2D_ARRAY,
                NULL);
        }
    }

    template<typename T, size_t E>
    void Texture2D<T, E>::Load(
        const std::string file,
        const size_t index)
    {
        if      constexpr (std::is_same_v<T, TexDataByteRGBA>)
        {
            int t_width, t_height, t_channels;

            auto* raw_data = reinterpret_cast<TexDataByteRGBA*>(stbi_load(
                file.c_str(),
                &t_width,
                &t_height,
                &t_channels,
                STBI_rgb_alpha));

            size_t raw_data_size =
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

            auto* raw_data = reinterpret_cast<TexDataFloatRGBA*>(stbi_loadf(
                file.c_str(),
                &t_width,
                &t_height,
                &t_channels,
                STBI_rgb_alpha));

            size_t raw_data_size =
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
            static_assert(false);
        }
    }

    template<typename T, size_t E>
    void Texture2D<T, E>::Delete()
    {
        if (created)
        {
            glDeleteTextures(1, &gl_texture_handle);
        }

        created = false;
    }

    template class Texture2D<TexDataByteRGBA, 1>;
    template class Texture2D<TexDataFloatRGBA, 1>;

    template class Texture2D<TexDataByteRGBA, 2>;
    template class Texture2D<TexDataFloatRGBA, 2>;
}
