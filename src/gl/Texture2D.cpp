#include "Texture2D.hpp"

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

    template<>
    Texture2D<TexDataFloatRGB>::Texture2D()
    {
    }

    template<typename T>
    Texture2D<T>::~Texture2D()
    {
        assert(!created);
    }

    template<typename T>
    void Texture2D<T>::Create(
        const uint32_t width_,
        const uint32_t height_,
        const std::vector<T>& data)
    {
        created = true;

        width = width_;
        height = height_;

        SetFormat();

        glActiveTexture(
            GL_TEXTURE0);

        glGenTextures(
            1, &gl_texture_handle);

        Update(data);
    };

    template<typename T>
    void Texture2D<T>::Update(const std::vector<T>& data)
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
            (GLvoid*)&(data[0]));

        glGenerateMipmap(
            GL_TEXTURE_2D);

        glBindTexture(
            GL_TEXTURE_2D,
            NULL);
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

    template<>
    void Texture2D<TexDataFloatRGB>::SetFormat()
    {
        gl_type = GL_FLOAT;
        gl_format = GL_RGB;
        gl_internal_format = GL_RGB32F;
    };

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
    template class Texture2D<TexDataFloatRGB>;
}
