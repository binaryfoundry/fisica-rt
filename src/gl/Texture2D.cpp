#include "Texture2D.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace GL
{
    template<>
    Texture2D<TexDataByteRGBA>::Texture2D()
    {
    }

    template<>
    Texture2D<TexDataFloatRGBA>::Texture2D()
    {
    }

    template<typename T>
    Texture2D<T>::~Texture2D()
    {
        assert(!created);
    }

    template<>
    void Texture2D<TexDataByteRGBA>::SetFormat()
    {
        gl_type = GL_UNSIGNED_BYTE;
        gl_format = GL_RGBA;
        gl_internal_format = GL_RGBA8;
    };

    template<>
    void Texture2D<TexDataFloatRGBA>::SetFormat()
    {
        gl_type = GL_FLOAT;
        gl_format = GL_RGBA;
        gl_internal_format = GL_RGBA32F;
    };

    template<typename T>
    void Texture2D<T>::Create(
        const uint32_t width_,
        const uint32_t height_)
    {
        created = true;

        if (data == nullptr)
        {
            data = std::make_shared<std::vector<T>>(
                width_ * height_);
        }

        width = width_;
        height = height_;

        SetFormat();

        glActiveTexture(
            GL_TEXTURE0);

        glGenTextures(
            1, &gl_texture_handle);

        Update();
    };

    template<typename T>
    void Texture2D<T>::Update()
    {
        glActiveTexture(
            GL_TEXTURE0);

        glBindTexture(
            GL_TEXTURE_2D,
            gl_texture_handle);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            gl_internal_format,
            width,
            height,
            0,
            gl_format,
            gl_type,
            (GLvoid*)&((*data)[0]));

        glGenerateMipmap(
            GL_TEXTURE_2D);

        glBindTexture(
            GL_TEXTURE_2D,
            NULL);
    }

    template<>
    void Texture2D<TexDataByteRGBA>::Load(
        const std::string file)
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

        data = std::make_shared<std::vector<TexDataByteRGBA>>(
            raw_data_size);

        data->assign(
            raw_data,
            raw_data + raw_data_size);

        stbi_image_free(
            raw_data);

        Create(
            t_width,
            t_height);
    }

    template<>
    void Texture2D<TexDataFloatRGBA>::Load(
        const std::string file)
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

        data = std::make_shared<std::vector<TexDataFloatRGBA>>(
            raw_data_size);

        data->assign(
            raw_data,
            raw_data + raw_data_size);

        stbi_image_free(
            raw_data);

        Create(
            t_width,
            t_height);
    }

    template<typename T>
    void Texture2D<T>::Delete()
    {
        if (created)
        {
            glDeleteTextures(1, &gl_texture_handle);
        }

        created = false;
    }

    template class Texture2D<TexDataByteRGBA>;
    template class Texture2D<TexDataFloatRGBA>;
}
