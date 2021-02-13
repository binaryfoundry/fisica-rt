#pragma once

#include "Camera.hpp"
#include "Uniforms.hpp"

#include "gl/GL.hpp"
#include "gl/Math.hpp"

#include <vector>
#include <memory>

class Render
{
private:
    GLuint quad_vertex_buffer = 0;
    GLuint quad_index_buffer = 0;

    GLuint frontbuffer_shader_program = 0;
    GLuint frontbuffer_sampler_state = 0;
    GLuint frontbuffer_position_attribute_location = 0;
    GLuint frontbuffer_texcoord_attribute_location = 0;
    GLuint frontbuffer_projection_uniform_location = 0;
    GLuint frontbuffer_view_uniform_location = 0;
    GLuint frontbuffer_texture_uniform_location = 0;

    GLuint raytracing_shader_program = 0;

    OpenGL::FrameBuffer framebuffer;

    std::shared_ptr<OpenGL::UniformBuffer<Transform>> transform;

    void Render::DrawQuad();

public:
    Render();

    void Init(
        const uint32_t framebuffer_width,
        const uint32_t framebuffer_height);
    void Deinit();
    void Draw(
        const uint32_t window_width,
        const uint32_t window_height,
        const std::unique_ptr<Camera>& camera);
};

