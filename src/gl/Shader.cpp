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

    void Shader::Load(std::string file_path)
    {
        File file(file_path, "r");

        program = file.ReadString();
    }

    void Shader::Link(std::string additional_defines)
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
        uniform_locations.clear();
        uniform_block_locations.clear();

        for (auto& attribute : vertex_info.attributes)
        {
            std::string type = std::get<0>(attribute);
            std::string name = std::get<1>(attribute);

            uint16_t size = attribute_size_map.at(type);
            attributes_total_size += size;

            GLuint location = glGetAttribLocation(
                gl_shader_handle,
                name.c_str());

            attribute_locations[location] = size;
        }

        for (auto& uniform : fragment_info.uniforms)
        {
            std::string type = std::get<0>(uniform);
            std::string name = std::get<1>(uniform);

            GLuint location = glGetUniformLocation(
                gl_shader_handle,
                name.c_str());

            uniform_locations[name] = location;
        }

        for (auto& uniform_block : fragment_info.uniform_blocks)
        {
            std::string name = uniform_block.name;

            GLuint location = glGetUniformBlockIndex(
                gl_shader_handle,
                name.c_str());

            uniform_block_locations[name] = location;
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

    void Shader::BindAttributes()
    {
        glUseProgram(
            gl_shader_handle);

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
    }
}
