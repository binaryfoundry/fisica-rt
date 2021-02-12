#include "GL.hpp"

#include <assert.h>

namespace OpenGL
{
    GLuint LoadShader(
        GLenum type,
        const char *shader_src);

    GLuint LinkShader(
        std::string vertex_shader_string,
        std::string fragment_shader_string)
    {
        GLuint vertex_shader = LoadShader(
            GL_VERTEX_SHADER,
            vertex_shader_string.c_str());

        GLuint fragment_shader = LoadShader(
            GL_FRAGMENT_SHADER,
            fragment_shader_string.c_str());

        GLuint program_object = glCreateProgram();

        if (program_object == 0)
        {
            throw std::runtime_error(
                "shader init error");
        }

        glAttachShader(
            program_object,
            vertex_shader);

        glAttachShader(
            program_object,
            fragment_shader);

        glLinkProgram(
            program_object);

        GLint linked;

        glGetProgramiv(
            program_object,
            GL_LINK_STATUS,
            &linked);

        if (!linked)
        {
            GLint info_len = 0;
            glGetProgramiv(
                program_object,
                GL_INFO_LOG_LENGTH,
                &info_len);

            if (info_len > 1)
            {
                char* info_log = new char[sizeof(char) * info_len];
                glGetProgramInfoLog(
                    program_object,
                    info_len,
                    NULL,
                    info_log);
                std::cout << "link error " << info_log << std::endl;
                free(info_log);
            }

            glDeleteProgram(
                program_object);
            throw std::runtime_error(
                "shader init error");
        }

        return program_object;
    }

    GLuint LoadShader(GLenum type, const char *shader_src)
    {
        GLuint shader;
        GLint compiled;

        shader = glCreateShader(type);
        if (shader == 0)
            return 0;

        glShaderSource(
            shader,
            1,
            &shader_src,
            NULL);

        glCompileShader(
            shader);

        glGetShaderiv(
            shader,
            GL_COMPILE_STATUS,
            &compiled);

        if (!compiled)
        {
            GLint info_len = 0;
            glGetShaderiv(
                shader,
                GL_INFO_LOG_LENGTH,
                &info_len);

            if (info_len > 1)
            {
                char* info_log = new char[sizeof(char) * info_len];
                glGetShaderInfoLog(
                    shader,
                    info_len,
                    NULL,
                    info_log);
                std::cout << "compile error " << info_log << std::endl;
            }

            glDeleteShader(
                shader);
            throw std::runtime_error(
                "shader init error");
        }

        return shader;
    }

    GLuint GenBuffer(const std::vector<float>& data)
    {
        GLuint gl_buffer_handle;
        const GLuint gl_target = GL_ARRAY_BUFFER;

        glGenBuffers(
            1,
            &gl_buffer_handle);

        glBindBuffer(
            gl_target,
            gl_buffer_handle);

        glBufferData(
            gl_target,
            sizeof(float) * data.size(),
            &(data)[0],
            GL_STATIC_DRAW);

        glBindBuffer(
            gl_target,
            NULL);

        return gl_buffer_handle;
    }

    GLuint GenBufferIndex(
        const std::vector<uint32_t>& data)
    {
        GLuint gl_buffer_handle;
        const GLuint gl_target = GL_ELEMENT_ARRAY_BUFFER;

        glGenBuffers(
            1,
            &gl_buffer_handle);

        glBindBuffer(
            gl_target,
            gl_buffer_handle);

        glBufferData(
            gl_target,
            sizeof(uint32_t) *data.size(),
            &(data)[0],
            GL_STATIC_DRAW);

        glBindBuffer(
            gl_target,
            NULL);

        return gl_buffer_handle;
    }

    GLuint GenTextureRGBA8(
        const uint32_t width,
        const uint32_t height,
        uint8_t* data)
    {
        GLuint gl_texture_handle;

        glActiveTexture(
            GL_TEXTURE0);

        glGenTextures(
            1, &gl_texture_handle);

        glBindTexture(
            GL_TEXTURE_2D,
            gl_texture_handle);

        GLuint gl_internal_format = GL_RGBA;
        GLuint gl_format = GL_RGBA;
        GLuint gl_type = GL_UNSIGNED_BYTE;

        uint8_t* data_ptr = &(data)[0];
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            gl_internal_format,
            width,
            height,
            0,
            gl_format,
            gl_type,
            (GLvoid*)data_ptr);

        glGenerateMipmap(
            GL_TEXTURE_2D);

        glBindTexture(
            GL_TEXTURE_2D,
            NULL);

        return gl_texture_handle;
    }

    void GenFrameBufferRGBA8(
        const uint32_t width,
        const uint32_t height,
        const bool mipmaps,
        FrameBuffer& fb)
    {
        const GLuint gl_type = GL_UNSIGNED_BYTE;
        const GLuint gl_format = GL_RGBA;
        const GLuint gl_internal_format = GL_RGBA;

        fb.width = width;
        fb.height = height;

        glGenFramebuffers(
            1,
            &fb.frame);

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            fb.frame);

        glGenTextures(
            1,
            &fb.texture);

        glBindTexture(
            GL_TEXTURE_2D,
            fb.texture);

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
            &fb.depth_renderbuffer);

        glBindRenderbuffer(
            GL_RENDERBUFFER,
            fb.depth_renderbuffer);

        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH_COMPONENT24,
            width,
            height);

        GLCheckError();

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            fb.depth_renderbuffer);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            fb.texture,
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
