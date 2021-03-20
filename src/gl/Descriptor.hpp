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
        Filter min_filter;
        Filter mag_filter;
        Filter wrap_s;
        Filter wrap_t;
        Filter wrap_r;
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
            Filter wrap_s,
            Filter wrap_t,
            Filter wrap_r = Filter::REPEAT)
        {
            sampler2Ds[name] = {
                texture.gl_texture_handle,
                min_filter,
                mag_filter,
                wrap_s,
                wrap_t,
                wrap_r
            };
        }

        void SetSampler2DArray(
            std::string name,
            GLTextureResource& texture,
            Filter min_filter,
            Filter mag_filter,
            Filter wrap_s,
            Filter wrap_t,
            Filter wrap_r = Filter::REPEAT)
        {
            sampler2D_arrays[name] = {
                texture.gl_texture_handle,
                min_filter,
                mag_filter,
                wrap_s,
                wrap_t,
                wrap_r
            };
        }

        void SetUniformBlock(
            std::string name,
            GLBufferResource& uniform_block)
        {
            uniform_blocks[name] = uniform_block.gl_buffer_handle;
        }

        void SetUniformMat4(
            std::string name,
            glm::mat4* mat4)
        {
            uniform_mat4s[name] = mat4;
        }

        void SetUniformFloat(
            std::string name,
            float* value)
        {
            uniform_floats[name] = value;
        }
    };
}
