#include "OpenGL.hpp"

#include <assert.h>
#include <sstream>

namespace GL
{
    GLuint LoadShader(
        const GLenum type,
        const char* shader_src);

    std::string InsertDefines(
        const std::string shader_string,
        const std::string defines)
    {
        const auto it = std::find(
            shader_string.begin(),
            shader_string.end(),
            '\n');

        const auto index = std::distance(
            shader_string.begin(),
            it);

        const std::string header = shader_string.substr(
            0,
            index + 1);

        const std::string body = shader_string.substr(
            index + 1);

        std::stringstream shader;
        shader << header;
        shader << defines << std::endl;
        shader << "#line 2" << std::endl;
        shader << body;
        return shader.str();
    }

    GLuint LinkShaderFile(
        const std::string shader_string)
    {
        const std::string vertex_shader_string = InsertDefines(
            shader_string,
            "#define COMPILING_VS");

        const std::string fragment_shader_string = InsertDefines(
            shader_string,
            "#define COMPILING_FS");

        return LinkShader(
            vertex_shader_string,
            fragment_shader_string);
    }

    GLuint LinkShader(
        const std::string vertex_shader_string,
        const std::string fragment_shader_string)
    {
        const GLuint vertex_shader = LoadShader(
            GL_VERTEX_SHADER,
            vertex_shader_string.c_str());

        const GLuint fragment_shader = LoadShader(
            GL_FRAGMENT_SHADER,
            fragment_shader_string.c_str());

        const GLuint program_object = glCreateProgram();

        if (program_object == 0)
        {
            throw std::runtime_error(
                "shader init error");
        }

        glAttachShader(
            program_object,
            vertex_shader);

        glAttachShader(
            program_object,
            fragment_shader);

        glLinkProgram(
            program_object);

        GLint linked;

        glGetProgramiv(
            program_object,
            GL_LINK_STATUS,
            &linked);

        if (!linked)
        {
            GLint info_len = 0;
            glGetProgramiv(
                program_object,
                GL_INFO_LOG_LENGTH,
                &info_len);

            if (info_len > 1)
            {
                char* info_log = new char[sizeof(char) * info_len];
                glGetProgramInfoLog(
                    program_object,
                    info_len,
                    NULL,
                    info_log);
                std::cout << "link error " << info_log << std::endl;
                free(info_log);
            }

            glDeleteProgram(
                program_object);
            throw std::runtime_error(
                "shader init error");
        }

        return program_object;
    }

    GLuint LoadShader(
        const GLenum type,
        const char* shader_src)
    {
        const GLuint shader = glCreateShader(type);

        if (shader == 0)
            return 0;

        glShaderSource(
            shader,
            1,
            &shader_src,
            NULL);

        GLint compiled;

        glCompileShader(
            shader);

        glGetShaderiv(
            shader,
            GL_COMPILE_STATUS,
            &compiled);

        if (!compiled)
        {
            GLint info_len = 0;
            glGetShaderiv(
                shader,
                GL_INFO_LOG_LENGTH,
                &info_len);

            if (info_len > 1)
            {
                char* info_log = new char[sizeof(char) * info_len];
                glGetShaderInfoLog(
                    shader,
                    info_len,
                    NULL,
                    info_log);
                std::cout << "compile error " << info_log << std::endl;
            }

            glDeleteShader(
                shader);
            throw std::runtime_error(
                "shader init error");
        }

        return shader;
    }

    GLuint GenBuffer(
        const std::vector<float>& data)
    {
        GLuint gl_buffer_handle;
        const GLuint gl_target = GL_ARRAY_BUFFER;

        glGenBuffers(
            1,
            &gl_buffer_handle);

        glBindBuffer(
            gl_target,
            gl_buffer_handle);

        glBufferData(
            gl_target,
            sizeof(float) * data.size(),
            &(data)[0],
            GL_STATIC_DRAW);

        glBindBuffer(
            gl_target,
            NULL);

        return gl_buffer_handle;
    }

    GLuint GenBufferIndex(
        const std::vector<uint32_t>& data)
    {
        GLuint gl_buffer_handle;
        const GLuint gl_target = GL_ELEMENT_ARRAY_BUFFER;

        glGenBuffers(
            1,
            &gl_buffer_handle);

        glBindBuffer(
            gl_target,
            gl_buffer_handle);

        glBufferData(
            gl_target,
            sizeof(uint32_t) *data.size(),
            &(data)[0],
            GL_STATIC_DRAW);

        glBindBuffer(
            gl_target,
            NULL);

        return gl_buffer_handle;
    }

    void CheckError()
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
}
