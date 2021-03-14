#include "Shader.hpp"

#include "Parser.hpp"
#include "../File.hpp"

const std::map<std::string, uint16_t> attribute_size_map =
{
    std::make_pair("vec2", 2),
    std::make_pair("vec3", 3),
    std::make_pair("vec4", 4),
};

namespace GL
{
    Shader::~Shader()
    {
        if (initialized)
        {
            assert(false);
        }
    }

    void Shader::Load(
        std::string file_path)
    {
        File file(file_path, "r");

        program = file.ReadString();
    }

    void Shader::Link(
        std::string additional_defines)
    {
        initialized = true;

        gl_shader_handle = LinkShaderFile(
            program,
            additional_defines);

        GL::CheckError();

        Parser vertex_info = Parser(
            ShaderParseType::VERTEX,
            program);

        Parser fragment_info = Parser(
            ShaderParseType::FRAGMENT,
            program);

        attributes_total_size = 0;
        attribute_locations.clear();
        sampler2D_locations.clear();
        sampler2D_array_locations.clear();
        uniform_block_locations.clear();

        descriptor_sets.clear();

        for (auto& attribute : vertex_info.attributes)
        {
            std::string type = std::get<0>(attribute);
            std::string name = std::get<1>(attribute);

            uint16_t size = attribute_size_map.at(type);
            attributes_total_size += size;

            GLuint location = glGetAttribLocation(
                gl_shader_handle,
                name.c_str());

            if (location == std::numeric_limits<GLuint>::max())
            {
                // TODO print warning
                continue;
            }

            attribute_locations[location] = size;
        }

        for (auto& sampler : fragment_info.uniform_sampler2Ds)
        {
            std::string type = std::get<0>(sampler);
            std::string name = std::get<1>(sampler);

            GLuint location = glGetUniformLocation(
                gl_shader_handle,
                name.c_str());

            sampler2D_locations[name] = location;
        }

        for (auto& sampler : fragment_info.uniform_sampler2D_arrays)
        {
            std::string type = std::get<0>(sampler);
            std::string name = std::get<1>(sampler);

            GLuint location = glGetUniformLocation(
                gl_shader_handle,
                name.c_str());

            sampler2D_array_locations[name] = location;
        }

        for (auto& uniform_block : fragment_info.uniform_blocks)
        {
            std::string name = uniform_block.name;

            GLuint location = glGetUniformBlockIndex(
                gl_shader_handle,
                name.c_str());

            uniform_block_locations[name] = location;
        }

        for (auto& uniform : vertex_info.uniform_mat4s)
        {
            std::string type = std::get<0>(uniform);
            std::string name = std::get<1>(uniform);

            GLuint location = glGetUniformLocation(
                gl_shader_handle,
                name.c_str());

            uniform_mat4_locations[name] = location;
        }
    }

    void Shader::Delete()
    {
        if (initialized)
        {
            glDeleteProgram(
                gl_shader_handle);
        }

        initialized = false;
    }

    void Shader::Set(
        Descriptor& descriptor,
        uint32_t index)
    {
        DescriptorSet set;

        for (auto& sampler : descriptor.sampler2Ds)
        {
            std::string name = sampler.first;
            SamplerDescriptor& desc = sampler.second;

            if (sampler2D_locations.find(name) ==
                sampler2D_locations.end())
            {
                throw new std::runtime_error(
                    "No matching uniform found");
            }

            GLuint location = sampler2D_locations.at(
                name);

            if (location == std::numeric_limits<GLuint>::max())
            {
                // TODO print warning
                continue;
            }

            set.sampler2Ds.push_back({
                location,
                desc
            });
        }

        for (auto& sampler : descriptor.sampler2D_arrays)
        {
            std::string name = sampler.first;
            SamplerDescriptor& desc = sampler.second;

            if (sampler2D_array_locations.find(name) ==
                sampler2D_array_locations.end())
            {
                throw new std::runtime_error(
                    "No matching uniform found");
            }

            GLuint location = sampler2D_array_locations.at(
                name);

            if (location == std::numeric_limits<GLuint>::max())
            {
                // TODO print warning
                continue;
            }

            set.sampler2D_arrays.push_back({
                location,
                desc
            });
        }

        for (auto& ubo : descriptor.uniform_blocks)
        {
            std::string name = ubo.first;
            GLuint handle = ubo.second;

            if (uniform_block_locations.find(name) ==
                uniform_block_locations.end())
            {
                throw new std::runtime_error(
                    "No matching uniform block found");
            }

            GLuint location = uniform_block_locations.at(
                name);

            if (location == std::numeric_limits<GLuint>::max())
            {
                // TODO print warning
                continue;
            }

            set.uniform_blocks.push_back({
                location,
                handle
            });
        }

        for (auto& uniform : descriptor.uniform_mat4s)
        {
            std::string name = uniform.first;
            glm::mat4* data = uniform.second;

            if (uniform_mat4_locations.find(name) ==
                uniform_mat4_locations.end())
            {
                throw new std::runtime_error(
                    "No matching uniform found");
            }

            GLuint location = uniform_mat4_locations.at(
                name);

            if (location == std::numeric_limits<GLuint>::max())
            {
                // TODO print warning
                continue;
            }

            set.uniform_mat4s.push_back({
                location,
                data
            });
        }

        descriptor_sets[index] = set;
    }

    void Shader::Bind(
        uint32_t descriptor_set_index)
    {
        GL::CheckError();

        glUseProgram(
            gl_shader_handle);

        GL::CheckError();

        GLuint accumulated_size = 0;

        for (auto& attribute : attribute_locations)
        {
            GLuint location = attribute.first;
            GLuint size = attribute.second;

            glEnableVertexAttribArray(
                location);

            glVertexAttribPointer(
                location,
                size,
                GL_FLOAT,
                GL_FALSE,
                attributes_total_size * sizeof(GLfloat),
                (GLvoid*)(accumulated_size * sizeof(GLfloat)));

            accumulated_size += size;
        }

        GL::CheckError();

        if (descriptor_sets.find(descriptor_set_index) ==
            descriptor_sets.end())
        {
            throw new std::runtime_error(
                "No matching descriptor set found");
        }

        uint32_t sampler_count = 0;
        DescriptorSet& set = descriptor_sets[descriptor_set_index];

        for (auto& texture : set.sampler2Ds)
        {
            GLuint location = std::get<0>(texture);
            SamplerDescriptor& desc = std::get<1>(texture);

            glActiveTexture(
                GL_TEXTURE0 + sampler_count);

            glBindTexture(
                GL_TEXTURE_2D,
                desc.handle);

            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_MIN_FILTER,
                desc.min_filter);

            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_MAG_FILTER,
                desc.mag_filter);

            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_WRAP_S,
                desc.wrap_s);

            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_WRAP_T,
                desc.wrap_t);

            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_WRAP_R,
                desc.wrap_r);

            glUniform1i(
                location,
                sampler_count);

            sampler_count++;
        }

        GL::CheckError();

        for (auto& texture : set.sampler2D_arrays)
        {
            GLuint location = std::get<0>(texture);
            SamplerDescriptor& desc = std::get<1>(texture);

            glActiveTexture(
                GL_TEXTURE0 + sampler_count);

            glBindTexture(
                GL_TEXTURE_2D_ARRAY,
                desc.handle);

            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_MAG_FILTER,
                desc.mag_filter);

            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_WRAP_S,
                desc.wrap_s);

            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_WRAP_T,
                desc.wrap_t);

            glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_WRAP_R,
                desc.wrap_r);

            glUniform1i(
                location,
                sampler_count);

            sampler_count++;
        }

        for (auto& ubo : set.uniform_blocks)
        {
            GLuint location = std::get<0>(ubo);
            GLuint handle = std::get<1>(ubo);

            glBindBufferBase(
                GL_UNIFORM_BUFFER,
                location,
                handle);

            glUniformBlockBinding(
                gl_shader_handle,
                location,
                location);
        }

        for (auto& uniform : set.uniform_mat4s)
        {
            GLuint location = std::get<0>(uniform);
            glm::mat4 data = *std::get<1>(uniform);

            glUniformMatrix4fv(
                location,
                1,
                false,
                &data[0][0]);
        }
    }
}
