#include "Pipeline.hpp"

namespace GL
{
    void Pipeline::FrontBuffer()
    {
        glBindFramebuffer(
            GL_FRAMEBUFFER,
            0);

        glViewport(
            0,
            0,
            window_width,
            window_height);
    }

    void Pipeline::SetWindowSize(
        const uint32_t window_width_,
        const uint32_t window_height_)
    {
        window_width = window_width_;
        window_height = window_height_;
    }

    void Pipeline::DrawQuad(
        Shader& shader)
    {
        glBindBuffer(
            GL_ARRAY_BUFFER,
            quad_vertex_buffer);

        glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            quad_index_buffer);

        shader.Bind(0);

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(quad_indices_data.size()),
            GL_UNSIGNED_INT,
            static_cast<char const*>(0));

        glUseProgram(
            NULL);
    }
}
