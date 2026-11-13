#include "Cascades.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    void SetTexture(GL::Descriptor& descriptor, const std::string& name, Texture2DResource& texture)
    {
        descriptor.SetSampler2D(name, texture,
            Filter::NEAREST, Filter::NEAREST,
            Wrap::CLAMP_TO_EDGE, Wrap::CLAMP_TO_EDGE);
    }
}

namespace Pipelines
{
    Cascades::Cascades()
    {
    }

    void Cascades::Init()
    {
        input = std::make_unique<GL::Texture2D<TexDataFloatRGBA>>(INPUT_WIDTH, INPUT_HEIGHT);
        std::fill(input->Data()->begin(), input->Data()->end(), glm::vec4(0.0f));
        input->Update();
        scene_dirty = true;
        rendered_interval = rendered_srgb = -1.0f;

        for (auto& cascade : cascade_fbs)
        {
            cascade = std::make_unique<FrameBuffer<TexDataFloatRGBA>>();
            cascade->Create(ATLAS_SIZE, ATLAS_SIZE, true);
        }
        for (auto& distance : distance_fbs)
        {
            distance = std::make_unique<FrameBuffer<TexDataFloatRGBA>>();
            distance->Create(INPUT_WIDTH, INPUT_HEIGHT, true);
        }
        gather_fb = std::make_unique<FrameBuffer<TexDataFloatRGBA>>();
        gather_fb->Create(INPUT_WIDTH, INPUT_HEIGHT, true);

        frontbuffer_shader.Load("files/gl/frontbuffer.glsl");
        frontbuffer_shader.Link();
        SetTexture(frontbuffer_set_0, "tex", *gather_fb);
        frontbuffer_set_0.SetUniformFloat("exposure", &exposure);
        frontbuffer_set_0.SetUniformFloat("srgb_falloff", &srgb_falloff);
        frontbuffer_set_0.SetUniformMat4("view", &view);
        frontbuffer_set_0.SetUniformMat4("projection", &projection);
        frontbuffer_shader.Set(frontbuffer_set_0, 0);

        distance_shader.Load("files/gl/distance.glsl");
        distance_shader.Link();
        SetTexture(distance_set_0, "scene_tex", *input);
        SetTexture(distance_set_0, "seeds_tex", *input);
        distance_set_0.SetUniformFloat("jump_distance", &jump_distance);

        cascades_shader.Load("files/gl/cascades.glsl");
        cascades_shader.Link();
        SetTexture(cascades_set_0, "scene_tex", *input);
        SetTexture(cascades_set_0, "distance_tex", *distance_fbs[0]);
        SetTexture(cascades_set_0, "prev_atlas", *cascade_fbs[0]);
        cascades_set_0.SetUniformFloat("cascade_level", &cascade_level);
        cascades_set_0.SetUniformFloat("cascade_count", &cascade_count);
        cascades_set_0.SetUniformFloat("base_interval_f", &base_interval_f);
        cascades_set_0.SetUniformFloat("srgb_falloff", &srgb_falloff);

        gather_shader.Load("files/gl/gather.glsl");
        gather_shader.Link();
        SetTexture(gather_set_0, "atlas", *cascade_fbs[0]);
        SetTexture(gather_set_0, "scene_tex", *input);
        gather_set_0.SetUniformFloat("srgb_falloff", &srgb_falloff);
        gather_shader.Set(gather_set_0, 0);
    }

    void Cascades::Deinit()
    {
        for (auto& cascade : cascade_fbs) cascade->Delete();
        for (auto& distance : distance_fbs) distance->Delete();
        gather_fb->Delete();
        input->Delete();
        frontbuffer_shader.Delete();
        distance_shader.Delete();
        cascades_shader.Delete();
        gather_shader.Delete();
    }

    void Cascades::UpdateDistanceField()
    {
        // Upload once per frame, even when a stroke changes many pixels.
        input->Update();
        jump_distance = 0.0f;
        SetTexture(distance_set_0, "seeds_tex", *input);
        distance_shader.Set(distance_set_0, 0);
        distance_fbs[0]->Bind();
        DrawQuad(distance_shader);

        int current = 0;
        int largest_jump = 1;
        while (largest_jump < std::max(INPUT_WIDTH, INPUT_HEIGHT)) largest_jump *= 2;
        for (int jump = largest_jump / 2; jump >= 1; jump /= 2)
        {
            jump_distance = static_cast<float>(jump);
            SetTexture(distance_set_0, "seeds_tex", *distance_fbs[current]);
            distance_shader.Set(distance_set_0, 0);
            distance_fbs[1 - current]->Bind();
            DrawQuad(distance_shader);
            current = 1 - current;
        }

        // JFA+1 improves nearest-seed accuracy at geometry boundaries.
        jump_distance = 1.0f;
        SetTexture(distance_set_0, "seeds_tex", *distance_fbs[current]);
        distance_shader.Set(distance_set_0, 0);
        distance_fbs[1 - current]->Bind();
        DrawQuad(distance_shader);
        SetTexture(cascades_set_0, "distance_tex", *distance_fbs[1 - current]);
    }

    void Cascades::RenderRadiance()
    {
        for (int i = NUM_CASCADES - 1; i >= 0; --i)
        {
            cascade_level = static_cast<float>(i);
            // The terminal cascade only queries this texture's dimensions.
            SetTexture(cascades_set_0, "prev_atlas",
                *cascade_fbs[i == NUM_CASCADES - 1 ? 0 : i + 1]);
            cascades_shader.Set(cascades_set_0, 0);
            cascade_fbs[i]->Bind();
            DrawQuad(cascades_shader);
        }

        gather_fb->Bind();
        DrawQuad(gather_shader);
        rendered_interval = base_interval_f;
        rendered_srgb = srgb_falloff;
    }

    void Cascades::Draw(
        const glm::mat4 projection_,
        const glm::mat4 view_,
        const bool upscale)
    {
        projection = projection_;
        view = view_;
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);

        if (scene_dirty || rendered_interval != base_interval_f || rendered_srgb != srgb_falloff)
        {
            if (scene_dirty) UpdateDistanceField();
            RenderRadiance();
            scene_dirty = false;
        }

        FrontBuffer();
        Clear();
        DrawQuad(frontbuffer_shader);
    }

    void Cascades::Paint(
        float u, float v, float brush_radius, const glm::vec4& color)
    {
        PaintStroke(glm::vec2(u, v), glm::vec2(u, v), brush_radius, color);
    }

    void Cascades::PaintStroke(
        const glm::vec2& from, const glm::vec2& to,
        float brush_radius, const glm::vec4& color)
    {
        const glm::vec2 resolution(input->Width(), input->Height());
        const glm::vec2 start = glm::clamp(from * resolution, glm::vec2(0.0f), resolution - 1.0f);
        const glm::vec2 end = glm::clamp(to * resolution, glm::vec2(0.0f), resolution - 1.0f);
        const glm::vec2 segment = end - start;
        const float squared_length = glm::dot(segment, segment);
        const float squared_radius = brush_radius * brush_radius;
        const glm::vec2 lower = glm::min(start, end) - brush_radius;
        const glm::vec2 upper = glm::max(start, end) + brush_radius;
        const int min_x = std::max(0, static_cast<int>(std::floor(lower.x)));
        const int min_y = std::max(0, static_cast<int>(std::floor(lower.y)));
        const int max_x = std::min(static_cast<int>(input->Width()) - 1, static_cast<int>(std::ceil(upper.x)));
        const int max_y = std::min(static_cast<int>(input->Height()) - 1, static_cast<int>(std::ceil(upper.y)));
        auto& data = *input->Data();

        for (int y = min_y; y <= max_y; ++y)
        {
            for (int x = min_x; x <= max_x; ++x)
            {
                const glm::vec2 offset = glm::vec2(x + 0.5f, y + 0.5f) - start;
                const float t = squared_length > 0.0f
                    ? glm::clamp(glm::dot(offset, segment) / squared_length, 0.0f, 1.0f) : 0.0f;
                const glm::vec2 distance = offset - segment * t;
                if (glm::dot(distance, distance) > squared_radius) continue;
                auto& pixel = data[y * input->Width() + x];
                if (pixel != color)
                {
                    pixel = color;
                    scene_dirty = true;
                }
            }
        }
    }

    void Cascades::ClearInput()
    {
        std::fill(input->Data()->begin(), input->Data()->end(), glm::vec4(0.0f));
        scene_dirty = true;
    }

    uint32_t Cascades::InputWidth() const { return input->Width(); }
    uint32_t Cascades::InputHeight() const { return input->Height(); }
}
