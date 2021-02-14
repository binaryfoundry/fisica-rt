#pragma once

#include "GL.hpp"

namespace GL
{
    template <typename T>
    class UniformBuffer
    {
    public:
        T object;
        GLuint gl_buffer_handle;

        UniformBuffer()
        {
            glGenBuffers(1, &gl_buffer_handle);
        }

        void Delete()
        {
            glDeleteBuffers(1, &gl_buffer_handle);
        }

        void Update()
        {
            glBindBuffer(
                GL_UNIFORM_BUFFER,
                gl_buffer_handle);

/*#if !defined(EMSCRIPTEN)
            GLvoid* buffer_map;

            buffer_map = glMapBufferRange(
                GL_UNIFORM_BUFFER,
                0,
                sizeof(T),
                GL_MAP_WRITE_BIT);

            memcpy(
                buffer_map,
                (void*)&object,
                sizeof(T));

            glUnmapBuffer(GL_UNIFORM_BUFFER);
#else*/
            glBufferData(
                GL_UNIFORM_BUFFER,
                sizeof(T),
                (void*)&object,
                GL_DYNAMIC_DRAW);

            glBindBuffer(
                GL_UNIFORM_BUFFER,
                0);
//#endif
        }
    };
}
