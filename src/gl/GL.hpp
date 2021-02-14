#pragma once

#include "../Math.hpp"

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

namespace GL
{
    static void CheckError()
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


    using TexDataByteRGBA = bvec4;
    using TexDataFloatRGBA = glm::vec4;

    GLuint LinkShader(
        std::string vertex_shader_string,
        std::string fragment_shader_string);

    GLuint GenBuffer(
        const std::vector<float>& data);

    GLuint GenBufferIndex(
        const std::vector<uint32_t>& data);
}
