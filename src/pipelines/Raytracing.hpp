#pragma once

#include "../Platform.hpp"
#include "../Camera.hpp"
#include "../Graphics.hpp"
#include "../Geometry.hpp"

#include "../math/Math.hpp"

#include <memory>

namespace Pipelines
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
        uint32_t padding_0;
        uint32_t padding_1;
        uint32_t padding_2;
    };

    class Raytracing : public Pipeline
    {
    private:
        bool initialized = false;
        bool render_environment = true;

        float exposure = 1.0f;

        const uint32_t scene_data_width = 8;
        const uint32_t scene_data_height = 1024;

        glm::mat4 view;
        glm::mat4 projection;

        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> framebuffer;
        std::unique_ptr<UniformBuffer<CameraUniforms>> camera_uniforms;
        std::unique_ptr<UniformBuffer<SceneUniforms>> scene_uniforms;

        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> environment;
        std::unique_ptr<Texture2D<TexDataFloatRGBA>> scene;
        std::unique_ptr<Texture2D<TexDataByteRGBA, 64>> noise;

        Shader frontbuffer_shader;
        Shader raytracing_shader;
        Shader environment_shader;

        Descriptor frontbuffer_set_0;
        Descriptor raytracing_set_0;

    public:
        Raytracing();

        void Init();

        void InitRaytracing(
            const uint32_t framebuffer_width,
            const uint32_t framebuffer_height,
            const uint16_t samples,
            const uint16_t bounces);

        void Deinit();

        void DeinitRaytracing();

        void Update(
            const std::vector<Sphere>& geometry);

        void Draw(
            const std::unique_ptr<Camera>& camera,
            const glm::mat4 projection,
            const glm::mat4 view,
            const bool upscale);
    };
}
