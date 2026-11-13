#include "Application.hpp"

#include "Input.hpp"
#include "imgui/imgui.h"
#include "math/Random.hpp"

#include <array>
#include <cmath>

#if !defined(EMSCRIPTEN)
#include "sdl/Main.hpp"
#else
#include "sdl/MainWeb.hpp"
#endif

int main(int argc, char* argv[])
{
    std::unique_ptr<IApplication> app = std::make_unique<Application>();
    return sdl_init(app);
}

void Application::Init()
{
    pipeline.Init();

    gui.Init();
    LoadExampleScene();

    key_down_callback = [this](Scancode key)
    {
        switch (key)
        {
        case Scancode::S_C:
            pipeline.ClearInput();
            paint_button = -1;
            break;
        case Scancode::S_R:
            LoadExampleScene();
            break;
        default:
            break;
        }
    };

    key_up_callback = [=](Scancode key)
    {
        switch (key)
        {
        default:
            break;
        }
    };

    fps_time = timer_start();

    prop.Animate(
        context->property_manager,
        0, 1.0f, 1.0f,
        Properties::EasingFunction::Linear,
        []() {
            std::cout << "Complete!\n";
        }
    );
}

void Application::LoadExampleScene()
{
    pipeline.ClearInput();
    paint_button = -1;

    // The reference demo draws this curve and shadow bar on a 512px canvas.
    const float canvas_size = 512.0f;
    const float radius = 4.0f * static_cast<float>(pipeline.InputWidth()) / canvas_size;
    const glm::vec4 light(
        (249.0f / 255.0f) * emitter_intensity,
        (168.0f / 255.0f) * emitter_intensity,
        (117.0f / 255.0f) * emitter_intensity,
        1.0f);

    glm::vec2 previous(0.0f);
    for (int point = 0; point < 34; ++point)
    {
        const float t = static_cast<float>(point) * 0.3f;
        const float angle = t * 0.1f * PIf;
        const glm::vec2 position = glm::vec2(
            100.0f + 100.0f * std::sin(angle + 1.0f) * std::cos(angle * 0.25f),
            50.0f + 100.0f * std::sin(angle * 0.7f)) / canvas_size;
        if (point == 0) previous = position;
        pipeline.PaintStroke(previous, position, radius, light);
        previous = position;
    }

    pipeline.PaintStroke(
        glm::vec2(90.0f, 300.0f) / canvas_size,
        glm::vec2(246.8f, 300.0f) / canvas_size,
        radius, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Application::Deinit()
{
    pipeline.Deinit();
    gui.Deinit();
}


void Application::Update()
{
    const float time_ms = timer_end(fps_time);
    fps_time = timer_start();
    fps_time_avg = fps_alpha * fps_time_avg + (1.0f - fps_alpha) * time_ms;

    const float fps_scale = std::max<float>(
        1.0f / std::min<float>(5.0f, fps_time_avg / 16.66666f), 0.1f);

    context->property_manager.Update(
        time_ms / 1000.0f);

    const bool reinit_pipeline = GuiUpdate();

    const float window_aspect_ratio =
        static_cast<float>(window_width) /
        window_height;

    pipeline.SetWindowSize(
        window_width,
        window_height);

    ViewScale();

    // Mouse painting into the radiance input buffer
    {
        ImGuiIO& io = ImGui::GetIO();
        const bool left_down = io.MouseDown[0];
        const bool right_down = io.MouseDown[1];

        if ((left_down || right_down) && !io.WantCaptureMouse)
        {
            // Map window mouse coords to input texture UV using the same
            // viewport rect that ViewScale uses for the rendered quad
            const float window_aspect =
                static_cast<float>(window_width) / window_height;
            const float fb_ratio =
                static_cast<float>(raytracing_framebuffer_width) /
                static_cast<float>(raytracing_framebuffer_height);
            const float aspect = window_aspect / fb_ratio;
            const bool wide =
                window_width / fb_ratio > window_height;
            float scale_x = wide
                ? std::floor(window_width / aspect)
                : static_cast<float>(window_width);
            float scale_y = wide
                ? static_cast<float>(window_height)
                : std::floor(window_height * aspect);
            if (!upscale)
            {
                scale_x = std::min<float>(
                    scale_x, static_cast<float>(raytracing_framebuffer_width));
                scale_y = std::min<float>(
                    scale_y, static_cast<float>(raytracing_framebuffer_height));
            }
            const float hpos =
                std::round((window_width / 2) - (scale_x / 2));
            const float vpos =
                std::round((window_height / 2) - (scale_y / 2));

            const float local_x = io.MousePos.x - hpos;
            const float local_y = io.MousePos.y - vpos;
            const float u = local_x / scale_x;
            const float v = local_y / scale_y;

            if (u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f)
            {
                const glm::vec2 uv(u, v);
                const int button = left_down ? 0 : 1;
                const glm::vec2 from = paint_button == button ? previous_paint_uv : uv;
                if (left_down)
                {
                    const glm::vec4 color = glm::vec4(
                        emitter_color[0] * emitter_intensity,
                        emitter_color[1] * emitter_intensity,
                        emitter_color[2] * emitter_intensity,
                        1.0f);
                    pipeline.PaintStroke(from, uv, brush_radius_emitter, color);
                }
                else
                {
                    const glm::vec4 occluder = glm::vec4(
                        0.0f, 0.0f, 0.0f, 1.0f);
                    pipeline.PaintStroke(from, uv, brush_radius_occluder, occluder);
                }
                previous_paint_uv = uv;
                paint_button = button;
            }
            else
            {
                paint_button = -1;
            }
        }
        else
        {
            paint_button = -1;
        }
    }

    pipeline.Draw(
        projection,
        view,
        upscale);

    gui.Draw(
        window_width,
        window_height);
}

void Application::ViewScale()
{
    const float window_aspect =
        static_cast<float>(window_width) /
        window_height;

    const float framebuffer_ratio =
        static_cast<float>(raytracing_framebuffer_width) /
        static_cast<float>(raytracing_framebuffer_height);

    const float aspect =
        window_aspect / framebuffer_ratio;

    const bool wide =
        window_width / framebuffer_ratio > window_height;

    const glm::vec2 h_scale = glm::vec2(
        std::floor(window_width / aspect), window_height);

    const glm::vec2 v_scale = glm::vec2(
        window_width, std::floor(window_height * aspect));

    glm::vec3 scale = wide ?
        glm::vec3(h_scale, 1) :
        glm::vec3(v_scale, 1);

    if (!upscale)
    {
        scale.x = std::min<float>(
            scale.x, static_cast<float>(raytracing_framebuffer_width));

        scale.y = std::min<float>(
            scale.y, static_cast<float>(raytracing_framebuffer_height));
    }

    const float hpos =
        std::round((window_width / 2) - (scale.x / 2));

    const float vpos =
        std::round((window_height / 2) - (scale.y / 2));

    projection = glm::ortho<float>(
        0,
        static_cast<float>(window_width),
        static_cast<float>(window_height),
        0,
        -1.0f,
        1.0f);

    view = glm::mat4(1.0f);

    view = glm::translate(
        view,
        glm::vec3(hpos, vpos, 0.0f));

    view = glm::scale(
        view,
        scale);
}

bool Application::GuiUpdate()
{
    ImGui::NewFrame();

    ImGui::Begin(
        "Menu",
        NULL,
        ImGuiWindowFlags_AlwaysAutoResize);

    //ImGui::LabelText(
    //    "Controls",
     //   "Mouse, WSAD.");

    ImGui::Checkbox(
        "Upscale",
        &upscale);

    ImGui::Separator();
    ImGui::Text("Emitter (Left Click)");

    ImGui::ColorEdit3(
        "Color",
        emitter_color);

    ImGui::SliderFloat(
        "Intensity",
        &emitter_intensity,
        0.1f,
        20.0f);

    ImGui::SliderFloat(
        "Brush Size##emitter",
        &brush_radius_emitter,
        1.0f,
        32.0f);

    ImGui::Separator();
    ImGui::Text("Occluder (Right Click)");

    ImGui::SliderFloat(
        "Brush Size##occluder",
        &brush_radius_occluder,
        1.0f,
        32.0f);

    ImGui::Separator();
    ImGui::Text("C - Clear scene");
    if (ImGui::Button("Reset example (R)"))
        LoadExampleScene();

    ImGui::Separator();
    ImGui::Text("Radiance Cascades");

    ImGui::SliderFloat(
        "Exposure",
        &pipeline.Exposure(),
        0.1f,
        5.0f);

    ImGui::SliderFloat(
        "Base Interval",
        &pipeline.BaseInterval(),
        1.0f,
        32.0f,
        "%.0f");

    {
        bool sf = pipeline.SrgbFalloff() > 0.5f;
        if (ImGui::Checkbox("sRGB Falloff", &sf))
            pipeline.SrgbFalloff() = sf ? 1.0f : 0.0f;
    }

    ImGui::Separator();
    ImGui::Text(
        "%.3f ms/frame (%.1f FPS)",
        fps_time_avg,
        1000.0f / fps_time_avg);

    ImGui::End();

    return false;
}
