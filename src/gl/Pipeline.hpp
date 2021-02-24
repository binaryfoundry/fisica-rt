#pragma once

#include "../Camera.hpp"
#include "../Geometry.hpp"

#include "../math/Math.hpp"

#include "OpenGL.hpp"
#include "Texture2D.hpp"
#include "UniformBuffer.hpp"
#include "FrameBuffer.hpp"

#include <memory>

namespace GL
{
    struct CameraUniforms
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 viewport;
        glm::vec4 position;
        glm::vec4 exposure;
    };

    struct SceneUniforms
    {
        uint32_t num_geometry;
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
        GLuint raytracing_camera_uniform_location = 0;
        GLuint raytracing_scene_uniform_location = 0;
        GLuint raytracing_noise_texture_uniform_location = 0;
        GLuint raytracing_environment_texture_uniform_location = 0;
        GLuint raytracing_scene_texture_uniform_location = 0;

        const uint32_t scene_data_width = 8;
        const uint32_t scene_data_height = 1024;

        std::unique_ptr<GL::FrameBuffer<TexDataFloatRGBA>> framebuffer;
        std::unique_ptr<GL::UniformBuffer<CameraUniforms>> camera_uniforms;
        std::unique_ptr<GL::UniformBuffer<SceneUniforms>> scene_uniforms;

        std::unique_ptr<GL::Texture2D<TexDataFloatRGBA>> environment;
        std::unique_ptr<GL::Texture2D<TexDataFloatRGBA>> scene;
        std::unique_ptr<GL::Texture2D<TexDataByteRGBA, 4>> noise;

        void DrawQuad();

    public:
        Render();

        void Init();

        void InitRaytracing(
            const uint32_t framebuffer_width,
            const uint32_t framebuffer_height);

        void Deinit();

        void DeinitRaytracing();

        void Update(
            const std::vector<Sphere>& geometry);

        void Draw(
            const uint32_t window_width,
            const uint32_t window_height,
            const std::unique_ptr<Camera>& camera);
    };
}
