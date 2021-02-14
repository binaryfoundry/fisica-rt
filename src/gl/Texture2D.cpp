#include "Texture2d.hpp"

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
        if (created)
        {
            assert("Resource not deleted.");
        }
    }

    template<typename T>
    void Texture2D<T>::Create(
        const uint32_t width,
        const uint32_t height,
        const std::vector<T>& data)
    {
        created = true;

        glActiveTexture(
            GL_TEXTURE0);

        glGenTextures(
            1, &gl_texture_handle);

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
    };

    template<>
    void Texture2D<TexDataByteRGBA>::SetFormat()
    {
        gl_internal_format = GL_RGBA;
        gl_format = GL_RGBA;
        gl_type = GL_UNSIGNED_BYTE;
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
