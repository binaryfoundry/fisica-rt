#pragma once

#include <map>
#include <memory>

#include "OpenGL.hpp"

#include "../properties/Property.hpp"

using namespace Properties;

namespace GL
{
    class Descriptor
    {
    public:
        std::map<std::string, GLuint> sampler2Ds;
        std::map<std::string, GLuint> sampler2D_arrays;
        std::map<std::string, GLuint> uniform_blocks;
        std::map<std::string, Property<glm::mat4>> uniform_mat4s;

        void SetSampler2D(
            std::string name,
            GLTextureResource& texture)
        {
            sampler2Ds[name] = texture.gl_texture_handle;
        }

        void SetSampler2DArray(
            std::string name,
            GLTextureResource& texture)
        {
            sampler2D_arrays[name] = texture.gl_texture_handle;
        }

        void SetUniformBlock(
            std::string name,
            GLBufferResource& uniform_block)
        {
            uniform_blocks[name] = uniform_block.gl_buffer_handle;
        }

        void SetUniformMat4(
            std::string name,
            Property<glm::mat4>& mat4)
        {
            uniform_mat4s[name] = mat4;
        }
    };
}
