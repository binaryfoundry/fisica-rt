#pragma once

#include "Camera.hpp"

#include "gl/GL.hpp"
#include "gl/Math.hpp"

#include <vector>

class Render
{
private:
    GLuint gl_shader_program = 0;
    GLuint gl_sampler_state = 0;

    GLuint position_attribute_location = 0;
    GLuint texcoord_attribute_location = 0;
    GLuint projection_uniform_location = 0;
    GLuint view_uniform_location = 0;
    GLuint texture_uniform_location = 0;
    GLuint texture_scale_uniform_location = 0;
    GLuint texture_uniform_flip = 0;

    GLuint vertex_buffer = 0;
    GLuint index_buffer = 0;

    uint32_t display_width = 0;
    uint32_t display_height = 0;

    OpenGL::FrameBuffer frame_buffer;

    void Render::DrawQuad(
        const glm::mat4 proj,
        const glm::mat4 view,
        const GLuint texture,
        const bool flip);

public:
    Render();

    void Init(
        const uint32_t display_width,
        const uint32_t display_height);
    void Deinit();
    void Draw(
        const uint32_t window_width,
        const uint32_t window_height,
        const std::unique_ptr<Camera>& camera);
};

