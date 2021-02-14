#include "FrameBuffer.hpp"

namespace GL
{
    static void GetTextureFormat(
        TextureFormat format,
        GLuint& gl_type,
        GLuint& gl_format,
        GLuint& gl_internal_format)
    {
        switch (format)
        {
        case TextureFormat::RGBA32F:
            gl_type = GL_FLOAT;
            gl_format = GL_RGBA;
            gl_internal_format = GL_RGBA32F;
            break;
        case TextureFormat::SRGB8_ALPHA8:
            gl_type = GL_UNSIGNED_BYTE;
            gl_format = GL_RGBA;
            gl_internal_format = GL_SRGB8_ALPHA8;
            break;
        case TextureFormat::RGBA8:
            gl_type = GL_UNSIGNED_BYTE;
            gl_format = GL_RGBA;
            gl_internal_format = GL_RGBA8;
            break;
        default:
            // not implemented
            assert(false);
        }
    }

    void GenFrameBuffer(
        const uint32_t width,
        const uint32_t height,
        const TextureFormat format,
        const bool mipmaps,
        FrameBuffer& fb)
    {
        GLuint gl_type = GL_UNSIGNED_BYTE;
        GLuint gl_format = GL_RGBA;
        GLuint gl_internal_format = GL_RGBA;

        GL::GetTextureFormat(
            format,
            gl_type,
            gl_format,
            gl_internal_format);

        fb.width = width;
        fb.height = height;

        glGenFramebuffers(
            1,
            &fb.gl_frame_handle);

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            fb.gl_frame_handle);

        glGenTextures(
            1,
            &fb.gl_texture_handle);

        glBindTexture(
            GL_TEXTURE_2D,
            fb.gl_texture_handle);

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
            &fb.gl_depth_renderbuffer_handle);

        glBindRenderbuffer(
            GL_RENDERBUFFER,
            fb.gl_depth_renderbuffer_handle);

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
            fb.gl_depth_renderbuffer_handle);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            fb.gl_texture_handle,
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
}
