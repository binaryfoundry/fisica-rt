#pragma once

#if !defined (EMSCRIPTEN)
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#else
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

#include <iostream>
#include <string>
#include <vector>

namespace OpenGL
{
    static void GLCheckError()
    {
        GLenum err = GL_NO_ERROR;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "gl error: " << err << std::endl;
        }

        if (err != GL_NO_ERROR)
        {
            throw std::runtime_error("gl error");
        }
    }

    enum class TextureFormat
    {
        RGBA8,
        SRGB8_ALPHA8,
        RGBA32F
    };

    struct FrameBuffer
    {
    public:
        uint32_t width = 0;
        uint32_t height = 0;

        GLuint frame = 0;
        GLuint texture = 0;
        GLuint depth_renderbuffer = 0;

        void Delete()
        {
            glDeleteBuffers(
                1, &frame);

            glDeleteTextures(
                1, &texture);

            glDeleteRenderbuffers(
                1, &depth_renderbuffer);
        }
    };

    GLuint LinkShader(
        std::string vertex_shader_string,
        std::string fragment_shader_string);

    GLuint GenBuffer(
        const std::vector<float>& data);

    GLuint GenBufferIndex(
        const std::vector<uint32_t>& data);

    GLuint GenTextureRGBA8(
        const uint32_t width,
        const uint32_t height,
        uint8_t* data);

    void GenFrameBuffer(
        const uint32_t width,
        const uint32_t height,
        const TextureFormat format,
        const bool mipmaps,
        FrameBuffer& fb);

    template <typename T>
    class UniformBuffer
    {
    public:
        T object;
        GLuint buffer;

        UniformBuffer()
        {
            glGenBuffers(1, &buffer);
        }

        void Delete()
        {
            glDeleteBuffers(1, &buffer);
        }

        void Update()
        {
            glBindBuffer(
                GL_UNIFORM_BUFFER,
                buffer);

#if !defined(EMSCRIPTEN)
            GLvoid* buffer_map;

            buffer_map = glMapBufferRange(
                GL_UNIFORM_BUFFER,
                0,
                sizeof(T),
                GL_MAP_WRITE_BIT);

            memcpy(
                buffer_map,
                (void*)&buffer,
                sizeof(T));

            glUnmapBuffer(GL_UNIFORM_BUFFER);
#else
            glBufferData(
                GL_UNIFORM_BUFFER,
                sizeof(T),
                buffer,
                GL_DYNAMIC_DRAW);

            glBindBuffer(
                GL_UNIFORM_BUFFER,
                0);
#endif
        }
    };
}
