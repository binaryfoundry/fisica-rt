#pragma once

#include <map>
#include <memory>

#include "OpenGL.hpp"
#include "Texture2D.hpp"

#include "../properties/Property.hpp"

using namespace Properties;

namespace GL
{
    struct SamplerDescriptor
    {
        GLuint handle;
        GLint min_filter;
        GLint mag_filter;
        GLint wrap_s;
        GLint wrap_t;
        GLint wrap_r;
    };

    class Descriptor
    {
    public:
        std::map<std::string, SamplerDescriptor> sampler2Ds;
        std::map<std::string, SamplerDescriptor> sampler2D_arrays;
        std::map<std::string, GLuint> uniform_blocks;
        std::map<std::string, glm::mat4*> uniform_mat4s;
        std::map<std::string, float*> uniform_floats;

        void SetSampler2D(
            std::string name,
            GLTextureResource& texture,
            Filter min_filter,
            Filter mag_filter,
            Wrap wrap_s,
            Wrap wrap_t,
            Wrap wrap_r = Wrap::REPEAT);

        void SetSampler2DArray(
            std::string name,
            GLTextureResource& texture,
            Filter min_filter,
            Filter mag_filter,
            Wrap wrap_s,
            Wrap wrap_t,
            Wrap wrap_r = Wrap::REPEAT);

        void SetUniformBlock(
            std::string name,
            GLBufferResource& uniform_block);

        void SetUniformMat4(
            std::string name,
            glm::mat4* mat4);

        void SetUniformFloat(
            std::string name,
            float* value);
    };
}
