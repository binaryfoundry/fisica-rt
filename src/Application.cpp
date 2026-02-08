#include "Application.hpp"

#include "Input.hpp"
#include "imgui/imgui.h"
#include "math/Random.hpp"

#include <array>

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

    key_down_callback = [this](Scancode key)
    {
        switch (key)
        {
        case Scancode::S_C:
            pipeline.ClearInput();
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

            const float local_x = static_cast<float>(mouse_x) - hpos;
            const float local_y = static_cast<float>(mouse_y) - vpos;
            const float u = local_x / scale_x;
            const float v = local_y / scale_y;

            if (u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f)
            {
                if (left_down)
                {
                    const glm::vec4 color = glm::vec4(
                        emitter_color[0] * emitter_intensity,
                        emitter_color[1] * emitter_intensity,
                        emitter_color[2] * emitter_intensity,
                        0.0f);
                    pipeline.Paint(u, v, brush_radius_emitter, color);
                }
                else
                {
                    const glm::vec4 occluder = glm::vec4(
                        0.0f, 0.0f, 0.0f, 1.0f);
                    pipeline.Paint(u, v, brush_radius_occluder, occluder);
                }
            }
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

    view = glm::mat4();

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

    ImGui::Separator();
    ImGui::Text(
        "%.3f ms/frame (%.1f FPS)",
        fps_time_avg,
        1000.0f / fps_time_avg);

    ImGui::End();

    return false;
}
