#pragma once

#include "OpenGL.hpp"

namespace GL
{
    template <typename T>
    class UniformBuffer
    {
    private:
        bool created = false;

    public:
        T object;
        GLuint gl_buffer_handle;

        UniformBuffer()
        {
            // Must be padded to 16 byte multiples
            assert(sizeof(T) % 16 == 0);

            glGenBuffers(1, &gl_buffer_handle);
        }

        virtual ~UniformBuffer()
        {
            assert(!created);
        }

        void Delete()
        {
            if (created)
            {
                glDeleteBuffers(1, &gl_buffer_handle);
            }

            created = false;
        }

        void Update()
        {
            created = true;

            glBindBuffer(
                GL_UNIFORM_BUFFER,
                gl_buffer_handle);

            glBufferData(
                GL_UNIFORM_BUFFER,
                sizeof(T),
                (void*)&object,
                GL_DYNAMIC_DRAW);

            glBindBuffer(
                GL_UNIFORM_BUFFER,
                0);
        }
    };
}
