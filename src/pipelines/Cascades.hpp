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
        bool initialized = false;

        glm::mat4 projection;
        glm::mat4 view;

        float exposure = 1.0f;

        std::unique_ptr<Texture2D<TexDataFloatRGBA>> input;

        static constexpr int NUM_CASCADES = 5;
        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> cascade_fbs[NUM_CASCADES];

        Shader frontbuffer_shader;
        Descriptor frontbuffer_set_0;

        Shader cascades_shader;
        Descriptor cascades_set_0;

        // Cascade uniforms (float for descriptor system compatibility)
        float cascade_level = 0.0f;
        float cascade_res_x = 512.0f;
        float cascade_res_y = 512.0f;
        float input_res_x = 512.0f;
        float input_res_y = 512.0f;
        float ray_count_f = 64.0f;
        float base_interval_f = 16.0f;

    public:
        Cascades();

        void Init();

        void Deinit();

        void Draw(
            const glm::mat4 projection_,
            const glm::mat4 view_,
            const bool upscale);

        // Paint an emitter or occluder into the input buffer.
        // u,v in [0,1], brush_radius in pixels.
        // If emitter is true, paints a light source; otherwise an occluder.
        void Paint(
            float u, float v,
            float brush_radius,
            bool emitter);

        // Clear the input buffer to empty (transparent black).
        void ClearInput();

        // Get the input texture dimensions for coordinate mapping.
        uint32_t InputWidth() const;
        uint32_t InputHeight() const;
    };
}
