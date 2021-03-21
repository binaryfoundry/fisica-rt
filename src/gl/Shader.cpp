#include "Shader.hpp"

#include "Parser.hpp"
#include "Texture2D.hpp"
#include "../File.hpp"

const std::map<std::string, uint16_t> attribute_size_map =
{
    std::make_pair("vec2", 2),
    std::make_pair("vec3", 3),
    std::make_pair("vec4", 4),
};

constexpr GLuint gl_not_found = std::numeric_limits<GLuint>::max();

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
        const std::string file_path)
    {
        File file(file_path, "r");

        program = file.ReadString();
    }

    void Shader::Link(
        const std::string additional_defines)
    {
        initialized = true;

        gl_shader_handle = LinkShaderFile(
            program,
            additional_defines);

        GL::CheckError();

        const Parser vertex_info = Parser(
            ShaderParseType::VERTEX,
            program);

        const Parser fragment_info = Parser(
            ShaderParseType::FRAGMENT,
            program);

        attributes_total_size = 0;
        attribute_locations.clear();
        sampler2D_locations.clear();
        sampler2D_array_locations.clear();
        uniform_block_locations.clear();
        uniform_mat4_locations.clear();
        uniform_float_locations.clear();

        descriptor_sets.clear();

        Descriptor empty_set;
        Set(empty_set, 0);

        for (const auto& attribute : vertex_info.attributes)
        {
            const std::string type = std::get<0>(attribute);
            const std::string name = std::get<1>(attribute);

            const uint16_t size = attribute_size_map.at(type);
            attributes_total_size += size;

            const GLuint location = glGetAttribLocation(
                gl_shader_handle,
                name.c_str());

            if (location == gl_not_found)
            {
                // TODO print warning
                continue;
            }

            attribute_locations[location] = size;
        }

        auto uniform_sampler2Ds =
            vertex_info.uniform_sampler2Ds;

        auto uniform_sampler2D_arrays =
            vertex_info.uniform_sampler2D_arrays;

        auto uniform_blocks =
            vertex_info.uniform_blocks;

        auto uniform_mat4s =
            vertex_info.uniform_mat4s;

        auto uniform_floats =
            vertex_info.uniform_floats;

        uniform_sampler2Ds.insert(
            std::end(uniform_sampler2Ds),
            std::begin(fragment_info.uniform_sampler2Ds),
            std::end(fragment_info.uniform_sampler2Ds));

        uniform_sampler2D_arrays.insert(
            std::end(uniform_sampler2D_arrays),
            std::begin(fragment_info.uniform_sampler2D_arrays),
            std::end(fragment_info.uniform_sampler2D_arrays));

        uniform_blocks.insert(
            std::end(uniform_blocks),
            std::begin(fragment_info.uniform_blocks),
            std::end(fragment_info.uniform_blocks));

        uniform_mat4s.insert(
            std::end(uniform_mat4s),
            std::begin(fragment_info.uniform_mat4s),
            std::end(fragment_info.uniform_mat4s));

        uniform_floats.insert(
            std::end(uniform_floats),
            std::begin(fragment_info.uniform_floats),
            std::end(fragment_info.uniform_floats));

        for (const auto& sampler : uniform_sampler2Ds)
        {
            const std::string type = std::get<0>(sampler);
            const std::string name = std::get<1>(sampler);

            const GLuint location = glGetUniformLocation(
                gl_shader_handle,
                name.c_str());

            sampler2D_locations[name] = location;
        }

        for (const auto& sampler : uniform_sampler2D_arrays)
        {
            const std::string type = std::get<0>(sampler);
            const std::string name = std::get<1>(sampler);

            const GLuint location = glGetUniformLocation(
                gl_shader_handle,
                name.c_str());

            sampler2D_array_locations[name] = location;
        }

        for (const auto& uniform_block : uniform_blocks)
        {
            const std::string name = uniform_block.name;

            const GLuint location = glGetUniformBlockIndex(
                gl_shader_handle,
                name.c_str());

            uniform_block_locations[name] = location;
        }

        for (const auto& uniform : uniform_mat4s)
        {
            const std::string type = std::get<0>(uniform);
            const std::string name = std::get<1>(uniform);

            const GLuint location = glGetUniformLocation(
                gl_shader_handle,
                name.c_str());

            uniform_mat4_locations[name] = location;
        }

        for (const auto& uniform : uniform_floats)
        {
            const std::string type = std::get<0>(uniform);
            const std::string name = std::get<1>(uniform);

            const GLuint location = glGetUniformLocation(
                gl_shader_handle,
                name.c_str());

            uniform_float_locations[name] = location;
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
        const Descriptor& descriptor,
        const uint32_t index)
    {
        DescriptorSet set;

        for (const auto& sampler : descriptor.sampler2Ds)
        {
            const std::string name = sampler.first;
            const SamplerDescriptor& desc = sampler.second;

            if (sampler2D_locations.find(name) ==
                sampler2D_locations.end())
            {
                throw new std::runtime_error(
                    "No matching uniform found");
            }

            const GLuint location = sampler2D_locations.at(
                name);

            if (location == gl_not_found)
            {
                // TODO print warning
                continue;
            }

            set.sampler2Ds.push_back({
                location,
                desc
            });
        }

        for (const auto& sampler : descriptor.sampler2D_arrays)
        {
            const std::string name = sampler.first;
            const SamplerDescriptor& desc = sampler.second;

            if (sampler2D_array_locations.find(name) ==
                sampler2D_array_locations.end())
            {
                throw new std::runtime_error(
                    "No matching uniform found");
            }

            const GLuint location = sampler2D_array_locations.at(
                name);

            if (location == gl_not_found)
            {
                // TODO print warning
                continue;
            }

            set.sampler2D_arrays.push_back({
                location,
                desc
            });
        }

        for (const auto& ubo : descriptor.uniform_blocks)
        {
            const std::string name = ubo.first;
            const GLuint handle = ubo.second;

            if (uniform_block_locations.find(name) ==
                uniform_block_locations.end())
            {
                throw new std::runtime_error(
                    "No matching uniform block found");
            }

            const GLuint location = uniform_block_locations.at(
                name);

            if (location == gl_not_found)
            {
                // TODO print warning
                continue;
            }

            set.uniform_blocks.push_back({
                location,
                handle
            });
        }

        for (const auto& uniform : descriptor.uniform_mat4s)
        {
            const std::string name = uniform.first;
            glm::mat4* data = uniform.second;

            if (uniform_mat4_locations.find(name) ==
                uniform_mat4_locations.end())
            {
                throw new std::runtime_error(
                    "No matching uniform found");
            }

            const GLuint location = uniform_mat4_locations.at(
                name);

            if (location == gl_not_found)
            {
                // TODO print warning
                continue;
            }

            set.uniform_mat4s.push_back({
                location,
                data
            });
        }

        for (const auto& uniform : descriptor.uniform_floats)
        {
            const std::string name = uniform.first;
            float* data = uniform.second;

            if (uniform_float_locations.find(name) ==
                uniform_float_locations.end())
            {
                throw new std::runtime_error(
                    "No matching uniform found");
            }

            const GLuint location = uniform_float_locations.at(
                name);

            if (location == gl_not_found)
            {
                // TODO print warning
                continue;
            }

            set.uniform_floats.push_back({
                location,
                data
            });
        }

        descriptor_sets[index] = set;
    }

    void Shader::Bind(
        const uint32_t descriptor_set_index)
    {
        GL::CheckError();

        glUseProgram(
            gl_shader_handle);

        GL::CheckError();

        GLuint accumulated_size = 0;

        for (auto& attribute : attribute_locations)
        {
            const GLuint location = attribute.first;
            const GLuint size = attribute.second;

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

        for (const auto& texture : set.sampler2Ds)
        {
            const GLuint location = std::get<0>(texture);
            const SamplerDescriptor& desc = std::get<1>(texture);

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

        for (const auto& texture : set.sampler2D_arrays)
        {
            const GLuint location = std::get<0>(texture);
            const SamplerDescriptor& desc = std::get<1>(texture);

            glActiveTexture(
                GL_TEXTURE0 + sampler_count);

            glBindTexture(
                GL_TEXTURE_2D_ARRAY,
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

        for (const auto& ubo : set.uniform_blocks)
        {
            const GLuint location = std::get<0>(ubo);
            const GLuint handle = std::get<1>(ubo);

            glBindBufferBase(
                GL_UNIFORM_BUFFER,
                location,
                handle);

            glUniformBlockBinding(
                gl_shader_handle,
                location,
                location);
        }

        for (const auto& uniform : set.uniform_mat4s)
        {
            const GLuint location = std::get<0>(uniform);
            const glm::mat4 data = *std::get<1>(uniform);

            glUniformMatrix4fv(
                location,
                1,
                false,
                &data[0][0]);
        }

        for (const auto& uniform : set.uniform_floats)
        {
            const GLuint location = std::get<0>(uniform);
            const GLfloat data = *std::get<1>(uniform);

            glUniform1f(
                location,
                data);
        }
    }
}
