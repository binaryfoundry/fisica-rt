#pragma once

#include "../Platform.hpp"
#include "../Graphics.hpp"

#include "../math/Math.hpp"

#include <memory>

namespace Pipelines
{
    class Cascades : public Pipeline
    {
    private:
        glm::mat4 projection;
        glm::mat4 view;

        float exposure = 1.0f;

        std::unique_ptr<Texture2D<TexDataFloatRGBA>> input;

        static constexpr int NUM_CASCADES = 6;
        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> cascade_fbs[NUM_CASCADES];

        Shader frontbuffer_shader;
        Descriptor frontbuffer_set_0;

        Shader cascades_shader;
        Descriptor cascades_set_0;

        float cascade_level = 0.0f;
        float cascade_res_x = 512.0f;
        float cascade_res_y = 512.0f;
        float input_res_x = 1024.0f;
        float input_res_y = 1024.0f;
        float ray_count_f = 64.0f;
        float base_interval_f = 24.0f;
        float prev_cascade_res = 256.0f;
        float bilinear_fix = 1.0f;
        float ringing_fix = 1.0f;
        float srgb_falloff = 1.0f;
        float use_4x_branching = 1.0f;

    public:
        Cascades();

        void Init();

        void Deinit();

        void Draw(
            const glm::mat4 projection_,
            const glm::mat4 view_,
            const bool upscale);

        void Paint(
            float u, float v,
            float brush_radius,
            const glm::vec4& color);

        void ClearInput();

        uint32_t InputWidth() const;
        uint32_t InputHeight() const;

        float& Exposure() { return exposure; }
        float& BaseInterval() { return base_interval_f; }
        float& RayCount() { return ray_count_f; }
        float& BilinearFix() { return bilinear_fix; }
        float& RingingFix() { return ringing_fix; }
        float& SrgbFalloff() { return srgb_falloff; }
        float& Use4xBranching() { return use_4x_branching; }
    };
}
