#include "Cascades.hpp"

#include <sstream>
#include <algorithm>
#include <cmath>

#define WIDTH 1024
#define HEIGHT 1024

namespace Pipelines
{
    // Coarsest (n) to finest (0): each level has 2x more probes per axis
    static const int cascade_resolutions[] = {
        512, 256, 128, 64, 32, 16
    };

    Cascades::Cascades()
    {
    }

    void Cascades::Init()
    {
        input = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>(WIDTH, HEIGHT);
        input->Update();

        for (int i = 0; i < NUM_CASCADES; i++)
        {
            cascade_fbs[i] = std::make_unique<FrameBuffer<TexDataFloatRGBA>>();
            cascade_fbs[i]->Create(cascade_resolutions[i], cascade_resolutions[i], true);
        }

        frontbuffer_shader.Load("files/gl/frontbuffer.glsl");
        frontbuffer_shader.Link();

        frontbuffer_set_0.SetSampler2D(
            "tex",
            *cascade_fbs[0],
            Filter::LINEAR,
            Filter::LINEAR,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);
        frontbuffer_set_0.SetUniformFloat("exposure", &exposure);
        frontbuffer_set_0.SetUniformMat4("view", &view);
        frontbuffer_set_0.SetUniformMat4("projection", &projection);
        frontbuffer_shader.Set(frontbuffer_set_0, 0);

        cascades_shader.Load("files/gl/cascades.glsl");
        cascades_shader.Link();

        cascades_set_0.SetSampler2D(
            "input_sampler",
            *input,
            Filter::LINEAR,
            Filter::LINEAR,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);
        cascades_set_0.SetSampler2D(
            "prev_cascade",
            *input,
            Filter::LINEAR,
            Filter::LINEAR,
            Wrap::CLAMP_TO_EDGE,
            Wrap::CLAMP_TO_EDGE);
        cascades_set_0.SetUniformFloat("cascade_level", &cascade_level);
        cascades_set_0.SetUniformFloat("cascade_res_x", &cascade_res_x);
        cascades_set_0.SetUniformFloat("cascade_res_y", &cascade_res_y);
        cascades_set_0.SetUniformFloat("input_res_x", &input_res_x);
        cascades_set_0.SetUniformFloat("input_res_y", &input_res_y);
        cascades_set_0.SetUniformFloat("ray_count_f", &ray_count_f);
        cascades_set_0.SetUniformFloat("base_interval_f", &base_interval_f);
        cascades_set_0.SetUniformFloat("prev_cascade_res", &prev_cascade_res);
        cascades_set_0.SetUniformFloat("bilinear_fix", &bilinear_fix);
        cascades_set_0.SetUniformFloat("ringing_fix", &ringing_fix);
        cascades_set_0.SetUniformFloat("srgb_falloff", &srgb_falloff);
        cascades_set_0.SetUniformFloat("use_4x_branching", &use_4x_branching);
        cascades_shader.Set(cascades_set_0, 0);

        input_res_x = static_cast<float>(WIDTH);
        input_res_y = static_cast<float>(HEIGHT);
    }

    void Cascades::Deinit()
    {
        for (int i = 0; i < NUM_CASCADES; i++)
            cascade_fbs[i]->Delete();
        input->Delete();
        frontbuffer_shader.Delete();
        cascades_shader.Delete();
    }

    void Cascades::Draw(
        const glm::mat4 projection_,
        const glm::mat4 view_,
        const bool upscale)
    {
        projection = projection_;
        view = view_;

        for (int i = NUM_CASCADES - 1; i >= 0; i--)
        {
            cascade_level = static_cast<float>(i);
            cascade_res_x = static_cast<float>(cascade_resolutions[i]);
            cascade_res_y = static_cast<float>(cascade_resolutions[i]);

            if (i < NUM_CASCADES - 1)
            {
                prev_cascade_res = static_cast<float>(cascade_resolutions[i + 1]);
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
                // Coarsest level: no previous cascade; disable far-field merge
                prev_cascade_res = 0.0f;
                cascades_set_0.SetSampler2D(
                    "prev_cascade",
                    *input,
                    Filter::LINEAR,
                    Filter::LINEAR,
                    Wrap::CLAMP_TO_EDGE,
                    Wrap::CLAMP_TO_EDGE);
            }

            cascades_shader.Set(cascades_set_0, 0);

            cascade_fbs[i]->Bind();
            Clear();
            DrawQuad(cascades_shader);
        }

        FrontBuffer();
        Clear();
        DrawQuad(frontbuffer_shader);
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
                if (dx * dx + dy * dy > r * r) continue;
                const int px = cx + dx;
                const int py = cy + dy;
                if (px < 0 || px >= static_cast<int>(w) || py < 0 || py >= static_cast<int>(h))
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
            (*data)[i] = glm::vec4(0.0f);
        input->Update();
    }

    uint32_t Cascades::InputWidth() const { return input->Width(); }
    uint32_t Cascades::InputHeight() const { return input->Height(); }
}
