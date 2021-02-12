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

    void GenFrameBufferRGBA8(
        const uint32_t width,
        const uint32_t height,
        const bool mipmaps,
        FrameBuffer& fb);
}
