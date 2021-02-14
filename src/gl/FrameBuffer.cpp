#include "FrameBuffer.hpp"

namespace GL
{
    template <typename T>
    void FrameBuffer<T>::Create(
        const uint32_t width_,
        const uint32_t height_,
        const bool mipmaps)
    {
        created = true;

        SetFormat();

        width = width_;
        height = height_;

        glGenFramebuffers(
            1,
            &gl_frame_handle);

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            gl_frame_handle);

        glGenTextures(
            1,
            &gl_texture_handle);

        glBindTexture(
            GL_TEXTURE_2D,
            gl_texture_handle);

        glTexImage2D(
            GL_TEXTURE_2D,
            mipmaps ? 0 : 1,
            gl_internal_format,
            width,
            height,
            0,
            gl_format,
            gl_type,
            0);

        glGenRenderbuffers(
            1,
            &gl_depth_renderbuffer_handle);

        glBindRenderbuffer(
            GL_RENDERBUFFER,
            gl_depth_renderbuffer_handle);

        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH_COMPONENT24,
            width,
            height);

        CheckError();

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            gl_depth_renderbuffer_handle);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            gl_texture_handle,
            0);

        GLenum draw_buffers[1] = {
            GL_COLOR_ATTACHMENT0
        };

        glDrawBuffers(
            1,
            draw_buffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            assert(false);
        }

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            0);
    }

    template <typename T>
    FrameBuffer<T>::~FrameBuffer()
    {
        if (created)
        {
            assert("Resource not deleted.");
        }
    }

    template <typename T>
    void FrameBuffer<T>::Delete()
    {
        if (created)
        {
            glDeleteBuffers(
                1, &gl_frame_handle);

            glDeleteTextures(
                1, &gl_texture_handle);

            glDeleteRenderbuffers(
                1, &gl_depth_renderbuffer_handle);
        }

        created = false;
    }

    template<>
    void FrameBuffer<TexDataByteRGBA>::SetFormat()
    {
        gl_internal_format = GL_RGBA;
        gl_format = GL_RGBA;
        gl_type = GL_UNSIGNED_BYTE;
    };

    template<>
    void FrameBuffer<TexDataFloatRGBA>::SetFormat()
    {
        gl_type = GL_FLOAT;
        gl_format = GL_RGBA;
        gl_internal_format = GL_RGBA32F;
    };

    template class FrameBuffer<TexDataByteRGBA>;
    template class FrameBuffer<TexDataFloatRGBA>;
};

