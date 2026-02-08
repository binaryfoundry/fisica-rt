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

        static constexpr int NUM_CASCADES = 6;
        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> cascade_fbs[NUM_CASCADES];

        Shader frontbuffer_shader;
        Descriptor frontbuffer_set_0;

        Shader cascades_shader;
        Descriptor cascades_set_0;

        // Cascade uniforms (float for descriptor system compatibility)
        float cascade_level = 0.0f;
        float cascade_res_x = 512.0f;
        float cascade_res_y = 512.0f;
        float input_res_x = 1024.0f;
        float input_res_y = 1024.0f;
        float ray_count_f = 48.0f;
        float base_interval_f = 16.0f;

    public:
        Cascades();

        void Init();

        void Deinit();

        void Draw(
            const glm::mat4 projection_,
            const glm::mat4 view_,
            const bool upscale);

        // Paint into the input buffer at UV coordinates.
        // u,v in [0,1], brush_radius in pixels.
        // color is the RGBA value to paint (e.g. emitter or occluder).
        void Paint(
            float u, float v,
            float brush_radius,
            const glm::vec4& color);

        // Clear the input buffer to empty (transparent black).
        void ClearInput();

        // Get the input texture dimensions for coordinate mapping.
        uint32_t InputWidth() const;
        uint32_t InputHeight() const;
    };
}
