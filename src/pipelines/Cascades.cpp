#include "Cascades.hpp"

#include <sstream>
#include <algorithm>
#include <cmath>

#define WIDTH 1024
#define HEIGHT 1024

#define NUM_CASCADE_LEVELS 6

namespace Pipelines
{
    static const int cascade_resolutions[NUM_CASCADE_LEVELS] = {
        512, 256, 128, 64, 32, 16
    };

    Cascades::Cascades()
    {
    }

    void Cascades::Init()
    {
        // Create input texture for scene data (initialized to black)
        input = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>(
            WIDTH,
            HEIGHT);

        input->Update();

        // Create cascade framebuffers at decreasing resolutions
        for (int i = 0; i < NUM_CASCADES; i++)
        {
            cascade_fbs[i] =
                std::make_unique<FrameBuffer<TexDataFloatRGBA>>();

            cascade_fbs[i]->Create(
                cascade_resolutions[i],
                cascade_resolutions[i],
                true);
        }

        // Setup frontbuffer shader (reads from finest cascade)
        frontbuffer_shader.Load("files/gl/frontbuffer.glsl");
        frontbuffer_shader.Link();

        frontbuffer_set_0.SetSampler2D(
            "tex",
            *cascade_fbs[0],
            Filter::LINEAR,
            Filter::LINEAR,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);

        frontbuffer_set_0.SetUniformFloat(
            "exposure",
            &exposure);

        frontbuffer_set_0.SetUniformMat4(
            "view",
            &view);

        frontbuffer_set_0.SetUniformMat4(
            "projection",
            &projection);

        frontbuffer_shader.Set(
            frontbuffer_set_0,
            0);

        // Setup cascades shader
        cascades_shader.Load("files/gl/cascades.glsl");
        cascades_shader.Link();

        // Initial descriptor setup for cascade shader
        cascades_set_0.SetSampler2D(
            "input_sampler",
            *input,
            Filter::LINEAR,
            Filter::LINEAR,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);

        // Dummy prev_cascade binding (will be updated per cascade in Draw)
        cascades_set_0.SetSampler2D(
            "prev_cascade",
            *input,
            Filter::LINEAR,
            Filter::LINEAR,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);

        // Cascade-specific uniforms (pointer-based, auto-update on Bind)
        cascades_set_0.SetUniformFloat(
            "cascade_level",
            &cascade_level);

        cascades_set_0.SetUniformFloat(
            "cascade_res_x",
            &cascade_res_x);

        cascades_set_0.SetUniformFloat(
            "cascade_res_y",
            &cascade_res_y);

        cascades_set_0.SetUniformFloat(
            "input_res_x",
            &input_res_x);

        cascades_set_0.SetUniformFloat(
            "input_res_y",
            &input_res_y);

        cascades_set_0.SetUniformFloat(
            "ray_count_f",
            &ray_count_f);

        cascades_set_0.SetUniformFloat(
            "base_interval_f",
            &base_interval_f);

        cascades_shader.Set(
            cascades_set_0,
            0);

        // Initialize resolution uniforms to match scene size
        input_res_x = static_cast<float>(WIDTH);
        input_res_y = static_cast<float>(HEIGHT);
    }

    void Cascades::Deinit()
    {
        for (int i = 0; i < NUM_CASCADES; i++)
        {
            cascade_fbs[i]->Delete();
        }

        input->Delete();
        frontbuffer_shader.Delete();
        cascades_shader.Delete();
    }

    void Cascades::Draw(
        const glm::mat4 projection_,
        const glm::mat4 view_,
        const bool upscale)
    {
        // Render cascades from highest (coarsest, farthest) to
        // lowest (finest, nearest). Each cascade merges with the
        // previously rendered (coarser) cascade for far-field radiance.

        for (int i = NUM_CASCADES - 1; i >= 0; i--)
        {
            // Update cascade-level uniforms (auto-read via pointers)
            cascade_level = static_cast<float>(i);
            cascade_res_x = static_cast<float>(cascade_resolutions[i]);
            cascade_res_y = static_cast<float>(cascade_resolutions[i]);

            // Update prev_cascade sampler to point to the next
            // higher (coarser) cascade's result
            if (i < NUM_CASCADES - 1)
            {
                cascades_set_0.SetSampler2D(
                    "prev_cascade",
                    *cascade_fbs[i + 1],
                    Filter::LINEAR,
                    Filter::LINEAR,
                    Wrap::CLAMP_TO_EDGE,
                    Wrap::CLAMP_TO_EDGE);
            }
            else
            {
                // Highest cascade has no previous; bind input as
                // dummy (shader skips prev_cascade at max level)
                cascades_set_0.SetSampler2D(
                    "prev_cascade",
                    *input,
                    Filter::LINEAR,
                    Filter::LINEAR,
                    Wrap::CLAMP_TO_EDGE,
                    Wrap::CLAMP_TO_EDGE);
            }

            // Rebuild descriptor set with updated sampler
            cascades_shader.Set(
                cascades_set_0,
                0);

            // Bind this cascade's framebuffer and render
            cascade_fbs[i]->Bind();

            Clear();

            DrawQuad(
                cascades_shader);
        }

        // cascade_fbs[0] now contains the final merged radiance.
        // Render to front buffer with tone mapping.

        projection = projection_;
        view = view_;

        FrontBuffer();

        Clear();

        DrawQuad(
            frontbuffer_shader);
    }

    void Cascades::Paint(
        float u, float v,
        float brush_radius,
        const glm::vec4& color)
    {
        const uint32_t w = input->Width();
        const uint32_t h = input->Height();

        auto& data = input->Data();

        const int cx = static_cast<int>(u * w);
        const int cy = static_cast<int>(v * h);
        const int r = static_cast<int>(brush_radius);

        for (int dy = -r; dy <= r; dy++)
        {
            for (int dx = -r; dx <= r; dx++)
            {
                if (dx * dx + dy * dy > r * r)
                    continue;

                const int px = cx + dx;
                const int py = cy + dy;

                if (px < 0 || px >= static_cast<int>(w) ||
                    py < 0 || py >= static_cast<int>(h))
                    continue;

                (*data)[py * w + px] = color;
            }
        }

        input->Update();
    }

    void Cascades::ClearInput()
    {
        const uint32_t w = input->Width();
        const uint32_t h = input->Height();

        auto& data = input->Data();

        for (uint32_t i = 0; i < w * h; i++)
        {
            (*data)[i] = glm::vec4(0.0f);
        }

        input->Update();
    }

    uint32_t Cascades::InputWidth() const
    {
        return input->Width();
    }

    uint32_t Cascades::InputHeight() const
    {
        return input->Height();
    }
}
