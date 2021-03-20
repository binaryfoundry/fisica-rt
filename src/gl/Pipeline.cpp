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
        glDisable(
            GL_CULL_FACE);

        glCullFace(
            GL_BACK);

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

    void Pipeline::Clear()
    {
        glClearColor(
            0, 0, 0, 1);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);
    }
}
