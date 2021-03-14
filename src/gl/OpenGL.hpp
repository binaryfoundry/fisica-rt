#pragma once

#include "../math/Math.hpp"

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
    enum class TextureFormat
    {
        RGBA8,
        SRGB8_ALPHA8,
        RGBA32F
    };

    class GLTextureResource
    {
    public:
        GLuint gl_texture_handle = 0;
    };

    class GLBufferResource
    {
    public:
        GLuint gl_buffer_handle = 0;
    };

    void CheckError();

    extern void CreateQuad();
    extern void DeleteQuad();
    extern const std::vector<uint32_t> quad_indices_data;
    extern GLuint quad_vertex_buffer;
    extern GLuint quad_index_buffer;

    GLuint LinkShaderFile(
        const std::string shader_string,
        const std::string defines = "");

    GLuint LinkShader(
        const std::string vertex_shader_string,
        const std::string fragment_shader_string);

    GLuint GenBuffer(
        const std::vector<float>& data);

    GLuint GenBufferIndex(
        const std::vector<uint32_t>& data);
}
