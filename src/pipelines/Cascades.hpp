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
        static constexpr int INPUT_WIDTH = 1024;
        static constexpr int INPUT_HEIGHT = 1024;
        static constexpr int ATLAS_SIZE = 512;
        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> cascade_fbs[NUM_CASCADES];
        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> gather_fb;
        std::unique_ptr<FrameBuffer<TexDataFloatRGBA>> distance_fbs[2];

        Shader distance_shader;
        Descriptor distance_set_0;
        float jump_distance = 0.0f;
        bool scene_dirty = true;
        float rendered_interval = -1.0f;
        float rendered_srgb = -1.0f;

        void UpdateDistanceField();
        void RenderRadiance();

        Shader frontbuffer_shader;
        Descriptor frontbuffer_set_0;

        Shader cascades_shader;
        Descriptor cascades_set_0;

        Shader gather_shader;
        Descriptor gather_set_0;

        float cascade_level = 0.0f;
        float cascade_count = static_cast<float>(NUM_CASCADES);
        float base_interval_f = 4.0f;
        float srgb_falloff = 1.0f;

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

        void PaintStroke(
            const glm::vec2& from, const glm::vec2& to,
            float brush_radius, const glm::vec4& color);

        void ClearInput();

        uint32_t InputWidth() const;
        uint32_t InputHeight() const;

        float& Exposure() { return exposure; }
        float& BaseInterval() { return base_interval_f; }
        float& SrgbFalloff() { return srgb_falloff; }
    };
}
