#include "Descriptor.hpp"

namespace GL
{

    GLint filter_gl_enum(
        Filter filter)
    {
        switch (filter)
        {
        case Filter::NEAREST:
            return GL_NEAREST;
        case Filter::LINEAR:
            return GL_LINEAR;
        case Filter::LINEAR_MIPMAP_LINEAR:
            return GL_LINEAR_MIPMAP_LINEAR;
        case Filter::LINEAR_MIPMAP_NEAREST:
            return GL_LINEAR_MIPMAP_NEAREST;
        case Filter::NEAREST_MIPMAP_LINEAR:
            return GL_LINEAR_MIPMAP_LINEAR;
        case Filter::NEAREST_MIPMAP_NEAREST:
            return GL_LINEAR_MIPMAP_NEAREST;
        }

        assert(false);

        return 0;
    }

    GLint wrap_gl_enum(
        Wrap wrap)
    {
        switch (wrap)
        {
        case Wrap::REPEAT:
            return GL_REPEAT;
        case Wrap::CLAMP_TO_EDGE:
            return GL_CLAMP_TO_EDGE;
        }

        assert(false);

        return 0;
    }

    void Descriptor::SetSampler2D(
        std::string name,
        Texture2DResource& texture,
        Filter min_filter,
        Filter mag_filter,
        Wrap wrap_s,
        Wrap wrap_t,
        Wrap wrap_r)
    {
        auto gl_texture = static_cast<GLTextureResource&>(
            texture);

        sampler2Ds[name] = {
            gl_texture.gl_texture_handle,
            filter_gl_enum(min_filter),
            filter_gl_enum(mag_filter),
            wrap_gl_enum(wrap_s),
            wrap_gl_enum(wrap_t),
            wrap_gl_enum(wrap_r)
        };
    }

    void Descriptor::SetSampler2DArray(
        std::string name,
        Texture2DResource& texture,
        Filter min_filter,
        Filter mag_filter,
        Wrap wrap_s,
        Wrap wrap_t,
        Wrap wrap_r)
    {
        auto gl_texture = static_cast<GLTextureResource&>(
            texture);

        sampler2D_arrays[name] = {
            gl_texture.gl_texture_handle,
            filter_gl_enum(min_filter),
            filter_gl_enum(mag_filter),
            wrap_gl_enum(wrap_s),
            wrap_gl_enum(wrap_t),
            wrap_gl_enum(wrap_r)
        };
    }

    void Descriptor::SetUniformBlock(
        std::string name,
        BufferResource& uniform_block)
    {
        auto gl_uniform_block = static_cast<GLBufferResource&>(
            uniform_block);

        uniform_blocks[name] = gl_uniform_block.gl_buffer_handle;
    }

    void Descriptor::SetUniformMat4(
        std::string name,
        glm::mat4* mat4)
    {
        uniform_mat4s[name] = mat4;
    }

    void Descriptor::SetUniformFloat(
        std::string name,
        float* value)
    {
        uniform_floats[name] = value;
    }
}