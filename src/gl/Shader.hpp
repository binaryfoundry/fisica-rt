#pragma once

#include "OpenGL.hpp"
#include "Descriptor.hpp"

#include <map>
#include <tuple>
#include <vector>
#include <string>

namespace GL
{
    class DescriptorSet
    {
    public:
        std::vector<std::tuple<GLuint, SamplerDescriptor>> sampler2Ds;
        std::vector<std::tuple<GLuint, SamplerDescriptor>> sampler2D_arrays;
        std::vector<std::tuple<GLuint, GLuint>> uniform_blocks;
        std::vector<std::tuple<GLuint, glm::mat4*>> uniform_mat4s;
        std::vector<std::tuple<GLuint, float*>> uniform_floats;
    };

    class Shader
    {
    private:
        GLuint gl_shader_handle;

        std::unordered_map<uint32_t, DescriptorSet> descriptor_sets;

        uint16_t attributes_total_size = 0;
        std::map<GLuint, GLuint> attribute_locations;

        std::map<std::string, GLuint> sampler2D_locations;
        std::map<std::string, GLuint> sampler2D_array_locations;
        std::map<std::string, GLuint> uniform_block_locations;
        std::map<std::string, GLuint> uniform_mat4_locations;
        std::map<std::string, GLuint> uniform_float_locations;

        bool initialized = false;

        std::string program;

    public:
        virtual ~Shader();

        void Load(const std::string file_path);
        void Link(const std::string additional_defines = "");
        void Delete();

        void Set(
            const Descriptor& descriptor,
            const uint32_t index);

        void Bind(
            const uint32_t descriptor_set_index = 0);
    };
}
