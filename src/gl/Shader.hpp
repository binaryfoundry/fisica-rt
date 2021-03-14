#pragma once

#include "OpenGL.hpp"

#include <map>
#include <string>

namespace GL
{
    class Shader
    {
    private:
        GLuint gl_shader_handle;
        std::map<GLuint, GLuint> attribute_locations;
        std::map<std::string, GLuint> uniform_locations;
        std::map<std::string, GLuint> uniform_block_locations;

        uint16_t attributes_total_size = 0;

        bool initialized = false;

        std::string program;

    public:
        virtual ~Shader();

        void Load(std::string file_path);
        void Link(std::string additional_defines = "");
        void Delete();

        void BindAttributes();
    };
}
