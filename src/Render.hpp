#pragma once

#include "Camera.hpp"

#include "math/Math.hpp"

#include "gl/OpenGL.hpp"
#include "gl/Texture2D.hpp"
#include "gl/UniformBuffer.hpp"
#include "gl/FrameBuffer.hpp"

#include <vector>
#include <memory>

struct Transform
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 inverse_projection;
    glm::mat4 inverse_view_rotation;
    glm::vec4 viewport;
    glm::vec4 camera_position;
    glm::vec4 exposure;
};

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
    GLuint raytracing_transform_uniform_location = 0;
    GLuint raytracing_noise_0_texture_uniform_location = 0;
    GLuint raytracing_noise_1_texture_uniform_location = 0;
    GLuint raytracing_environment_texture_uniform_location = 0;
    GLuint raytracing_scene_texture_uniform_location = 0;

    std::unique_ptr<GL::FrameBuffer<TexDataFloatRGBA>> framebuffer;
    std::unique_ptr<GL::UniformBuffer<Transform>> transform;

    void DrawQuad();

public:
    Render();

    void Init(
        const uint32_t framebuffer_width,
        const uint32_t framebuffer_height);
    void Deinit();
    void Draw(
        const uint32_t window_width,
        const uint32_t window_height,
        const float exposure,
        const std::unique_ptr<Camera>& camera,
        const std::unique_ptr<GL::Texture2D<TexDataFloatRGBA>>& environment,
        const std::unique_ptr<GL::Texture2D<TexDataByteRGBA>>& noise_0,
        const std::unique_ptr<GL::Texture2D<TexDataByteRGBA>>& noise_1,
        const std::unique_ptr<GL::Texture2D<TexDataFloatRGBA>>& scene);
};

